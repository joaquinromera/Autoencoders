"""
Compatibility shim to make TensorFlow 2.13's keras work with old Keras 2.x import style.
This allows the original 2020 script to work without modifications.
"""
import sys
import tensorflow.keras as keras

# Make 'keras' importable as a standalone module
sys.modules['keras'] = keras
sys.modules['keras.models'] = keras.models
sys.modules['keras.layers'] = keras.layers
sys.modules['keras.initializers'] = keras.initializers
sys.modules['keras.optimizers'] = keras.optimizers
sys.modules['keras.regularizers'] = keras.regularizers
sys.modules['keras.datasets'] = keras.datasets
sys.modules['keras.callbacks'] = keras.callbacks
sys.modules['keras.utils'] = keras.utils
sys.modules['keras.utils.vis_utils'] = keras.utils  # vis_utils was moved
