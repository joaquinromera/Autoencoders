"""Bake an input permutation into the generator's dense decoder weights."""

import numpy as np


def reorder_dense_decoder_inputs(decoder, order):
    """Clone a linear BN/Dense chain so new_input[j] means old_input[order[j]]."""
    from tensorflow.keras.layers import BatchNormalization, Dense, InputLayer
    from tensorflow.keras.models import clone_model

    order = np.asarray(order)
    if (order.ndim != 1 or order.dtype.kind not in "iu"
            or sorted(order.tolist()) != list(range(decoder.input_shape[-1]))):
        raise ValueError("input order must be a permutation of all decoder inputs")
    validate_sequential_decoder(decoder)
    reordered = clone_model(decoder)
    reordered.set_weights(decoder.get_weights())
    for layer in reordered.layers:
        if isinstance(layer, InputLayer):
            continue
        if isinstance(layer, Dense):
            weights = layer.get_weights()
            layer.set_weights([weights[0][order], *weights[1:]])
            return reordered
        if not isinstance(layer, BatchNormalization) or list(layer.axis) not in ([1], [-1]):
            raise ValueError("only feature BatchNormalization may precede the first Dense layer")
        layer.set_weights([weight[order] for weight in layer.get_weights()])
    raise ValueError("decoder must contain a Dense layer")


def validate_sequential_decoder(decoder):
    from tensorflow.keras.layers import InputLayer

    previous = decoder.inputs[0]
    for layer in decoder.layers:
        if not isinstance(layer, InputLayer):
            if layer.input is not previous:
                raise ValueError("input reordering requires a sequential dense decoder")
            previous = layer.output
    if len(decoder.outputs) != 1 or decoder.outputs[0] is not previous:
        raise ValueError("input reordering requires one sequential output")
