#!/usr/bin/env bash
#
# Train a matrix of autoencoder models that vary one STFT/training parameter at
# a time, render each model's latent-automation reconstruction to audio, and
# stage the comparison figures (combined spectrogram grids) + audio + manifest
# for the thesis "Resultados" chapter.
#
# Variants (hop = win/4 to hold the 75% overlap constant):
#   res-ep010           win 2048  hop 512   epochs 10    (epochs axis)
#   res-ep030           win 2048  hop 512   epochs 30    (epochs axis)
#   res-ep050           win 2048  hop 512   epochs 50    (epochs axis)
#   res-ep200           win 2048  hop 512   epochs 200   (epochs axis)
#   res-win2048-ep500   win 2048  hop 512   epochs 500   (shared baseline)
#   res-win512          win 512   hop 128   epochs 500   (window axis)
#   res-win1024         win 1024  hop 256   epochs 500   (window axis)
#   res-win4096         win 4096  hop 1024  epochs 500   (window axis)
#
# Idempotent: skips the download if the source exists, skips training a variant
# whose data.json exists, and skips reconstructing one whose reconstruction.wav
# exists. Set FORCE=1 to redo everything.
#
# Env overrides: SRC_URL, SRC_SS (start offset s), SRC_T (length s), FORCE.

set -eo pipefail

# ---- config ----------------------------------------------------------------
SRC_URL="${SRC_URL:-ytsearch1:Johann Sebastian Bach Toccata and Fugue in D minor BWV 565 organ}"
SRC_SS="${SRC_SS:-0}"
SRC_T="${SRC_T:-45}"

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
TF_DIR="$REPO/Extras/tensorflow"
PY="$TF_DIR/venv/bin/python"
GEN="$REPO/Extras/generated"
THESIS="${THESIS:-$REPO/../thesis}"
FIG="$THESIS/figures"
AUD="$THESIS/audio"
SRCDIR="$GEN/_source"
SRC="$SRCDIR/source.wav"

mkdir -p "$GEN" "$AUD" "$SRCDIR"

# ---- 1. source download + trim (idempotent) --------------------------------
if [ ! -f "$SRC" ]; then
  echo "### downloading source ..."
  "$PY" "$TF_DIR/download_audio_from_youtube.py" "$SRC_URL" \
    -o "$SRCDIR" -n source_full --sample-rate 22050 --mono
  echo "### trimming source to ${SRC_T}s (offset ${SRC_SS}s) ..."
  ffmpeg -y -ss "$SRC_SS" -t "$SRC_T" -i "$SRCDIR/source_full.wav" \
    -ar 22050 -ac 1 "$SRC"
else
  echo "### source exists, skipping download: $SRC"
fi

# ---- 2. training + reconstruction matrix -----------------------------------
names=(res-ep010 res-ep030 res-ep050 res-ep200 res-win2048-ep500 res-win512 res-win1024 res-win4096)
wins=(2048 2048 2048 2048 2048 512 1024 4096)
hops=(512 512 512 512 512 128 256 1024)
eps=(10 30 50 200 500 500 500 500)

for i in "${!names[@]}"; do
  name="${names[$i]}"; win="${wins[$i]}"; hop="${hops[$i]}"; ep="${eps[$i]}"
  d="$GEN/$name"
  mkdir -p "$d"
  if [ -f "$d/data.json" ] && [ -z "${FORCE:-}" ]; then
    echo "### [$name] already trained, skipping (set FORCE=1 to retrain)"
  else
    # The training script uses an unseeded split + glorot_normal init, so a bad
    # initialization occasionally trips the legacy Adam clipnorm with a graph
    # shape error. Failures happen fast (first epoch), so retry a few times.
    ok=0
    for attempt in 1 2 3 4 5; do
      echo "### [$name] training win=$win hop=$hop epochs=$ep (attempt $attempt) ..."
      if ( cd "$d" && "$PY" "$TF_DIR/generate_and_save_models_from_audio.py" \
            -a "$SRC" -e "$ep" -w "$win" -j "$hop" ) > "$d/train.log" 2>&1; then
        ok=1; break
      fi
      echo "### [$name] attempt $attempt failed (tail of $d/train.log):"
      tail -3 "$d/train.log" || true
    done
    if [ "$ok" -ne 1 ]; then
      echo "### [$name] training FAILED after 5 attempts" >&2
      exit 1
    fi
  fi
  if [ -f "$d/reconstruction.wav" ] && [ -z "${FORCE:-}" ]; then
    echo "### [$name] reconstruction exists, skipping"
  else
    echo "### [$name] reconstructing ..."
    "$PY" "$TF_DIR/reconstruct_from_model.py" --model-dir "$d" \
      --out "$d/reconstruction.wav" --metrics-json "$d/metrics.json"
  fi
done

