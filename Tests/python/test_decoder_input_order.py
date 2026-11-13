"""Reordered inputs must preserve sound through normalization and serialization."""

import tempfile
import unittest
from pathlib import Path

import numpy as np
import tensorflow as tf

from decoder_input_order import reorder_dense_decoder_inputs


def normalized_decoder():
    inputs = tf.keras.Input(shape=(3,))
    norm = tf.keras.layers.BatchNormalization()
    dense = tf.keras.layers.Dense(4, activation="relu")
    model = tf.keras.Model(inputs, dense(norm(inputs)))
    norm.set_weights([np.array([2, 3, 4]), np.array([0.5, -1, 2]),
                      np.array([-2, 1, 3]), np.array([4, 9, 16])])
    dense.set_weights([np.arange(12).reshape(3, 4) / 10, np.arange(4)])
    return model


class DecoderOrderTests(unittest.TestCase):
    def test_non_self_inverse_permutation_preserves_arbitrary_outputs(self):
        decoder = normalized_decoder()
        values = np.random.default_rng(1).uniform(-4, 5, (40, 3)).astype(np.float32)
        original_weights = [w.copy() for w in decoder.get_weights()]
        order = [2, 0, 1]
        reordered = reorder_dense_decoder_inputs(decoder, order)
        np.testing.assert_allclose(reordered(values[:, order]), decoder(values), rtol=1e-6, atol=1e-6)
        self.assertEqual(len(reordered.layers), len(decoder.layers))
        for before, after in zip(original_weights, decoder.get_weights()):
            np.testing.assert_array_equal(before, after)

    def test_saved_model_retains_reordered_input_contract(self):
        decoder = normalized_decoder()
        order = [1, 2, 0]
        reordered = reorder_dense_decoder_inputs(decoder, order)
        path = Path(tempfile.mkdtemp(prefix="autoencoders-order-")) / "decoder"
        reordered.save(path)
        loaded = tf.keras.models.load_model(path, compile=False)
        values = np.array([[0.2, -0.5, 1.2]], dtype=np.float32)
        np.testing.assert_allclose(loaded(values[:, order]), decoder(values), rtol=1e-6, atol=1e-6)

    def test_rejects_incomplete_duplicate_and_fractional_orders(self):
        for order in ([0, 1], [0, 0, 2], [0.0, 1.0, 2.0]):
            with self.assertRaises(ValueError):
                reorder_dense_decoder_inputs(normalized_decoder(), order)

    def test_rejects_an_unsupported_input_transform(self):
        inputs = tf.keras.Input(shape=(3,))
        hidden = tf.keras.layers.Activation("relu")(inputs)
        decoder = tf.keras.Model(inputs, tf.keras.layers.Dense(2)(hidden))
        with self.assertRaises(ValueError):
            reorder_dense_decoder_inputs(decoder, [2, 0, 1])

    def test_rejects_raw_input_skip_connection_after_first_dense(self):
        inputs = tf.keras.Input(shape=(3,))
        hidden = tf.keras.layers.Dense(3)(inputs)
        decoder = tf.keras.Model(inputs, tf.keras.layers.Add()([hidden, inputs]))
        with self.assertRaises(ValueError):
            reorder_dense_decoder_inputs(decoder, [2, 0, 1])


if __name__ == "__main__":
    unittest.main()
