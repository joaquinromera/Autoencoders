# -*- coding: utf-8 -*-

"""Replay exported latent automation as MIDI CC or write it to a MIDI file."""

import argparse
import json
import time
from pathlib import Path

from latent_midi import (
    DEFAULT_SCLIP_RANGE,
    DEFAULT_XMAX_RANGE,
    coerce_float,
    parse_z_ranges,
    resolve_frame_period_seconds,
    resolve_knob_cc_numbers,
    resolve_slider_cc_numbers,
    value_to_midi_cc,
)


def _import_mido():
    try:
        import mido
    except ImportError as exc:
        raise RuntimeError(
            "This script requires the 'mido' package. Install Extras/tensorflow/requirements.txt first. "
            "Live MIDI output also needs 'python-rtmidi'."
        ) from exc

    return mido


def _load_payload(path):
    with open(path, 'r', encoding='utf-8') as handle:
        return json.load(handle)


def _validate_payload(payload):
    if "parameters" not in payload:
        raise ValueError("automation payload is missing the top-level 'parameters' object")

    if "ztrack" not in payload:
        raise ValueError("automation payload is missing the top-level 'ztrack' array")

    ztrack = payload["ztrack"]
    if not isinstance(ztrack, list) or not ztrack:
        raise ValueError("ztrack must be a non-empty list of latent frames")

    z_ranges = parse_z_ranges(payload["parameters"])
    if not z_ranges:
        raise ValueError("parameters.zRange is missing or empty")

    latent_dim = len(z_ranges)
    for frame_index, frame in enumerate(ztrack):
        if len(frame) != latent_dim:
            raise ValueError(
                f"ztrack frame {frame_index} has {len(frame)} values, expected {latent_dim}"
            )

    return ztrack, z_ranges, latent_dim


def _build_knob_messages(payload, enabled):
    if not enabled:
        return []

    parameters = payload["parameters"]
    x_max_cc, s_clip_cc = resolve_knob_cc_numbers(payload)
    messages = []

    if "xMax" in parameters:
        messages.append(
            (x_max_cc, value_to_midi_cc(coerce_float(parameters["xMax"]), *DEFAULT_XMAX_RANGE))
        )

    if "sClip" in parameters:
        messages.append(
            (s_clip_cc, value_to_midi_cc(coerce_float(parameters["sClip"]), *DEFAULT_SCLIP_RANGE))
        )

    return messages


def _build_slider_frame_messages(ztrack, z_ranges, slider_cc_numbers, force_all_cc):
    previous_values = [None] * len(slider_cc_numbers)
    frame_messages = []

    for frame_index, frame in enumerate(ztrack):
        current_messages = []

        for slider_index, (value, (minimum, maximum), controller_number) in enumerate(
            zip(frame, z_ranges, slider_cc_numbers)
        ):
            cc_value = value_to_midi_cc(value, minimum, maximum)
            if force_all_cc or frame_index == 0 or previous_values[slider_index] != cc_value:
                current_messages.append((controller_number, cc_value))
                previous_values[slider_index] = cc_value

        frame_messages.append(current_messages)

    return frame_messages


def _write_midi_file(output_path, channel, knob_messages, frame_messages, frame_period_seconds):
    mido = _import_mido()
    midi_file = mido.MidiFile(ticks_per_beat=480)
    track = mido.MidiTrack()
    midi_file.tracks.append(track)

    tempo = mido.bpm2tempo(120)
    track.append(mido.MetaMessage('set_tempo', tempo=tempo, time=0))

    for controller_number, controller_value in knob_messages:
        track.append(
            mido.Message(
                'control_change',
                channel=channel,
                control=controller_number,
                value=controller_value,
                time=0,
            )
        )

    pending_seconds = 0.0

    for frame_index, messages in enumerate(frame_messages):
        if frame_index > 0:
            pending_seconds += frame_period_seconds

        wrote_message = False

        for controller_number, controller_value in messages:
            delta_seconds = pending_seconds if not wrote_message else 0.0
            delta_ticks = int(round(mido.second2tick(delta_seconds, midi_file.ticks_per_beat, tempo)))
            track.append(
                mido.Message(
                    'control_change',
                    channel=channel,
                    control=controller_number,
                    value=controller_value,
                    time=delta_ticks,
                )
            )
            wrote_message = True

        if wrote_message:
            pending_seconds = 0.0

    end_delta_ticks = int(round(mido.second2tick(pending_seconds, midi_file.ticks_per_beat, tempo)))
    track.append(mido.MetaMessage('end_of_track', time=end_delta_ticks))
    midi_file.save(output_path)


