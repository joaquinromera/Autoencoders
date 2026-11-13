"""Bundle integration: decoder, trajectories, ranges, profiles and MIDI agree."""

import json
import os
import tempfile
import unittest
from pathlib import Path

import numpy as np
import tensorflow as tf

from latent_influence import InfluenceSettings
from latent_metadata import build_latent_ranges
from latent_midi import parse_z_ranges, resolve_slider_cc_numbers, value_to_midi_cc
from model_bundle import prepare_ranked_bundle, write_bundle_files


class BundleTests(unittest.TestCase):
    def setUp(self):
        inputs = tf.keras.Input(shape=(3,))
        layer = tf.keras.layers.Dense(129)
        self.decoder = tf.keras.Model(inputs, layer(inputs))
        layer.set_weights([np.repeat([[0], [0.02], [0.2]], 129, axis=1), np.full(129, 0.5)])
        self.track = np.array([[-1, -2, -3], [0, 0, 0], [1, 2, 3]], dtype=np.float32)
        self.parameters = {"win_length": 256, "hop_length": 64, "sr": 22050,
                           "xMax": "40", "sClip": "-40", "latent_dim": 3}

    def prepare(self):
        return prepare_ranked_bundle(self.decoder, self.track, self.parameters,
                                      ["example.wav"], [3], InfluenceSettings(3, 33))

    def test_reorders_every_coordinate_bearing_part_of_bundle(self):
        decoder, payload = self.prepare()
        order = payload["latent_analysis"]["input_order"]
        self.assertEqual(order, [2, 1, 0])
        np.testing.assert_array_equal(payload["ztrack"], self.track[:, order])
        original_ranges = build_latent_ranges(self.track)
        self.assertEqual(payload["parameters"]["zRange"], [original_ranges[i] for i in order])
        self.assertEqual([d["original_dimension"] for d in payload["latent_analysis"]["dimensions"]], order)
        np.testing.assert_allclose(decoder(np.array(payload["ztrack"])), self.decoder(self.track), atol=1e-6)
        self.assertEqual(payload["automation"]["frame_period_seconds"], 64 / 22050)
        self.assertNotIn("zRange", self.parameters)
        json.dumps(payload, allow_nan=False)

    def test_midi_quantization_preserves_reordered_trajectory_coordinates(self):
        _, payload = self.prepare()
        ccs = resolve_slider_cc_numbers(payload, 3)
        self.assertEqual(ccs, [1, 2, 3])
        ranges = parse_z_ranges(payload["parameters"])
        original_ranges = parse_z_ranges({"zRange": build_latent_ranges(self.track)})
        order = payload["latent_analysis"]["input_order"]
        for original, ranked in zip(self.track, payload["ztrack"]):
            before = [value_to_midi_cc(value, *bounds) for value, bounds in zip(original, original_ranges)]
            after = [value_to_midi_cc(value, *bounds) for value, bounds in zip(ranked, ranges)]
            self.assertEqual(after, [before[i] for i in order])

    def test_saved_bundle_converts_with_the_existing_fdeep_exporter(self):
        decoder, payload = self.prepare()
        directory = Path(tempfile.mkdtemp(prefix="autoencoders-bundle-"))
        previous = Path.cwd()
        try:
            os.chdir(directory)
            decoder.save("decoder")
            write_bundle_files(payload)
            automation = json.loads(Path("automation.json").read_text())
            data = json.loads(Path("data.json").read_text())
            self.assertEqual(automation, payload)
            self.assertEqual({key: value for key, value in data.items() if key != "model"}, payload)
            self.assertEqual(data["model"]["input_shapes"], [[3]])
        finally:
            os.chdir(previous)

    def test_ranges_remain_finite_for_constant_dimensions(self):
        ranges = build_latent_ranges(np.array([[0, 10], [0, 10]]))
        for bounds in ranges:
            self.assertLess(float(bounds["min"]), float(bounds["max"]))
        for invalid in (np.empty((0, 2)), np.array([[np.nan]])):
            with self.assertRaises(ValueError):
                build_latent_ranges(invalid)


if __name__ == "__main__":
    unittest.main()
