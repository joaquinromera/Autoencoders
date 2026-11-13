# Offline ordering of latent controls

The TensorFlow generator analyzes the trained decoder before saving it. Exported
inputs are ordered from greater to lesser estimated spectral influence. The
existing C++ application displays them in that order without analysis or sorting.

The analysis also exports typical response profiles for a future UI. It does not
draw those profiles in the current application.

## Running it

Use the normal training command. Analysis is enabled automatically, including with
`--model_only`. Two optional arguments control its cost:

```bash
../../tensorflow/venv/bin/python ../../tensorflow/generate_and_save_models_from_audio.py \
  -a /absolute/path/to/input.wav -e 200 \
  --analysis-references 16 --analysis-points 129
```

The defaults evaluate approximately 16 × 131 × latent dimensions decoder inputs.
The extra two sweep positions include the observed training endpoints. Inference
runs in batches of 256, one dimension at a time. Increasing sweep resolution helps
find narrow responsive regions; finite sampling can still miss them.

## What influence means

Reference frames are selected deterministically, taking turns between source files
and preferring states distant from those already selected in range-normalized
latent coordinates. With a reference budget smaller than the number of files, only
the first files receive references; increase the budget to cover every source.

For each reference, sweep one dimension while keeping the others fixed. Convert
decoded spectra back to power with the model's `xMax` and `sClip`, matching the
synth's [-120, 120] dB clamp. Average power into 32 mel bands and apply a smooth
-80 dB power floor before taking logarithms. Level changes are retained.

For each reference, compute the span of each band's log power across the observed
training range, then take the RMS across bands. Rank dimensions by the 90th
percentile of these reference scores. A narrow strong response contributes to the
span rather than being averaged over a mostly unresponsive slider. Ties preserve
original dimension order.

The padded playable margins are included in profiles and `playable_peak_effect`,
but excluded from the ranking. A constant training dimension therefore has zero
observed-range influence even if moving into its padding changes the sound.

These are spectral estimates, not calibrated audibility probabilities. They use
the model's default spectral settings and sample rate, before random-phase
resynthesis, overlap-add and output `tanh`. Actual audibility also depends on those
stages, playback conditions and the other controls' positions. Listening validation
on representative trained models is still needed to tune the heuristic.

## Coordinate contract

`decoder/`, `data.json` and `automation.json` use the same **new input order**:

- The decoder's initial batch-normalization vectors and first dense kernel rows
  are permuted. No extra inference layer or custom Keras operation is added.
- `parameters.zRange`, `ztrack` columns and analysis dimensions follow that order.
- MIDI CC 1 controls the first exported dimension, CC 2 the second, and so on.
  Knob CC numbers and frame timing remain unchanged.
- An export-time check compares predictions before and after the permutation.

Regenerate MIDI from the matching `automation.json` when replacing a model with a
newly ordered export. Previously generated MIDI refers to the previous coordinate
order. Existing model bundles are not modified, and consumers may ignore the new
metadata. No changes were made to the separate PyTorch exporter.

## `latent_analysis` schema, version 1

Both JSON files include the same optional top-level object:

| Field | Meaning |
| --- | --- |
| `version`, `method` | Schema version and `log_mel_sweep_v1` algorithm |
| `profile_scope` | `training_reference_summary`; not a live current-state map |
| `input_order` | Zero-based original dimension for each exported input |
| `reference_frame_indices` | Selected rows of the original/reordered trajectory |
| `reference_parameters` | Sample rate, FFT size, span and offset used for analysis |
| `dimensions` | One record per input, already in exported order |

Each dimension record includes:

| Field | Meaning |
| --- | --- |
| `original_dimension` | Zero-based identity before reordering |
| `score` | Ranking score described above |
| `typical_effect`, `peak_effect` | Median and maximum observed-range reference scores |
| `playable_peak_effect` | Maximum reference score over the full padded range |
| `observed_range` | Unpadded training minimum and maximum |
| `positions` | Sample positions normalized to the exported slider range |
| `response_mean`, `response_peak` | Mean/maximum over references of adjacent spectral change, divided by normalized step size |
| `spectral_level_mean_db` | Mean over references of log mean FFT-bin power; not output dBFS |

For P positions, response arrays contain P−1 values, one per interval. Level arrays
contain P values. Draw responses on a shared scale between dimensions; independently
normalizing each curve would exaggerate nearly inactive controls. A flat region
means little local change, not necessarily silence or similarity to another region.

## Verification

Run the Python tests from the repository root with the existing TensorFlow venv:

```bash
PYTHONPATH=Extras/tensorflow TF_CPP_MIN_LOG_LEVEL=2 \
  Extras/tensorflow/venv/bin/python -m unittest discover -s Tests/python -v
```

Tests cover influence ordering, narrow peaks, context dependence, suppression below
the energy floor, latent unit scaling, input-permutation equivalence, SavedModel
round-trips, Frugally Deep conversion, bundle alignment and MIDI quantization.
