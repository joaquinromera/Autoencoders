"""Behavioral checks for offline rankings, narrow responses and shared scales."""

import unittest

import numpy as np

from latent_influence import InfluenceSettings, analyze_latent_influence, select_reference_indices, sweep_positions
from latent_metadata import build_latent_ranges


def parameters(track):
    return {"win_length": 256, "sr": 22050, "xMax": "40", "sClip": "-40",
            "latent_dim": track.shape[1], "zRange": build_latent_ranges(track)}


def repeat_spectrum(values):
    return np.repeat(values[:, None], 129, axis=1)


class InfluenceTests(unittest.TestCase):
    def setUp(self):
        self.track = np.array([[-1, -1, -1], [0, 0, 0], [1, 1, 1]], dtype=np.float32)
        self.settings = InfluenceSettings(3, 129)

    def analyze(self, predict, track=None):
        track = self.track if track is None else track
        return analyze_latent_influence(predict, track, parameters(track), [len(track)], self.settings)

    def test_orders_decoded_effect_and_keeps_inactive_dimension_last(self):
        predict = lambda z: repeat_spectrum(0.5 + z @ [0, 0.02, 0.2])
        result = self.analyze(predict)
        self.assertEqual(result["input_order"], [2, 1, 0])
        self.assertEqual(result["dimensions"][0]["score"], 0)
        self.assertTrue(np.allclose(result["dimensions"][0]["response_mean"], 0))
        self.assertGreater(result["dimensions"][2]["score"], 5 * result["dimensions"][1]["score"])

    def test_finds_narrow_response_away_from_the_center(self):
        def predict(z):
            peak = np.exp(-((z[:, 1] - 0.75) / 0.035) ** 2)
            return repeat_spectrum(0.5 + 0.3 * peak + z[:, 2] * 0.005)
        result = self.analyze(predict)
        self.assertEqual(result["input_order"][0], 1)
        profile = result["dimensions"][1]
        midpoint = (np.array(profile["positions"][:-1]) + profile["positions"][1:]) / 2
        self.assertGreater(midpoint[np.argmax(profile["response_peak"])] , 0.75)
        self.assertGreater(max(profile["response_peak"]), 100)

    def test_equal_range_fractions_are_independent_of_latent_units(self):
        baseline = self.analyze(lambda z: repeat_spectrum(0.5 + z[:, 0] * 0.1))
        scaled = self.track.copy()
        scaled[:, 0] *= 100
        result = self.analyze(lambda z: repeat_spectrum(0.5 + z[:, 0] * 0.001), scaled)
        self.assertAlmostEqual(baseline["dimensions"][0]["score"], result["dimensions"][0]["score"], places=5)

    def test_profiles_preserve_different_reference_contexts(self):
        result = self.analyze(lambda z: repeat_spectrum(0.5 + 0.2 * z[:, 0] * z[:, 1]))
        profile = result["dimensions"][0]
        self.assertGreater(profile["score"], 0)
        self.assertGreater(max(profile["response_peak"]), max(profile["response_mean"]))
        self.assertEqual(len(profile["response_mean"]), len(profile["positions"]) - 1)
        self.assertEqual(len(profile["spectral_level_mean_db"]), len(profile["positions"]))

    def test_changes_below_energy_floor_are_suppressed(self):
        quiet = parameters(self.track)
        quiet["sClip"] = "-120"
        predict = lambda z: repeat_spectrum(z[:, 0] * 0.1)
        result = analyze_latent_influence(predict, self.track, quiet, [3], self.settings)
        self.assertLess(result["dimensions"][0]["score"], 0.001)

    def test_constant_track_has_no_observed_effect_but_can_have_margin_response(self):
        track = np.ones((3, 2), dtype=np.float32)
        result = self.analyze(lambda z: repeat_spectrum(z[:, 1] * 0.2), track)
        self.assertEqual(result["input_order"], [0, 1])
        self.assertEqual(result["dimensions"][1]["score"], 0)
        self.assertGreater(result["dimensions"][1]["playable_peak_effect"], 0)

    def test_reference_selection_covers_sources_without_duplicates(self):
        track = np.arange(40, dtype=np.float32).reshape(20, 2)
        indices = select_reference_indices(track, [3, 17], 8)
        self.assertTrue(any(indices < 3))
        self.assertTrue(any(indices >= 3))
        self.assertEqual(len(set(indices)), 8)
        np.testing.assert_array_equal(indices, select_reference_indices(track, [3, 17], 8))

    def test_rejects_invalid_configuration_and_nonfinite_results(self):
        with self.assertRaises(ValueError):
            InfluenceSettings(0, 129)
        with self.assertRaises(ValueError):
            self.analyze(lambda z: repeat_spectrum(np.full(len(z), np.nan)))
        with self.assertRaises(ValueError):
            select_reference_indices(self.track, [2], 3)

    def test_sweep_has_distinct_normalized_positions_at_large_coordinate_offsets(self):
        coordinates, positions = sweep_positions(
            {"min": "100000", "max": "100001"}, [100000.1, 100000.9], 257,
        )
        self.assertTrue(np.all(np.diff(coordinates) > 0))
        self.assertTrue(np.all(np.diff(positions) > 0))
        self.assertEqual((positions[0], positions[-1]), (0, 1))


if __name__ == "__main__":
    unittest.main()
