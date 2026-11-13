# -*- coding: utf-8 -*-

"""
Render a WAV file as a dB-scaled STFT spectrogram PNG.

All spectrograms use the same analysis parameters and dB range so that figures
produced for different models are directly comparable in the thesis. The audio
is always analyzed at a fixed resolution (independent of the model's training
window) because we are visualizing the *rendered output*, not the model.
"""

import argparse
import os

os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "3")

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import librosa  # noqa: E402
import librosa.display  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wav", required=True, help="Input WAV file")
    parser.add_argument("--out", required=True, help="Output PNG path")
    parser.add_argument("--title", default="", help="Plot title")
    parser.add_argument("--sr", type=int, default=22050, help="Display sample rate")
    parser.add_argument("--n-fft", type=int, default=2048, help="Display FFT size")
    parser.add_argument("--hop", type=int, default=512, help="Display hop length")
    parser.add_argument("--vmin", type=float, default=-80.0, help="Min dB for color scale")
    parser.add_argument("--vmax", type=float, default=0.0, help="Max dB for color scale")
    args = parser.parse_args()

    y, sr = librosa.load(args.wav, sr=args.sr, mono=True)
    stft = np.abs(librosa.stft(y, n_fft=args.n_fft, hop_length=args.hop))
    stft_db = librosa.amplitude_to_db(stft, ref=np.max)

    fig, ax = plt.subplots(figsize=(7.5, 3.2))
    img = librosa.display.specshow(
        stft_db, sr=sr, hop_length=args.hop,
        x_axis="time", y_axis="linear", cmap="magma",
        vmin=args.vmin, vmax=args.vmax, ax=ax,
    )
    ax.set_ylim(0, sr / 2)
    if args.title:
        ax.set_title(args.title)
    ax.set_xlabel("Tiempo (s)")
    ax.set_ylabel("Frecuencia (Hz)")
    colorbar = fig.colorbar(img, ax=ax, format="%+0.0f dB")
    colorbar.set_label("Nivel relativo")
    fig.tight_layout()
    fig.savefig(args.out, dpi=150)
    plt.close(fig)
    print(f"[spectrogram] wrote {args.out} ({stft_db.shape[1]} frames, sr={sr})")


if __name__ == "__main__":
    main()
