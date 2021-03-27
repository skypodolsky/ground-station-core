#pragma once

#include "sat.h"

typedef enum rot_type_t {
	ROT_TYPE_NONE,
	ROT_TYPE_AZ,
	ROT_TYPE_EL
} rot_type_t;

int rotctl_open(observation_t *obs, rot_type_t type);
int rotctl_close(observation_t *obs, rot_type_t type);
int rotctl_send_az(observation_t *obs, double az);
int rotctl_send_el(observation_t *obs, double el);
int rotctl_send_and_wait(observation_t *obs, double az, double el);
int rotctl_calibrate(observation_t *obs, rot_type_t type);
int rotctl_stop(observation_t *obs);
