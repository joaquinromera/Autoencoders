# -*- coding: utf-8 -*-

"""Helpers for exporting and replaying latent-space automation as MIDI CC."""

from typing import Dict, List, Optional, Sequence, Tuple

DEFAULT_SLIDER_CC_BASE = 1
DEFAULT_XMAX_CC = 16
DEFAULT_SCLIP_CC = 17

DEFAULT_XMAX_RANGE = (0.0, 100.0)
DEFAULT_SCLIP_RANGE = (-100.0, 0.0)


def coerce_float(value) -> float:
    """Parse either a JSON number or a stringified number."""

    return float(value)


def clamp(value: float, minimum: float, maximum: float) -> float:
    """Clamp a scalar to the provided bounds."""

    return max(minimum, min(maximum, value))


def build_default_slider_ccs(latent_dim: int) -> List[int]:
    """Return the synth's default CC layout for the latent sliders."""

    return [coerce_cc_number(DEFAULT_SLIDER_CC_BASE + index) for index in range(latent_dim)]


def coerce_cc_number(value) -> int:
    """Validate a MIDI controller number."""

    cc_number = int(value)
    if cc_number < 0 or cc_number > 127:
        raise ValueError(f"MIDI CC numbers must be between 0 and 127, got {cc_number}")

    return cc_number


def parse_z_ranges(parameters: Dict) -> List[Tuple[float, float]]:
    """Extract latent slider ranges from an exported model payload."""

    return [
        (coerce_float(item["min"]), coerce_float(item["max"]))
        for item in parameters.get("zRange", [])
    ]


def resolve_frame_period_seconds(payload: Dict) -> float:
    """Resolve frame spacing from explicit automation metadata or model parameters."""

    automation = payload.get("automation", {})
    if isinstance(automation, dict) and "frame_period_seconds" in automation:
        frame_period_seconds = coerce_float(automation["frame_period_seconds"])
        if frame_period_seconds <= 0.0:
            raise ValueError("automation.frame_period_seconds must be greater than zero")
        return frame_period_seconds

    parameters = payload.get("parameters", {})
    if "hop_length" not in parameters or "sr" not in parameters:
        raise ValueError(
            "automation metadata is missing frame timing information; expected automation.frame_period_seconds "
            "or parameters.hop_length and parameters.sr"
        )

    sample_rate = coerce_float(parameters["sr"])
    hop_length = coerce_float(parameters["hop_length"])
    if sample_rate <= 0.0:
        raise ValueError("parameters.sr must be greater than zero")

    return hop_length / sample_rate


def resolve_slider_cc_numbers(
    payload: Dict,
    latent_dim: int,
    explicit_cc_numbers: Optional[Sequence[int]] = None,
    slider_cc_base: Optional[int] = None
) -> List[int]:
    """Resolve the CC numbers used for latent slider replay."""

    if explicit_cc_numbers:
        if len(explicit_cc_numbers) != latent_dim:
            raise ValueError(
                "the number of --slider-cc values must match the latent dimension "
                f"({latent_dim}, got {len(explicit_cc_numbers)})"
            )
        return [coerce_cc_number(value) for value in explicit_cc_numbers]

    if slider_cc_base is not None:
        return [coerce_cc_number(int(slider_cc_base) + index) for index in range(latent_dim)]

    automation = payload.get("automation", {})
    if isinstance(automation, dict):
        default_slider_ccs = automation.get("default_slider_cc")
        if default_slider_ccs is not None:
            if len(default_slider_ccs) != latent_dim:
                raise ValueError(
                    "automation.default_slider_cc does not match the latent dimension "
                    f"({latent_dim}, got {len(default_slider_ccs)})"
                )
            return [coerce_cc_number(value) for value in default_slider_ccs]

    return build_default_slider_ccs(latent_dim)


def resolve_knob_cc_numbers(payload: Dict) -> Tuple[int, int]:
    """Resolve the CC numbers used for the synth's xMax and sClip knobs."""

    automation = payload.get("automation", {})
    if isinstance(automation, dict):
        x_max_cc = coerce_cc_number(automation.get("default_xmax_cc", DEFAULT_XMAX_CC))
        s_clip_cc = coerce_cc_number(automation.get("default_sclip_cc", DEFAULT_SCLIP_CC))
        return x_max_cc, s_clip_cc

    return DEFAULT_XMAX_CC, DEFAULT_SCLIP_CC


def value_to_midi_cc(value: float, minimum: float, maximum: float) -> int:
    """Quantise a latent-space value into a 7-bit MIDI CC value."""

    if maximum <= minimum:
        return 0

    normalised = (coerce_float(value) - minimum) / (maximum - minimum)
    normalised = clamp(normalised, 0.0, 1.0)
    return int(round(normalised * 127.0))
