/// @file SweepScheduler.h
/// @brief Asks for slider sweeps once the sliders settle

#pragma once

#include <JobScheduler.h>
#include <SweepJob.h>

using SweepScheduler = JobScheduler<SweepJob>;
