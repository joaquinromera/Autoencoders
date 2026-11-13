# Autoencoders

## Requirements

### Common

- `git` (with submodule support; clone with `--recurse-submodules`)
- `cmake` >= 3.14
- A C++20-capable compiler
- `curl` and `unzip` (used by `build_pytorch.sh` to fetch and extract LibTorch)
- Network access on the first CMake configure (for the pinned `vcpkg` checkout fetched via `FetchContent`) and when running `build_pytorch.sh`
- Python (the helper scripts under `Extras/` pin two separate interpreters):
    - Python **3.8.x** for `Extras/tensorflow/` (TensorFlow 2.13, `numpy<1.24`, `librosa`, `scipy`, `scikit-learn`, `h5py`, `matplotlib`, `mido`, `python-rtmidi`, `yt-dlp`, `pygame`, `tqdm`) — see `Extras/tensorflow/requirements.txt`
    - Python **3.12.x** for `Extras/pytorch/` (`torch==2.2.0`, `torchaudio==2.2.0`, `pytorch_lightning`, `librosa`, `numpy<2.0`, `pandas`, `scikit-learn`, `matplotlib`, `fire`, `tensorboard`) — see `Extras/pytorch/requirements.txt`
- Optional developer tooling referenced by the build: `clang-tidy`, `clang-format`, `cppcheck`, `gcovr` (only needed for the corresponding targets / `ENABLE_CLANG_TIDY` / `ENABLE_COVERAGE`)

### macOS

- macOS on Apple Silicon (`arm64`) or Intel (`x86_64`)
- Xcode Command Line Tools (`xcode-select --install`) — provides `clang`/`clang++` with C++20, `git`, `codesign`, and `install_name_tool` (the last two are used by `build_pytorch.sh` to re-link and re-sign the LibTorch dylibs)
- Homebrew (recommended) for installing `cmake` and the optional tooling above:
  ```bash
  brew install cmake
  brew install --cask --no-quarantine temurin  # only if your toolchain needs a JDK; not required for the base build
  brew install llvm cppcheck gcovr             # optional: clang-tidy / clang-format / static analysis / coverage
  ```

### Linux (x86_64)

- A C++20 compiler: `g++` >= 10 or `clang++` >= 14
- Base toolchain: `git`, `cmake` >= 3.14, `ninja-build` (optional), `pkg-config`, `curl`, `unzip`, `python3`
- JUCE runtime/development libraries. On Debian/Ubuntu:
  ```bash
  sudo apt update
  sudo apt install -y \
      build-essential git cmake pkg-config curl unzip \
      libasound2-dev libjack-jackd2-dev \
      libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev \
      libxcomposite-dev libxrender-dev \
      libfreetype6-dev libfontconfig1-dev \
      libglu1-mesa-dev mesa-common-dev \
      libcurl4-openssl-dev \
      libwebkit2gtk-4.1-dev
  ```
  (On older distros substitute `libwebkit2gtk-4.0-dev`.)
- For the Python helpers, install both Python 3.8 and 3.12 (e.g. via `pyenv` or `deadsnakes`), since the two `Extras/` environments pin different interpreters.

## Build

This project uses:

- `vcpkg` for JUCE, frugally-deep, and GoogleTest
- a repo-local LibTorch install for the TorchScript backend

### Clean macOS arm64 build

```bash
git clone --recurse-submodules https://github.com/joaquinromera/autoencoders.git
cd autoencoders
./build_pytorch.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build -j12
ctest --test-dir build --output-on-failure
```

The app bundle is generated at `build/bin/Autoencoders.app`.

### Notes

