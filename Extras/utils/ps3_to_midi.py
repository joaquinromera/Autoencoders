#!/usr/bin/env python3
import math
import time
import sys
import pygame
import rtmidi
from rtmidi.midiconstants import NOTE_ON, NOTE_OFF, CONTROL_CHANGE, PITCH_BEND
pygame.init()
pygame.joystick.init()

# ---------- Config ----------
MIDI_CHANNEL = 0  # 0..15 (0 = channel 1)
DEADZONE = 0.12   # ignore tiny stick noise
POLL_HZ = 250

# Axis -> CC mapping (PS3 typical: 0=LX,1=LY,2=RX,3=RY, 12/13 analog triggers vary by driver)
AXIS_TO_CC = {
    0: 1,   # Mod Wheel
    1: 74,  # Brightness/Filter
    2: 10,  # Pan
    3: 11,  # Expression
}
# Optional: map one axis to Pitch Bend instead of CC (14-bit center=8192)
PITCH_BEND_AXIS = None  # e.g. 2

# Buttons -> MIDI notes (cross/square/triangle/circle are common 0..3 but may vary)
BUTTON_TO_NOTE = {
    0: 60,  # Cross -> C4
    1: 62,  # Circle -> D4
    2: 64,  # Square -> E4
    3: 67,  # Triangle -> G4
    4: 69,  # L1
    5: 71,  # R1
}

# D-Pad (hats) ->
