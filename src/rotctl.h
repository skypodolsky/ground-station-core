/*
 * Ground Station Core (GSC)
 * Copyright (C) 2021  International Space University
 * Contributors:
 *   Stanislav Barantsev

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
int rotctl_park_and_wait(observation_t *obs, double az, double el);
int rotctl_calibrate(observation_t *obs, bool azimuth, bool elevation);
float rotctl_get_azimuth(observation_t *obs);
float rotctl_get_elevation(observation_t *obs);
int rotctl_stop(observation_t *obs);
