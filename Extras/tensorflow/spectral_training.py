"""Train the original dense spectral autoencoder; export is handled separately."""

from datetime import datetime
from glob import glob
from pathlib import Path

import numpy as np


def load_training_spectra(pattern, settings):
    import librosa

    files = sorted(glob(pattern))
    if not files:
        raise FileNotFoundError(f"No audio files matched pattern: {pattern}")
    spectra = [load_spectrum(path, settings, librosa) for path in files]
    combined = np.vstack(spectra)
    maximum = float(combined.max())
    if not np.isfinite(maximum) or maximum <= 0:
        raise ValueError("training audio has no energy above the spectral floor")
    settings["xMax"] = str(maximum)
    return combined / maximum, [str(Path(path).resolve()) for path in files], [len(s) for s in spectra]


def load_spectrum(path, settings, librosa):
    audio, _ = librosa.load(path, sr=settings["sr"], mono=True, duration=120)
    audio = np.trim_zeros(audio)
    if not len(audio) or not np.isfinite(audio).all():
        raise ValueError(f"Audio must contain finite, non-zero samples: {path}")
    spectrum = librosa.stft(audio, n_fft=settings["win_length"], hop_length=settings["hop_length"]).T
    with np.errstate(divide="ignore"):
        decibels = 10 * np.log10(np.abs(spectrum) ** 2)
    return decibels.clip(float(settings["sClip"]), None) - float(settings["sClip"])


def dense_stack(inputs, widths):
    from tensorflow.keras.layers import BatchNormalization, Dense

    output = inputs
    for width in widths:
        output = BatchNormalization()(output)
        output = Dense(width, activation="relu", kernel_initializer="glorot_normal")(output)
    return output


def build_autoencoder(feature_count, latent_dim=8):
    from tensorflow.keras import backend, layers, models, optimizers

    backend.clear_session()
    widths = [512, 256, 128, 64, 32, 16, latent_dim]
    inputs = layers.Input(shape=(feature_count,))
    encoder = models.Model(inputs, dense_stack(inputs, widths))
    latent = layers.Input(shape=(latent_dim,), name="z")
    output = layers.Dense(feature_count, activation="relu")(dense_stack(latent, widths[-2::-1]))
    decoder = models.Model(latent, output, name="decoder")
    autoencoder = models.Model(inputs, decoder(encoder(inputs)), name="autoencoder")
    autoencoder.compile(optimizer=optimizers.Adam(0.002, clipnorm=0.5), loss="mse", metrics=["mse"])
    return autoencoder, encoder, decoder


def train_spectral_autoencoder(spectra, latent_dim, epochs):
    from sklearn.model_selection import train_test_split
    from tensorflow.keras.callbacks import TensorBoard

    training, validation = train_test_split(spectra, test_size=0.05, shuffle=True)
    autoencoder, encoder, decoder = build_autoencoder(spectra.shape[1], latent_dim)
    silence = np.zeros((int(len(training) * 0.1), spectra.shape[1]))
    augmented = np.vstack([silence, training])
    logdir = Path("logs", datetime.now().strftime("%Y%m%d-%H%M%S"))
    autoencoder.fit(augmented, augmented, epochs=epochs, batch_size=128, shuffle=True,
                    validation_data=(validation, validation), callbacks=[TensorBoard(logdir, histogram_freq=1)])
    return decoder, encoder.predict(spectra, batch_size=128, verbose=0)
