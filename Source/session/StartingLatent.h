/// @file StartingLatent.h
/// @brief Where a freshly loaded model starts

#pragma once

#include <ControlRanges.h>
#include <LatentTrack.h>

#include <vector>

/// @brief The latent vector a model starts at after loading
///
/// The track frame closest to the track's mean, so the first thing heard is a
/// sound from the training audio; the middle of each range when there is no
/// track (or it does not match the ranges).
std::vector<float> startingLatent(const ControlRanges &ranges, const LatentTrack *track);