def _send_live(port_name, channel, knob_messages, frame_messages, frame_period_seconds, loop_forever):
    mido = _import_mido()

    with mido.open_output(port_name) as port:
        while True:
            for controller_number, controller_value in knob_messages:
                port.send(
                    mido.Message(
                        'control_change',
                        channel=channel,
                        control=controller_number,
                        value=controller_value,
                    )
                )

            next_frame_time = time.monotonic()

            for frame_index, messages in enumerate(frame_messages):
                if frame_index > 0:
                    next_frame_time += frame_period_seconds
                    sleep_duration = next_frame_time - time.monotonic()
                    if sleep_duration > 0.0:
                        time.sleep(sleep_duration)

                for controller_number, controller_value in messages:
                    port.send(
                        mido.Message(
                            'control_change',
                            channel=channel,
                            control=controller_number,
                            value=controller_value,
                        )
                    )

            if not loop_forever:
                break


def main():
    parser = argparse.ArgumentParser(
        description="Replay exported latent automation through MIDI CC or save it as a MIDI file."
    )
    parser.add_argument(
        '-a', '--automation',
        help="Path to automation.json or data.json exported by generate_and_save_models_from_audio.py",
        required=False
    )
    parser.add_argument(
        '--list-ports',
        help="List available MIDI output ports and exit unless --port or --output-midi is also provided",
        action='store_true'
    )
    parser.add_argument(
        '-p', '--port',
        help="Name of the MIDI output port to stream to in real time",
        required=False
    )
    parser.add_argument(
        '-o', '--output-midi',
        help="Write the automation to a standard .mid file instead of or in addition to live playback",
        required=False
    )
    parser.add_argument(
        '-c', '--channel',
        help="MIDI channel number (1-16). The synth listens to CC messages on any channel by default.",
        type=int,
        default=1
    )
    parser.add_argument(
        '--slider-cc',
        help="Explicit CC numbers for the latent sliders. Must match the model's latent dimension exactly.",
        nargs='*',
        type=int,
        default=None
    )
    parser.add_argument(
        '--slider-cc-base',
        help="Starting CC number for the latent sliders. Defaults to the exported mapping or CC 1.",
        type=int,
        default=None
    )
    parser.add_argument(
        '--send-knobs',
        help="Also send the model's xMax and sClip defaults once at the start of playback",
        action='store_true'
    )
    parser.add_argument(
        '--force-all-cc',
        help="Send every slider CC on every frame instead of suppressing repeated values",
        action='store_true'
    )
    parser.add_argument(
        '--loop',
        help="Loop forever when using live playback through --port",
        action='store_true'
    )
    args = parser.parse_args()

    if args.channel < 1 or args.channel > 16:
        parser.error("--channel must be between 1 and 16")

    if args.list_ports:
        mido = _import_mido()
        output_names = mido.get_output_names()
        if not output_names:
            print("No MIDI output ports available.")
        else:
            for output_name in output_names:
                print(output_name)

        if not args.port and not args.output_midi:
            return

    if not args.automation:
        parser.error("--automation is required unless you only use --list-ports")

    if not args.port and not args.output_midi:
        parser.error("provide at least one destination: --port and/or --output-midi")

    payload = _load_payload(args.automation)
    ztrack, z_ranges, latent_dim = _validate_payload(payload)
    frame_period_seconds = resolve_frame_period_seconds(payload)
    slider_cc_numbers = resolve_slider_cc_numbers(payload, latent_dim, args.slider_cc, args.slider_cc_base)
    knob_messages = _build_knob_messages(payload, args.send_knobs)
    frame_messages = _build_slider_frame_messages(
        ztrack,
        z_ranges,
        slider_cc_numbers,
        force_all_cc=args.force_all_cc
    )
    zero_based_channel = args.channel - 1

    if args.output_midi:
        output_path = Path(args.output_midi).expanduser().resolve()
        _write_midi_file(output_path, zero_based_channel, knob_messages, frame_messages, frame_period_seconds)
        print(f"Wrote MIDI automation to {output_path}")

    if args.port:
        _send_live(
            args.port,
            zero_based_channel,
            knob_messages,
            frame_messages,
            frame_period_seconds,
            loop_forever=args.loop
        )


if __name__ == "__main__":
    main()
