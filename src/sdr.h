#pragma once

#include <signal.h>
#include <unistd.h>

#include "sat.h"

int sdr_set_freq(double freq);
int sdr_start(satellite_t *sat);
int sdr_stop(observation_t *obs);