# ---- 3. stage audio (for upload) -------------------------------------------
cp "$GEN/res-ep010/reconstruction.wav"         "$AUD/recon-ep010.wav"
cp "$GEN/res-ep030/reconstruction.wav"         "$AUD/recon-ep030.wav"
cp "$GEN/res-ep050/reconstruction.wav"         "$AUD/recon-ep050.wav"
cp "$GEN/res-ep200/reconstruction.wav"         "$AUD/recon-ep200.wav"
cp "$GEN/res-win2048-ep500/reconstruction.wav" "$AUD/recon-ep500.wav"
cp "$GEN/res-win2048-ep500/reconstruction.wav" "$AUD/recon-win2048.wav"
cp "$GEN/res-win512/reconstruction.wav"        "$AUD/recon-win512.wav"
cp "$GEN/res-win1024/reconstruction.wav"       "$AUD/recon-win1024.wav"
cp "$GEN/res-win4096/reconstruction.wav"       "$AUD/recon-win4096.wav"
cp "$SRC" "$AUD/source.wav"

# ---- 4. figures: source + combined grids -----------------------------------
"$PY" "$TF_DIR/render_spectrogram.py" --wav "$SRC" --out "$FIG/res-source.png" --title ""

"$PY" "$TF_DIR/render_grid.py" --out "$FIG/res-ep-grid.png" --fig-height 12.5 \
  --panel "$AUD/recon-ep010.wav::10 épocas" \
  --panel "$AUD/recon-ep030.wav::30 épocas" \
  --panel "$AUD/recon-ep050.wav::50 épocas" \
  --panel "$AUD/recon-ep200.wav::200 épocas" \
  --panel "$AUD/recon-ep500.wav::500 épocas"

"$PY" "$TF_DIR/render_grid.py" --out "$FIG/res-win-grid.png" --fig-height 12.5 \
  --panel "$AUD/recon-win512.wav::Ventana 512 / hop 128" \
  --panel "$AUD/recon-win1024.wav::Ventana 1024 / hop 256" \
  --panel "$AUD/recon-win2048.wav::Ventana 2048 / hop 512 (por defecto)" \
  --panel "$AUD/recon-win4096.wav::Ventana 4096 / hop 1024"

# ---- 5. manifest + metrics summary -----------------------------------------
"$PY" - "$GEN" "$AUD" <<'PY'
import json, sys
from pathlib import Path

gen, aud = Path(sys.argv[1]), Path(sys.argv[2])
rows = [
    ("res-ep010",         "recon-ep010.wav",  "Reconstruccion con 10 epocas (muy poco entrenamiento)"),
    ("res-ep030",         "recon-ep030.wav",  "Reconstruccion con 30 epocas"),
    ("res-ep050",         "recon-ep050.wav",  "Reconstruccion con 50 epocas"),
    ("res-ep200",         "recon-ep200.wav",  "Reconstruccion con 200 epocas"),
    ("res-win2048-ep500", "recon-ep500.wav",  "Reconstruccion con 500 epocas (bien entrenado)"),
    ("res-win512",        "recon-win512.wav", "Ventana FFT 512 / hop 128 (mucha resolucion temporal)"),
    ("res-win1024",       "recon-win1024.wav","Ventana FFT 1024 / hop 256 (mayor resolucion temporal)"),
    ("res-win2048-ep500", "recon-win2048.wav","Ventana FFT 2048 / hop 512 (configuracion por defecto)"),
    ("res-win4096",       "recon-win4096.wav","Ventana FFT 4096 / hop 1024 (mayor resolucion frecuencial)"),
]

def mse_of(name):
    path = gen / name / "metrics.json"
    if path.exists():
        return json.load(open(path)).get("mse")
    return None

lines = [
    "# Audio de la comparacion de parametros de entrenamiento",
    "",
    "Subi estos WAV (por ejemplo a YouTube o Drive) y reemplaza los marcadores",
    "`PENDIENTE-*` por los enlaces reales en `07-resultados.tex`.",
    "",
    "| Archivo | Descripcion | MSE recon. | Enlace |",
    "|---|---|---|---|",
]
for name, wav, desc in rows:
    m = mse_of(name)
    mtxt = f"{m:.5f}" if isinstance(m, float) else "—"
    slot = "PENDIENTE-" + wav.replace("recon-", "").replace(".wav", "")
    lines.append(f"| `{wav}` | {desc} | {mtxt} | {slot} |")
lines.append("")

(aud / "MANIFEST.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
print("wrote", aud / "MANIFEST.md")
print("\nReconstruction MSE by model:")
for name in ["res-ep010", "res-ep030", "res-ep050", "res-ep200", "res-win2048-ep500", "res-win512", "res-win1024", "res-win4096"]:
    print(f"  {name:20s} {mse_of(name)}")
PY

echo "### DONE."
