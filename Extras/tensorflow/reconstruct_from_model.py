# -*- coding: utf-8 -*-

"""
Offline reconstruction of a trained decoder's latent-automation trajectory.

Given a model directory containing the Keras SavedModel ``decoder/`` and the
``automation.json`` produced by ``generate_and_save_models_from_audio.py``, this
script renders the latent trajectory (``ztrack``) back to audio, reproducing the
exact DSP used by the real-time synthesizer in ``Source/dsp/Autoencoder.cpp``:

    log10Power = (prediction * xMax + sClip) / 10        (clamped to [-12, 12])
    magnitude  = 10 ** (log10Power * 0.5)
    spectrum   = magnitude * exp(i * random_phase)       (random phase per bin)
    audio      = inverse-STFT(spectrum) with a Hann window and overlap-add

The result is the "reconstrucción aproximada del audio de entrenamiento"
described in the thesis: the decoder applied to the encoder's latent trajectory
of the source material. It also reports the reconstruction MSE against the
source's normalized spectrogram, useful as a convergence metric.
"""

import argparse
import json
import os
from pathlib import Path

os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "3")

import numpy as np
import librosa
import soundfile as sf


def _load_payload(model_dir):
    """Load automation.json (preferred) or data.json from the model directory."""
    for candidate in ("automation.json", "data.json"):
        path = Path(model_dir) / candidate
        if path.exists():
            with open(path, "r", encoding="utf-8") as handle:
                return json.load(handle)
    raise FileNotFoundError(
        f"Neither automation.json nor data.json found in {model_dir}"
    )


def _source_spectrogram(path, sr, win, hop, sclip, xmax):
    """Recompute the normalized training-target spectrogram (matches training)."""
    x, _ = librosa.load(path, sr=sr, mono=True, duration=120)
    x = np.trim_zeros(x)
    F = librosa.stft(x, n_fft=win, hop_length=hop).T
    S = 10.0 * np.log10(np.abs(F) ** 2 + 1e-20)
    S = S.clip(sclip, None) - sclip
    return (S / xmax).astype(np.float32)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model-dir", required=True,
                        help="Directory containing decoder/ and automation.json")
    parser.add_argument("--out", default=None,
                        help="Output WAV path (default: <model-dir>/reconstruction.wav)")
    parser.add_argument("--metrics-json", default=None,
                        help="Optional path to write reconstruction metrics as JSON")
    parser.add_argument("--seed", type=int, default=0,
                        help="Seed for the random phase (reproducible renders)")
    args = parser.parse_args()

    import tensorflow as tf

    model_dir = Path(args.model_dir)
    out = Path(args.out) if args.out else model_dir / "reconstruction.wav"

    payload = _load_payload(model_dir)
    params = payload["parameters"]
    win = int(params["win_length"])
    hop = int(params["hop_length"])
    sr = int(params["sr"])
    xmax = float(params["xMax"])
    sclip = float(params["sClip"])

    ztrack = np.asarray(payload["ztrack"], dtype=np.float32)
    if ztrack.ndim != 2 or ztrack.shape[0] == 0:
        raise ValueError("ztrack must be a non-empty 2D array")

    decoder = tf.keras.models.load_model(str(model_dir / "decoder"), compile=False)
    pred = decoder.predict(ztrack, batch_size=256, verbose=0)  # (frames, bins) in [0, 1]
    n_bins = pred.shape[1]
    if n_bins != win // 2 + 1:
        raise ValueError(f"decoder output {n_bins} != win//2+1 = {win // 2 + 1}")

    # Magnitude recovery, identical to Autoencoder.cpp::calculate.
    log10_power = (pred.astype(np.float64) * xmax + sclip) / 10.0
    log10_power = np.where(np.isfinite(log10_power), log10_power, -12.0)
    log10_power = np.clip(log10_power, -12.0, 12.0)
    magnitude = np.power(10.0, log10_power * 0.5)

    rng = np.random.default_rng(args.seed)
    phase = rng.uniform(0.0, 2.0 * np.pi, size=magnitude.shape)
    spec = (magnitude * np.exp(1j * phase)).T.astype(np.complex64)  # (bins, frames)

    y = librosa.istft(spec, hop_length=hop, win_length=win, n_fft=win, window="hann")
    peak = float(np.max(np.abs(y))) if y.size else 0.0
    if peak > 0:
        y = (y / peak) * 0.97
    y = y.astype(np.float32)

    out.parent.mkdir(parents=True, exist_ok=True)
    sf.write(str(out), y, sr, subtype="PCM_16")
    duration = len(y) / float(sr)
    print(f"[reconstruct] wrote {out}")
    print(f"[reconstruct] frames={ztrack.shape[0]} bins={n_bins} win={win} "
          f"hop={hop} sr={sr} dur={duration:.2f}s peak={peak:.4f}")

    mse = None
    sources = payload.get("automation", {}).get("source_files", [])
    if sources and Path(sources[0]).exists():
        try:
            x_src = _source_spectrogram(sources[0], sr, win, hop, sclip, xmax)
            n = min(x_src.shape[0], pred.shape[0])
            if x_src.shape[1] == pred.shape[1] and n > 0:
                mse = float(np.mean((pred[:n] - x_src[:n]) ** 2))
                print(f"[reconstruct] reconstruction MSE (normalized spectrogram) = {mse:.6f}")
        except Exception as exc:  # noqa: BLE001 - diagnostics only
            print(f"[reconstruct] MSE skipped: {type(exc).__name__}: {exc}")
    else:
        print("[reconstruct] MSE skipped: source file not available")

    if args.metrics_json:
        with open(args.metrics_json, "w", encoding="utf-8") as handle:
            json.dump({
                "model_dir": str(model_dir),
                "win_length": win,
                "hop_length": hop,
                "sr": sr,
                "frames": int(ztrack.shape[0]),
                "duration_seconds": duration,
                "mse": mse,
            }, handle, indent=2)
        print(f"[reconstruct] wrote {args.metrics_json}")


if __name__ == "__main__":
    main()
