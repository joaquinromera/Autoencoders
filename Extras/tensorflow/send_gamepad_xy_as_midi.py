# -*- coding: utf-8 -*-

"""Read a gamepad's two sticks and emit MIDI CC for the synth's two XY controllers."""

import argparse
import json
import os
import sys
import time
from pathlib import Path

from latent_midi import coerce_cc_number


def _import_mido():
    try:
        import mido
    except ImportError as exc:
        raise RuntimeError(
            "This script requires the 'mido' package. Install Extras/tensorflow/requirements.txt first. "
            "Live MIDI output also needs 'python-rtmidi'."
        ) from exc

    return mido


def _import_pygame():
    try:
        os.environ.setdefault("SDL_JOYSTICK_HIDAPI", "1")
        os.environ.setdefault("SDL_JOYSTICK_HIDAPI_PS4", "1")
        os.environ.setdefault("SDL_JOYSTICK_HIDAPI_PS5", "1")
        import pygame
    except ImportError as exc:
        raise RuntimeError(
            "This script requires the 'pygame' package. Install Extras/tensorflow/requirements.txt first."
        ) from exc

    return pygame


def _apply_deadzone(value, deadzone):
    value = max(-1.0, min(1.0, float(value)))
    if abs(value) <= deadzone:
        return 0.0

    scaled = (abs(value) - deadzone) / (1.0 - deadzone)
    return scaled if value >= 0.0 else -scaled


def _apply_calibration(value, calibration_entry=None):
    raw_value = max(-1.0, min(1.0, float(value)))
    if not calibration_entry:
        return raw_value

    center_value = float(calibration_entry["center"])
    minimum_value = float(calibration_entry["min"])
    maximum_value = float(calibration_entry["max"])

    if raw_value >= center_value:
        span = max(1e-6, maximum_value - center_value)
        calibrated_value = (raw_value - center_value) / span
    else:
        span = max(1e-6, center_value - minimum_value)
        calibrated_value = (raw_value - center_value) / span

    return max(-1.0, min(1.0, calibrated_value))


def _axis_to_normalized(value, deadzone, invert=False, response_curve=1.0, axis_scale=1.0, calibration_entry=None):
    calibrated_value = _apply_calibration(value, calibration_entry)
    adjusted = _apply_deadzone(-calibrated_value if invert else calibrated_value, deadzone)
    magnitude = abs(adjusted) ** response_curve
    shaped = magnitude if adjusted >= 0.0 else -magnitude
    scaled_shaped = max(-1.0, min(1.0, shaped * axis_scale))
    return (scaled_shaped + 1.0) * 0.5


def _normalized_to_cc_value(normalized):
    normalized = max(0.0, min(1.0, float(normalized)))
    return int(round(normalized * 127.0))


def _list_midi_ports():
    mido = _import_mido()
    output_names = mido.get_output_names()
    if not output_names:
        print("No MIDI output ports available.")
        return

    for index, output_name in enumerate(output_names):
        print(f"{index}: {output_name}")


def _list_gamepads():
    pygame = _import_pygame()
    pygame.init()
    pygame.joystick.init()

    try:
        joystick_count = pygame.joystick.get_count()
        if joystick_count == 0:
            print("No gamepads detected.")
            return

        for index in range(joystick_count):
            joystick = pygame.joystick.Joystick(index)
            joystick.init()
            print(
                f"{index}: {joystick.get_name()} "
                f"(axes={joystick.get_numaxes()}, buttons={joystick.get_numbuttons()}, hats={joystick.get_numhats()})"
            )
            joystick.quit()
    finally:
        pygame.joystick.quit()
        pygame.quit()


def _open_joystick(gamepad_index):
    pygame = _import_pygame()
    pygame.init()
    pygame.joystick.init()

    joystick_count = pygame.joystick.get_count()
    if joystick_count == 0:
        raise RuntimeError("No gamepads detected.")

    if gamepad_index < 0 or gamepad_index >= joystick_count:
        raise RuntimeError(
            f"Gamepad index {gamepad_index} is out of range; detected {joystick_count} gamepad(s)."
        )

    joystick = pygame.joystick.Joystick(gamepad_index)
    joystick.init()
    return pygame, joystick


