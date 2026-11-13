# -*- coding: utf-8 -*-

"""Train a spectral autoencoder and export inputs ranked by decoded influence."""

import argparse


def make_autoencoder(filename, epochs=1, hop=None, win=None, analysis_settings=None):
    """Train, analyze and save a decoder with consistently reordered metadata."""
    from model_bundle import save_ranked_decoder
    from spectral_training import load_training_spectra, train_spectral_autoencoder

    settings = {"sClip": "-60.0", "sr": 22050, "hop_length": hop or 512,
                "win_length": win or 2048, "latent_dim": 8}
    spectra, source_files, frame_counts = load_training_spectra(filename, settings)
    decoder, latent_track = train_spectral_autoencoder(spectra, settings["latent_dim"], epochs)
    print("Analyzing latent influence and response profiles...")
    return save_ranked_decoder(decoder, latent_track, settings, source_files, frame_counts, analysis_settings)


def parse_arguments():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-a", "--audio", required=True, help="Audio file or glob pattern")
    parser.add_argument("-e", "--epochs", required=True, type=int, help="Training epochs")
    parser.add_argument("-j", "--hop", type=int, help="STFT hop length (default: 512)")
    parser.add_argument("-w", "--win", type=int, help="FFT window size (default: 2048)")
    parser.add_argument("-m", "--model_only", action="store_true", help="Skip Frugally Deep conversion")
    parser.add_argument("--analysis-references", type=int, default=16, help="Reference latent states (default: 16)")
    parser.add_argument("--analysis-points", type=int, default=129, help="Sweep points per dimension (default: 129)")
    return parser.parse_args()


def main():
    from latent_influence import InfluenceSettings
    from model_bundle import write_bundle_files

    args = parse_arguments()
    analysis = InfluenceSettings(args.analysis_references, args.analysis_points)
    payload = make_autoencoder(args.audio, args.epochs, args.hop, args.win, analysis)
    write_bundle_files(payload, args.model_only)
    print("Saved decoder/ and automation.json." if args.model_only else "Saved decoder/, automation.json and data.json.")


if __name__ == "__main__":
    main()
