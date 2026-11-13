# -*- coding: utf-8 -*-

"""
Render several WAV files as a single stacked-spectrogram PNG.

Each ``--panel`` is "wav::label" and becomes one subplot, all sharing the same
analysis parameters and dB range so the panels are directly comparable. Used to
build the side-by-side training-parameter comparison figures for the thesis.
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
    parser.add_argument("--out", required=True, help="Output PNG path")
    parser.add_argument("--panel", action="append", required=True,
                        help='One panel as "wav_path::label" (repeatable)')
    parser.add_argument("--sr", type=int, default=22050)
    parser.add_argument("--n-fft", type=int, default=2048)
    parser.add_argument("--hop", type=int, default=512)
    parser.add_argument("--vmin", type=float, default=-80.0)
    parser.add_argument("--vmax", type=float, default=0.0)
    parser.add_argument("--fig-width", type=float, default=9.0)
    parser.add_argument("--fig-height", type=float, default=None,
                        help="Total figure height in inches (default: 2.5 per panel)")
    args = parser.parse_args()

    panels = []
    for spec in args.panel:
        wav, _, label = spec.partition("::")
        panels.append((wav, label))

    n = len(panels)
    fig_height = args.fig_height if args.fig_height else 2.5 * n
    fig, axes = plt.subplots(n, 1, figsize=(args.fig_width, fig_height))
    if n == 1:
        axes = [axes]

    for ax, (wav, label) in zip(axes, panels):
        y, sr = librosa.load(wav, sr=args.sr, mono=True)
        stft = np.abs(librosa.stft(y, n_fft=args.n_fft, hop_length=args.hop))
        stft_db = librosa.amplitude_to_db(stft, ref=np.max)
        img = librosa.display.specshow(
            stft_db, sr=sr, hop_length=args.hop,
            x_axis="time", y_axis="linear", cmap="magma",
            vmin=args.vmin, vmax=args.vmax, ax=ax,
        )
        ax.set_ylim(0, sr / 2)
        if label:
            ax.set_title(label)
        ax.set_xlabel("Tiempo (s)")
        ax.set_ylabel("Frecuencia (Hz)")
        colorbar = fig.colorbar(img, ax=ax, format="%+0.0f dB")
        colorbar.set_label("Nivel relativo")

    fig.tight_layout()
    fig.savefig(args.out, dpi=160)
    plt.close(fig)
    print(f"[grid] wrote {args.out} ({n} panels)")


if __name__ == "__main__":
    main()