def _validate_axis_indices(joystick, axis_indices):
    axis_count = joystick.get_numaxes()
    for axis_name, axis_index in axis_indices.items():
        if axis_index < 0 or axis_index >= axis_count:
            raise RuntimeError(
                f"{axis_name} axis index {axis_index} is out of range for {joystick.get_name()} "
                f"(available axes: 0..{axis_count - 1})"
            )


def _build_axis_config(args):
    return [
        {
            "label": "left_x",
            "axis_index": args.left_x_axis,
            "controller_number": coerce_cc_number(args.left_x_cc),
            "invert": False,
        },
        {
            "label": "left_y",
            "axis_index": args.left_y_axis,
            "controller_number": coerce_cc_number(args.left_y_cc),
            "invert": not args.no_invert_y,
        },
        {
            "label": "right_x",
            "axis_index": args.right_x_axis,
            "controller_number": coerce_cc_number(args.right_x_cc),
            "invert": False,
        },
        {
            "label": "right_y",
            "axis_index": args.right_y_axis,
            "controller_number": coerce_cc_number(args.right_y_cc),
            "invert": not args.no_invert_y,
        },
    ]


def _load_calibration(calibration_path):
    if calibration_path is None:
        return {}

    with open(calibration_path, "r", encoding="utf-8") as handle:
        payload = json.load(handle)

    return payload.get("axes", {})


def _sample_axis_centers(pygame, joystick, axis_config, duration_seconds=0.75, sample_hz=120.0):
    sample_count = max(1, int(duration_seconds * sample_hz))
    accumulators = {item["label"]: 0.0 for item in axis_config}

    for _ in range(sample_count):
        pygame.event.pump()
        for item in axis_config:
            accumulators[item["label"]] += joystick.get_axis(item["axis_index"])
        time.sleep(1.0 / sample_hz)

    return {label: total / sample_count for label, total in accumulators.items()}


def _run_calibration(args):
    calibration_path = Path(args.calibrate).expanduser().resolve()
    pygame, joystick = _open_joystick(args.gamepad_index)
    axis_config = _build_axis_config(args)
    _validate_axis_indices(
        joystick,
        {item["label"]: item["axis_index"] for item in axis_config}
    )

    try:
        print(f"Calibrating gamepad {args.gamepad_index}: {joystick.get_name()}")
        print("Leave both sticks centered, then press Enter.")
        input()

        center_values = _sample_axis_centers(pygame, joystick, axis_config)

        extrema = {
            item["label"]: {
                "axis_index": item["axis_index"],
                "center": center_values[item["label"]],
                "min": center_values[item["label"]],
                "max": center_values[item["label"]],
            }
            for item in axis_config
        }

        print(
            f"Move both sticks slowly through their full travel in every direction for "
            f"{args.calibrate_duration:.1f} seconds, then release them."
        )
        deadline = time.monotonic() + args.calibrate_duration
        next_progress_second = int(args.calibrate_duration)
        while time.monotonic() < deadline:
            pygame.event.pump()
            for item in axis_config:
                axis_label = item["label"]
                raw_value = joystick.get_axis(item["axis_index"])
                extrema[axis_label]["min"] = min(extrema[axis_label]["min"], raw_value)
                extrema[axis_label]["max"] = max(extrema[axis_label]["max"], raw_value)

            remaining_seconds = max(0.0, deadline - time.monotonic())
            rounded_remaining_seconds = int(remaining_seconds)
            if rounded_remaining_seconds < next_progress_second:
                print(f"Calibration capture: {remaining_seconds:.1f}s remaining")
                next_progress_second = rounded_remaining_seconds

            time.sleep(1.0 / args.poll_hz)

        calibration_payload = {
            "gamepad_name": joystick.get_name(),
            "axes": extrema,
        }
        calibration_path.parent.mkdir(parents=True, exist_ok=True)
        with open(calibration_path, "w", encoding="utf-8") as handle:
            json.dump(calibration_payload, handle, indent=2, sort_keys=True)

        print(f"Saved calibration to {calibration_path}")
        return extrema
    finally:
        joystick.quit()
        pygame.joystick.quit()
        pygame.quit()


