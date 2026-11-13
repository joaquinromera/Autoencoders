"""Prepare and write a ranked decoder bundle for the existing synth and MIDI tools."""

import json
import os
import sys
from pathlib import Path

import numpy as np

from decoder_input_order import reorder_dense_decoder_inputs
from latent_influence import analyze_latent_influence
from latent_metadata import build_automation_metadata, build_latent_ranges, validate_latent_track


def prepare_ranked_bundle(decoder, latent_track, parameters, source_files, source_frame_counts,
                          analysis_settings=None):
    track = validate_latent_track(latent_track)
    if int(parameters["latent_dim"]) != track.shape[1] or len(source_files) != len(source_frame_counts):
        raise ValueError("latent dimension and source metadata must match the trajectory")
    parameters = dict(parameters, zRange=build_latent_ranges(track))
    predict = lambda values: decoder.predict(values, batch_size=256, verbose=0)
    analysis = analyze_latent_influence(predict, track, parameters, source_frame_counts, analysis_settings)
    order = analysis["input_order"]
    ranked_decoder = reorder_dense_decoder_inputs(decoder, order)
    verify_reordered_decoder(decoder, ranked_decoder, track, order, analysis["reference_frame_indices"])
    return ranked_decoder, assemble_ranked_payload(track, parameters, analysis, source_files, source_frame_counts)


def assemble_ranked_payload(track, parameters, analysis, source_files, source_frame_counts):
    order = analysis["input_order"]
    ranked_track = track[:, order]
    parameters = dict(parameters, zRange=[parameters["zRange"][index] for index in order])
    ranked_analysis = dict(analysis, dimensions=[analysis["dimensions"][index] for index in order])
    return {
        "parameters": parameters,
        "ztrack": ranked_track.tolist(),
        "automation": build_automation_metadata(parameters, ranked_track, source_files, source_frame_counts),
        "latent_analysis": ranked_analysis,
    }


def verify_reordered_decoder(original, reordered, track, order, reference_indices):
    """Fail export if the reordered decoder changes reference reconstructions."""
    reference = track[reference_indices]
    before = original(reference, training=False).numpy()
    after = reordered(reference[:, order], training=False).numpy()
    np.testing.assert_allclose(after, before, rtol=1e-5, atol=1e-6,
                               err_msg="input permutation changed decoder output")


def save_ranked_decoder(decoder, latent_track, parameters, source_files, source_frame_counts,
                        analysis_settings=None):
    ranked, payload = prepare_ranked_bundle(
        decoder, latent_track, parameters, source_files, source_frame_counts, analysis_settings,
    )
    # Validate metadata before saving the corresponding decoder.
    json.dumps(payload, allow_nan=False)
    ranked.save("decoder")
    print("Exported decoder input order (original zero-based dimensions):",
          payload["latent_analysis"]["input_order"])
    return payload


def write_bundle_files(payload, model_only=False):
    """Both JSON files contain the same ordered trajectory, ranges and analysis."""
    write_json("automation.json", payload)
    if not model_only:
        converter = load_fdeep_converter()
        model = converter.load_model("./decoder")
        bundle = dict(payload, model=converter.model_to_fdeep_json(model, True))
        write_json("data.json", bundle)


def write_json(path, payload):
    encoded = json.dumps(payload, ensure_ascii=False, indent=2, allow_nan=False, sort_keys=True)
    Path(path).write_text(encoded + "\n", encoding="utf-8")


def load_fdeep_converter():
    export_path = (Path(os.environ["FDEEP_PATH"]) if os.environ.get("FDEEP_PATH") else
                   Path(__file__).resolve().parents[2] / "3rdParty/frugally-deep/keras_export")
    if not export_path.exists():
        raise FileNotFoundError(f"Missing {export_path}; initialize submodules or set FDEEP_PATH")
    sys.path.insert(0, str(export_path))
    import convert_model
    return convert_model
