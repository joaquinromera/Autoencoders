"""Ranges and MIDI metadata for a decoder's exported input coordinates."""

import numpy as np

from latent_midi import DEFAULT_SCLIP_CC, DEFAULT_XMAX_CC, build_default_slider_ccs


def validate_latent_track(latent_track):
    track = np.asarray(latent_track, dtype=np.float32)
    if track.ndim != 2 or 0 in track.shape or not np.isfinite(track).all():
        raise ValueError("latent_track must be a finite, non-empty 2D array")
    return track


def build_latent_ranges(latent_track):
    """Include a small playable margin around the encoded training frames."""
    track = validate_latent_track(latent_track)
    minimum, maximum = track.min(axis=0), track.max(axis=0)
    spans = maximum - minimum
    padding = np.where(spans > 1e-6, spans * 0.05, np.maximum(abs(maximum), 1) * 0.1)
    padding = np.maximum(padding, 1e-3)
    return [{"min": f"{low:.9g}", "max": f"{high:.9g}"}
            for low, high in zip(minimum - padding, maximum + padding)]


def build_automation_metadata(settings, latent_track, source_files, source_frame_counts):
    """CC numbers follow exported input order, as they do in the synth."""
    return {
        "frame_count": int(latent_track.shape[0]),
        "frame_period_seconds": float(settings["hop_length"]) / float(settings["sr"]),
        "source_files": source_files,
        "source_frame_counts": source_frame_counts,
        "default_slider_cc": build_default_slider_ccs(int(settings["latent_dim"])),
        "default_xmax_cc": DEFAULT_XMAX_CC,
        "default_sclip_cc": DEFAULT_SCLIP_CC,
    }