def _monitor_axes(args):
    pygame, joystick = _open_joystick(args.gamepad_index)

    try:
        print(f"Monitoring gamepad {args.gamepad_index}: {joystick.get_name()}")
        print("Press Ctrl+C to stop.")

        while True:
            pygame.event.pump()
            axis_values = [
                f"{axis_index}:{joystick.get_axis(axis_index):+0.3f}"
                for axis_index in range(joystick.get_numaxes())
            ]
            line = "  ".join(axis_values)
            sys.stdout.write(f"\r{line}   ")
            sys.stdout.flush()
            time.sleep(1.0 / args.poll_hz)
    except KeyboardInterrupt:
        sys.stdout.write("\n")
        sys.stdout.flush()
    finally:
        joystick.quit()
        pygame.joystick.quit()
        pygame.quit()


def _send_live(args):
    mido = _import_mido()
    pygame, joystick = _open_joystick(args.gamepad_index)

    axis_config = _build_axis_config(args)
    calibration_data = _load_calibration(args.calibration)
    _validate_axis_indices(
        joystick,
        {item["label"]: item["axis_index"] for item in axis_config}
    )

    zero_based_channel = args.channel - 1
    poll_interval = 1.0 / args.poll_hz
    previous_cc_values = {}
    smoothed_normalized_values = {}

    print(f"Using gamepad {args.gamepad_index}: {joystick.get_name()}")
    print(f"Sending MIDI CC to port: {args.port}")

    try:
        with mido.open_output(args.port) as port:
            while True:
                pygame.event.pump()

                for item in axis_config:
                    axis_value = joystick.get_axis(item["axis_index"])
                    target_normalized_value = _axis_to_normalized(
                        axis_value,
                        args.deadzone,
                        invert=item["invert"],
                        response_curve=args.response_curve,
                        axis_scale=args.axis_scale,
                        calibration_entry=calibration_data.get(item["label"])
                    )
                    controller_number = item["controller_number"]

                    previous_smoothed_value = smoothed_normalized_values.get(controller_number, 0.5)
                    smoothed_normalized_value = (
                        (previous_smoothed_value * args.smoothing) +
                        (target_normalized_value * (1.0 - args.smoothing))
                    )
                    cc_value = _normalized_to_cc_value(smoothed_normalized_value)

                    previous_cc_value = previous_cc_values.get(controller_number)
                    if previous_cc_value is not None and abs(previous_cc_value - cc_value) < args.min_cc_step:
                        smoothed_normalized_values[controller_number] = smoothed_normalized_value
                        continue

                    port.send(
                        mido.Message(
                            "control_change",
                            channel=zero_based_channel,
                            control=controller_number,
                            value=cc_value,
                        )
                    )
                    smoothed_normalized_values[controller_number] = smoothed_normalized_value
                    previous_cc_values[controller_number] = cc_value

                    if args.verbose:
                        print(
                            f"{item['label']}: axis={item['axis_index']} raw={axis_value:+.3f} "
                            f"cc={controller_number} value={cc_value}"
                        )

                time.sleep(poll_interval)
    except KeyboardInterrupt:
        pass
    finally:
        joystick.quit()
        pygame.joystick.quit()
        pygame.quit()


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Read the two analog sticks from a gamepad and send them as MIDI CC. "
            "Defaults match the synth's two XY controllers: left stick -> CC 1/2, right stick -> CC 3/4."
        )
    )
    parser.add_argument(
        "--list-ports",
        action="store_true",
        help="List available MIDI output ports and exit unless --port is also provided"
    )
    parser.add_argument(
        "--list-gamepads",
        action="store_true",
        help="List detected gamepads and exit unless --port is also provided"
    )
    parser.add_argument(
        "--monitor-axes",
        action="store_true",
        help="Print raw gamepad axis values continuously to help determine axis indexes and directions"
    )
    parser.add_argument(
        "--calibrate",
        default=None,
        help="Run an interactive axis calibration wizard and save the result to this JSON file"
    )
    parser.add_argument(
        "--calibration",
        default=None,
        help="Path to a previously saved calibration JSON file"
    )
    parser.add_argument(
        "--calibrate-duration",
        type=float,
        default=12.0,
        help="Duration in seconds for the calibration sweep capture"
    )
    parser.add_argument(
        "-p",
        "--port",
        default=None,
        help="Name of the MIDI output port to send CC messages to"
    )
    parser.add_argument(
        "--gamepad-index",
        type=int,
        default=0,
        help="Gamepad index to use when multiple controllers are connected"
    )
    parser.add_argument(
        "-c",
        "--channel",
        type=int,
        default=1,
        help="MIDI channel number (1-16)"
    )
    parser.add_argument(
        "--left-x-cc",
        type=int,
        default=1,
        help="CC number for the left stick X axis"
    )
    parser.add_argument(
        "--left-y-cc",
        type=int,
        default=2,
        help="CC number for the left stick Y axis"
    )
    parser.add_argument(
        "--right-x-cc",
        type=int,
        default=3,
        help="CC number for the right stick X axis"
    )
    parser.add_argument(
        "--right-y-cc",
        type=int,
        default=4,
        help="CC number for the right stick Y axis"
    )
    parser.add_argument(
        "--left-x-axis",
        type=int,
        default=0,
        help="Gamepad axis index for the left stick X axis"
    )
    parser.add_argument(
        "--left-y-axis",
        type=int,
        default=1,
        help="Gamepad axis index for the left stick Y axis"
    )
    parser.add_argument(
        "--right-x-axis",
        type=int,
        default=2,
        help="Gamepad axis index for the right stick X axis"
    )
    parser.add_argument(
        "--right-y-axis",
        type=int,
        default=3,
        help="Gamepad axis index for the right stick Y axis"
    )
    parser.add_argument(
        "--deadzone",
        type=float,
        default=0.02,
        help="Stick deadzone in normalized axis units (0.0-0.95)"
    )
    parser.add_argument(
        "--response-curve",
        type=float,
        default=1.0,
        help="Axis response curve. Use 1.0 for linear response; values above 1.0 make the center less sensitive."
    )
    parser.add_argument(
        "--axis-scale",
        type=float,
        default=1.0,
        help="Scale shaped axis range. Values below 1.0 reduce sensitivity but also reduce the maximum reachable CC range."
    )
    parser.add_argument(
        "--smoothing",
        type=float,
        default=0.0,
        help="Temporal smoothing amount from 0.0 to <1.0. Higher values feel softer but lag more."
    )
    parser.add_argument(
        "--min-cc-step",
        type=int,
        default=1,
        help="Minimum MIDI CC delta before sending an update"
    )
    parser.add_argument(
        "--poll-hz",
        type=float,
        default=120.0,
        help="Polling rate for reading the gamepad"
    )
    parser.add_argument(
        "--no-invert-y",
        action="store_true",
        help="Do not invert the Y axes. By default stick-up increases the MIDI value."
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print every CC update as it is sent"
    )
    args = parser.parse_args()

    if args.channel < 1 or args.channel > 16:
        parser.error("--channel must be between 1 and 16")

    if args.deadzone < 0.0 or args.deadzone >= 1.0:
        parser.error("--deadzone must be in the range [0.0, 1.0)")

    if args.response_curve < 1.0:
        parser.error("--response-curve must be greater than or equal to 1.0")

    if args.axis_scale <= 0.0:
        parser.error("--axis-scale must be greater than zero")

    if args.smoothing < 0.0 or args.smoothing >= 1.0:
        parser.error("--smoothing must be in the range [0.0, 1.0)")

    if args.min_cc_step < 1 or args.min_cc_step > 127:
        parser.error("--min-cc-step must be between 1 and 127")

    if args.poll_hz <= 0.0:
        parser.error("--poll-hz must be greater than zero")

    if args.calibrate_duration <= 0.0:
        parser.error("--calibrate-duration must be greater than zero")

    if args.list_ports:
        _list_midi_ports()
        if not args.port:
            return

    if args.list_gamepads:
        _list_gamepads()
        if not args.port:
            return

    if args.monitor_axes:
        _monitor_axes(args)
        if not args.port:
            return

    if args.calibrate:
        _run_calibration(args)
        if not args.port:
            return

    if not args.port:
        parser.error(
            "--port is required unless you only use --list-ports, --list-gamepads, --monitor-axes, and/or --calibrate"
        )

    _send_live(args)


if __name__ == "__main__":
    main()