- `./build_pytorch.sh` downloads the official CPU-only LibTorch archive into `./pytorch-install`.
- The script auto-detects your platform (macOS arm64, macOS x86_64, Linux x86_64) and downloads the matching prebuilt LibTorch archive; for other platforms it falls back to building PyTorch from source. If you already have a compatible LibTorch install elsewhere, set `Torch_DIR` when configuring CMake instead of running the script.
- `BUILD_TESTS` defaults to `OFF`. Use `-DBUILD_TESTS=ON` when you want the `PresenterTests` and `AutoencoderTests` targets plus `ctest` coverage of the unit tests.
- The first configure needs network access so CMake can fetch the pinned `vcpkg` checkout.
- If you cloned without `--recurse-submodules`, initialise them before running the Python scripts: `git submodule update --init --recursive`.
- PyTorch C++ install reference: https://pytorch.org/cppdocs/installing.html

Generate an app-loadable `data.json` model bundle from a wave file with
`./Extras/tensorflow/generate_and_save_models_from_audio.py`. By default the script now
uses a `512`-sample hop and a `2048`-sample FFT window so the exported model
matches the current runtime synth settings.

Example:

```bash
mkdir -p Extras/generated/my_model
cd Extras/generated/my_model
../../tensorflow/venv/bin/python ../../tensorflow/generate_and_save_models_from_audio.py \
  -a /absolute/path/to/input.wav \
  -e 1
```

The script writes `automation.json`, `data.json`, `decoder/`, and `logs/` into the current working directory.
The TensorFlow export orders decoder inputs by estimated spectral influence and
stores typical response profiles in `latent_analysis`. Decoder weights, slider ranges
and automation coordinates are reordered together; C++ performs no analysis when loading.
See [offline latent analysis](Extras/tensorflow/latent_analysis.md) for the scoring
method, analysis options and metadata format. Regenerate MIDI from the matching
`automation.json` when switching to a newly ordered export.
`automation.json` contains the encoder-derived latent trajectory (`ztrack`) plus timing metadata, so it can be replayed
later as MIDI CC automation.

Replay that automation through a MIDI output port or export it as a `.mid` file with
`./Extras/tensorflow/send_automation_as_midi.py`.

Example:

```bash
../../tensorflow/venv/bin/python ../../tensorflow/send_automation_as_midi.py \
  --automation ./automation.json \
  --output-midi ./automation.mid
```

For live playback into the synth, route the script into a virtual MIDI output that the app is listening to. On macOS,
the built-in IAC Driver is the simplest option. By default the replay script uses the synth's current CC layout:

- latent sliders: `CC 1..N` (or the model file's `automation.default_slider_cc`)
- `xMax`: `CC 16`
- `sClip`: `CC 17`
- map A x/y: `CC 20/21`, map B x/y: `CC 22/23` (a centred controller sits on the piece's mean)

Press `M` (or the MIDI map button) to show every control's CC; click a tag and move a controller to bind it.

For real-time control from a PlayStation/Xbox-style controller, use
`./Extras/tensorflow/send_gamepad_xy_as_midi.py`. By default it maps the left stick to `CC 1/2` and the right stick
to `CC 3/4`, which drive the first four latent sliders with the synth's default CC layout. Pass
`--left-x-cc 20 --left-y-cc 21` to steer map A with the left stick instead.

This script is a stripped down version of the Jupyter notebook `./Extras/notebooks/Audio_Autoencoders.ipynb`.

---

## Tensorflow in C++

* https://itnext.io/how-to-use-your-c-muscle-using-tensorflow-2-0-and-xcode-without-using-bazel-builds-9dc82d5e7f80
* https://itnext.io/creating-a-tensorflow-dnn-in-c-part-1-54ce69bbd586
* https://medium.com/@hamedmp/exporting-trained-tensorflow-models-to-c-the-right-way-cf24b609d183

---

## Keras to c++

* https://stackoverflow.com/questions/36720498/convert-keras-model-to-c
* https://github.com/Dobiasd/frugally-deep
* https://github.com/gosha20777/keras2cpp
* https://github.com/moof2k/kerasify
* https://medium.com/@htmbx6/build-and-train-neural-network-with-tensorflow-c-f13f22d3c5b6
* https://medium.com/analytics-vidhya/deploying-tensorflow-2-1-as-c-c-executable-1d090845055c
