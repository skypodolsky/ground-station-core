#pragma once

#include "sat.h"

typedef enum rot_type_t {
	ROT_TYPE_AZ,
	ROT_TYPE_EL
} rot_type_t;

int rotctl_open(observation_t *obs, rot_type_t type);
int rotctl_close(observation_t *obs, rot_type_t type);
int rotctl_send_and_wait(observation_t *obs, double az, double el);
