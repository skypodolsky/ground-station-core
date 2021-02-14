#pragma once

#include "sat.h"

int rotctl_open(observation_t *obs);
int rotctl_close(observation_t *obs);
int rotctl_send(observation_t *obs, double az, double el);
