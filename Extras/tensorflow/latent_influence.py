"""Offline influence ranking and typical response profiles for latent controls."""

from dataclasses import dataclass

import numpy as np

from latent_metadata import validate_latent_track
from spectral_response import SpectralResponse, summarize_sweep


@dataclass(frozen=True)
class InfluenceSettings:
    reference_count: int = 16
    sweep_points: int = 129

    def __post_init__(self):
        if (not isinstance(self.reference_count, int) or not isinstance(self.sweep_points, int)
                or self.reference_count < 1 or self.sweep_points < 3):
            raise ValueError("influence analysis needs at least one reference and three sweep points")


def select_reference_indices(track, source_frame_counts, limit):
    """Round-robin files, selecting normalized latent states farthest from those chosen."""
    counts = np.asarray(source_frame_counts, dtype=int)
    if (counts.ndim != 1 or not len(counts) or np.any(counts <= 0)
            or counts.sum() != len(track) or limit < 1):
        raise ValueError("source_frame_counts must partition the latent trajectory")
    normalized = (track - track.min(axis=0)) / np.maximum(np.ptp(track, axis=0), 1e-6)
    groups = np.split(np.arange(len(track)), np.cumsum(counts)[:-1])
    selected, distances = [], np.full(len(track), np.inf)
    while len(selected) < min(limit, len(track)):
        for group in groups:
            candidates = group[~np.isin(group, selected)]
            if len(candidates) and len(selected) < min(limit, len(track)):
                index = int(candidates[np.argmax(distances[candidates])])
                selected.append(index)
                distances = np.minimum(distances, ((normalized - normalized[index]) ** 2).sum(axis=1))
    return np.asarray(selected)


def sweep_positions(bounds, observed, count):
    """Sample the playable range, including the exact observed bounds."""
    low, high = float(bounds["min"]), float(bounds["max"])
    if not np.isfinite([low, high]).all() or low >= high:
        raise ValueError("latent ranges must be finite and increasing")
    # Match decoder input precision; otherwise near-duplicate float64 samples
    # could imply a huge slope across a sub-float32 step.
    coordinates = np.unique(np.r_[np.linspace(low, high, count), observed].astype(np.float32))
    if len(coordinates) < 2:
        raise ValueError("latent range collapses at decoder input precision")
    precise = coordinates.astype(np.float64)
    return coordinates, (precise - precise[0]) / (precise[-1] - precise[0])


def measure_dimension(predict, references, track, dimension, bounds, metric, count):
    observed = [track[:, dimension].min(), track[:, dimension].max()]
    coordinates, positions = sweep_positions(bounds, observed, count)
    candidates = np.repeat(references[:, None, :], len(coordinates), axis=1)
    candidates[:, :, dimension] = coordinates
    predictions = predict(candidates.reshape(-1, track.shape[1]))
    features, level = metric.features(np.asarray(predictions).reshape(len(references), len(coordinates), -1))
    observed_mask = (coordinates >= observed[0]) & (coordinates <= observed[1])
    summary = summarize_sweep(features, level, positions, observed_mask)
    return dict(summary, original_dimension=dimension, positions=positions.tolist(),
                observed_range=[float(value) for value in observed])


def analyze_latent_influence(predict, latent_track, parameters, source_frame_counts, settings=None):
    """Return JSON-ready analysis in original input order; no decoder mutation."""
    settings = settings or InfluenceSettings()
    track = validate_latent_track(latent_track)
    ranges = parameters["zRange"]
    if len(ranges) != track.shape[1]:
        raise ValueError("zRange must match the latent dimension")
    indices = select_reference_indices(track, source_frame_counts, settings.reference_count)
    metric = SpectralResponse(parameters)
    dimensions = [measure_dimension(predict, track[indices], track, index, bounds,
                                    metric, settings.sweep_points)
                  for index, bounds in enumerate(ranges)]
    order = sorted(range(len(dimensions)), key=lambda index: -dimensions[index]["score"])
    return analysis_metadata(parameters, settings, indices, dimensions, order, metric.floor_db)


def analysis_metadata(parameters, settings, indices, dimensions, order, floor_db):
    return {
        "version": 1,
        "method": "log_mel_sweep_v1",
        "profile_scope": "training_reference_summary",
        "score_definition": "p90_reference_rms_band_span_in_observed_range",
        "response_definition": "rms_band_change_per_unit_normalized_position",
        "reference_frame_indices": indices.tolist(),
        "requested_sweep_points": settings.sweep_points,
        "spectral_floor_db": floor_db,
        "mel_bands": 32,
        "reference_parameters": {key: parameters[key] for key in ("sr", "win_length", "xMax", "sClip")},
        "input_order_definition": "exported_input_index_to_original_dimension",
        "profile_positions": "normalized_playable_range; responses cover adjacent intervals",
        "input_order": order,
        "dimensions": dimensions,
    }
