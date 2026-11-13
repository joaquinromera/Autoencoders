# Train a model, export its automation and play it with a gamepad

Every block runs from the repository root. It uses `Extras/tensorflow/debussy.wav`
and the TensorFlow venv at `Extras/tensorflow/venv` (Python 3.8).

## 1. Train a model on debussy.wav

```bash
git submodule update --init --recursive   # frugally-deep is needed to write data.json

mkdir -p Extras/generated/debussy
(cd Extras/generated/debussy && \
  ../../tensorflow/venv/bin/python ../../tensorflow/generate_and_save_models_from_audio.py \
    -a ../../tensorflow/debussy.wav \
    -e 200)
```

- Writes `data.json` (load it in the app), `automation.json`, `decoder/` and `logs/`
  into `Extras/generated/debussy`.
- Only the first 2 minutes of the audio are used.
- After training, inputs are ordered by estimated spectral influence and typical
  response profiles are exported. See [analysis options and metadata](latent_analysis.md).
- Generate MIDI from this export's `automation.json`; an older MIDI file can use a
  different latent order.
- If training fails during the first epoch with a shape error, run it again. A bad
  random initialization occasionally trips it.

## 2. Export the automation as a MIDI file

```bash
Extras/tensorflow/venv/bin/python Extras/tensorflow/send_automation_as_midi.py \
  --automation Extras/generated/debussy/automation.json \
  --output-midi Extras/generated/debussy/automation.mid
```

- `--send-knobs` also writes the model's `xMax` (CC 16) and `sClip` (CC 17) at the start.
- `--port "IAC Driver Bus 1"` streams it live into the app instead of, or as well as, the file.

## 3. Send the gamepad sticks as MIDI

```bash
# Check the MIDI port and that the controller is detected
Extras/tensorflow/venv/bin/python Extras/tensorflow/send_gamepad_xy_as_midi.py --list-ports
Extras/tensorflow/venv/bin/python Extras/tensorflow/send_gamepad_xy_as_midi.py --list-gamepads

# Optional: print raw axis values to find the stick axis numbers (Ctrl+C to stop)
Extras/tensorflow/venv/bin/python Extras/tensorflow/send_gamepad_xy_as_midi.py --monitor-axes

# Optional: calibrate the sticks
Extras/tensorflow/venv/bin/python Extras/tensorflow/send_gamepad_xy_as_midi.py \
  --calibrate Extras/generated/gamepad-calibration.json

# Stream: left stick -> CC 1/2, right stick -> CC 3/4
Extras/tensorflow/venv/bin/python Extras/tensorflow/send_gamepad_xy_as_midi.py \
  --port "IAC Driver Bus 1" \
  --calibration Extras/generated/gamepad-calibration.json \
  --verbose
```

- The IAC Driver must be enabled in Audio MIDI Setup for `IAC Driver Bus 1` to exist.
- If `--monitor-axes` shows the right stick on other axes, pass
  `--right-x-axis N --right-y-axis M`.
- Drop `--calibration` if you skipped calibrating.

## 4. Open the app

```bash
open build/bin/Autoencoders.app
```

- Load `Extras/generated/debussy/data.json`.
- In Settings, check that `IAC Driver Bus 1` is the enabled MIDI input. The app keeps
  only one MIDI input enabled; with none enabled it picks the first one available.
