"""Deterministic spectral-change estimates, before random phase and output tanh."""

import numpy as np


class SpectralResponse:
    """Compare level-preserving log mel-band power with a smooth energy floor."""

    def __init__(self, parameters, bands=32, floor_db=-80.0):
        import librosa

        self.xmax = float(parameters["xMax"])
        self.sclip = float(parameters["sClip"])
        self.floor_db = float(floor_db)
        self.filters = librosa.filters.mel(
            sr=int(parameters["sr"]), n_fft=int(parameters["win_length"]),
            n_mels=bands, norm=1, dtype=np.float64,
        )
        if not np.isfinite([self.xmax, self.sclip, self.floor_db]).all():
            raise ValueError("spectral scaling and floor must be finite")

    def features(self, predictions):
        predictions = np.asarray(predictions, dtype=np.float64)
        if predictions.shape[-1] != self.filters.shape[1] or not np.isfinite(predictions).all():
            raise ValueError("decoder returned non-finite values or an unexpected spectrum size")
        power_db = np.clip(predictions * self.xmax + self.sclip, -120, 120)
        power = 10 ** (power_db / 10)
        floor = 10 ** (self.floor_db / 10)
        bands = 10 * np.log10(power @ self.filters.T + floor)
        level = 10 * np.log10(power.mean(axis=-1) + floor)
        return bands, level


def summarize_sweep(features, level, positions, observed_mask):
    """Keep narrow transitions and whole-range effect as separate measurements."""
    differences = np.sqrt(np.mean(np.diff(features, axis=1) ** 2, axis=-1))
    response = differences / np.diff(positions)
    excursions = spectral_excursions(features[:, observed_mask, :])
    return {
        "score": float(np.percentile(excursions, 90)),
        "typical_effect": float(np.median(excursions)),
        "peak_effect": float(np.max(excursions)),
        "playable_peak_effect": float(np.max(spectral_excursions(features))),
        "response_mean": response.mean(axis=0).tolist(),
        "response_peak": response.max(axis=0).tolist(),
        "spectral_level_mean_db": level.mean(axis=0).tolist(),
    }


def spectral_excursions(features):
    """RMS of each band's sweep span, preserving a narrow but strong response."""
    if features.shape[1] < 2:
        return np.zeros(features.shape[0])
    return np.sqrt(np.mean(np.ptp(features, axis=1) ** 2, axis=-1))
