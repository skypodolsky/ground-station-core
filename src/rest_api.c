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

#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

#include "sat.h"
#include "log.h"
#include "json.h"
#include "stats.h"
#include "rotctl.h"
#include "helpers.h"
#include "rest_api.h"

static int rest_api_get_status(char *payload, char **reply_buf, const char **error)
{
	int ret = 0;
	struct tm tm_aos;

	json_object *replyObj = json_object_new_object();
	if (!replyObj) {
		*error = "Out of memory";
		return -1;
	}

	json_object *statusObj = json_object_new_object();
	if (!statusObj) {
		*error = "Out of memory";
		ret = -1;
		goto out;
	}

	json_object_object_add(replyObj, "status", statusObj);

	observation_t *obs = sat_get_observation();
	if (obs) {
		global_stats_t *stats = stats_get_instance();

		json_object *uptimeObj = json_object_new_int(stats->observation_uptime);
		if (!uptimeObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "uptime", uptimeObj);

		json_object *satScheduledObj = json_object_new_int(stats->satellites_scheduled);
		if (!satScheduledObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "satellites_scheduled", satScheduledObj);

		json_object *satTrackedObj = json_object_new_int(stats->satellites_tracked);
		if (!satTrackedObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "satellites_tracked", satTrackedObj);

		json_object *satPreemptedObj = json_object_new_int(stats->satellites_preempted);
		if (!satPreemptedObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "satellites_preempted", satPreemptedObj);

		json_object *satVHFObj = json_object_new_int(stats->satellites_vhf);
		if (!satVHFObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "satellites_vhf", satVHFObj);

		json_object *satUHFObj = json_object_new_int(stats->satellites_uhf);
		if (!satVHFObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "satellites_uhf", satUHFObj);

		satellite_t *sat = sat_find_next();
		if (sat) {
			char buf[128];
			tm_aos = *localtime(&sat->next_aos);
			strftime(buf, sizeof(buf), "%d %B %Y - %I:%M%p %Z", &tm_aos);

			json_object *satAOSObj = json_object_new_string(buf);
			if (!satAOSObj) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			json_object_object_add(statusObj, "satellite_next_aos", satAOSObj);

			json_object *satNameObj = json_object_new_string(sat->name);
			if (!satNameObj) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			json_object_object_add(statusObj, "satellite_next_name", satNameObj);

		}

		if (obs->active) {
			json_object *stateObj = json_object_new_string("tracking in progress");
			if (!stateObj) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			json_object_object_add(statusObj, "state", stateObj);

			satellite_t *active = obs->active;
			time_t current_time = time(NULL) + obs->sim_time;

			json_object *progressObj = json_object_new_int((current_time - active->next_aos) * 100 / (active->next_los - active->next_aos));
			if (!progressObj) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			json_object_object_add(statusObj, "progress", progressObj);

		} else {
			json_object *stateObj = json_object_new_string("waiting");
			if (!stateObj) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			json_object_object_add(statusObj, "state", stateObj);
		}
	} else {
		json_object *stateObj = json_object_new_string("not set up");
		if (!stateObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "state", stateObj);
	}


	*reply_buf = strdup(json_object_to_json_string(replyObj));
	if (!reply_buf) {
		json_object_put(replyObj);
		return -1;
	}

out:
	json_object_put(replyObj);
	return ret;
}

static int rest_api_get_observation(char *payload, char **reply_buf, const char **error)
{
	int ret = 0;
	satellite_t *sat;
	observation_t *obs = sat_get_observation();

	if (!obs) {
		*error = "No observation is set up";
		return -1;
	}

	json_object *replyObj = json_object_new_object();
	if (!replyObj) {
		*error = "Out of memory";
		return -1;
	}

	json_object *satArrObj = json_object_new_array();
	if (!satArrObj) {
		*error = "Out of memory";
		return -1;
	}

	json_object_object_add(replyObj, "satellite", satArrObj);

	LIST_FOREACH(sat, &obs->satellites_list, entries) {
		struct tm tm_aos;
		struct tm tm_los;
		char buf[64];

		json_object *satObj = json_object_new_object();
		if (!satObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_array_add(satArrObj, satObj);

		/** name */
		json_object *satNameObj = json_object_new_string(sat->name);
		if (!satNameObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(satObj, "name", satNameObj);

		/** min elevation */
		json_object *satElevObj = json_object_new_double(sat->max_elevation);
		if (!satElevObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(satObj, "max elevation", satElevObj);

		/** frequency */
		json_object *satFreqObj = json_object_new_double(sat->frequency);
		if (!satFreqObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(satObj, "frequency", satFreqObj);

		tm_aos = *localtime(&sat->next_aos);
		tm_los = *localtime(&sat->next_los);
		
		/** AOS */
		strftime(buf, sizeof(buf), "%d %B %Y - %I:%M%p %Z", &tm_aos);

		json_object *satAOSObj = json_object_new_string(buf);
		if (!satAOSObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(satObj, "AOS", satAOSObj);

		/** LOS */
		strftime(buf, sizeof(buf), "%d %B %Y - %I:%M%p %Z", &tm_los);

		json_object *satLOSObj = json_object_new_string(buf);
		if (!satLOSObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(satObj, "LOS", satLOSObj);
	}

	*reply_buf = strdup(json_object_to_json_string(replyObj));
	if (!reply_buf) {
		json_object_put(replyObj);
		return -1;
	}

out:
	json_object_put(replyObj);
	return ret;
}

static int rest_api_set_observation(char *payload, char **reply_buf, const char **error)
{
	int ret;
	struct json_object *jObj;
	struct json_object *observationObj;
	observation_t *observation;

	ret = 0;

	observation = sat_get_observation();
	if (observation)
		if (observation->active) {
			*error = "Tracking in progress";
			return -1;
		}

	LOG_V("parsing JSON request...");
	jObj = json_tokener_parse(payload);
	if (!jObj) {
		*error = "Couldn't parse JSON request";
		return -1;
	}

	if (!json_object_object_get_ex(jObj, "observation", &observationObj)) {
		*error = "'/observation' object is missing";
		ret = -1;
		goto out;
	}

	if ((observation = sat_setup_observation()) == NULL) {
		ret = -1;
		goto out;
	}

	json_object *satellite;
	struct json_object *valObj;
	json_object_object_get_ex(observationObj, "satellite", &satellite);

	if (satellite) {
		int i;

		for (i = 0; i < json_object_array_length(satellite); i++) {
			json_object *jsatellitePart;

			satellite_t *sat = calloc(1, sizeof(satellite_t));
			if (!satellite) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			sat->obs = observation;
			LIST_INSERT_HEAD(&observation->satellites_list, sat, entries);

			jsatellitePart = json_object_array_get_idx(satellite, i);
			if (!jsatellitePart) {
				*error = "'/observation/satellite': couldn't get CG array index";
				ret = -1;
				goto out;
			}

			const char *satName = json_get_string_by_key(jsatellitePart, "name");
			if (!satName) {
				*error = "'/observation/satellite/name' string is not specified!";
				ret = -1;
				goto out;
			}

			LOG_V("satellite name: [ %s ]", satName);

			const char *satModulation = json_get_string_by_key(jsatellitePart, "modulation");
			if (!satModulation) {
				*error = "'/observation/satellite/modulation' string is not specified!";
				ret = -1;
				goto out;
			}

			if (!json_get_int_by_key(jsatellitePart, "baud_rate", &sat->baudRate)) {
				*error = "'/observation/satellite/baud_rate' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("Baud rate: [ %d ]", sat->baudRate);

			if (!json_get_int_by_key(jsatellitePart, "bandwidth", &sat->bandwidth)) {
				*error = "'/observation/satellite/bandwidth' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("Bandwidth: [ %d ]", sat->bandwidth);

			if (streq(satModulation, "bpsk")) {
				sat->modulation = MODULATION_BPSK;
				if (!json_object_object_get_ex(jsatellitePart, "bpsk_manchester", &valObj)) {
					*error = "'/observation/satellite/bpsk_manchester' object is missing";
					ret = -1;
					goto out;
				}

				if (json_object_get_type(valObj) == json_type_boolean) {
					sat->bpskManchester = json_object_get_boolean(valObj);
				} else {
					*error = "'/observation/satellite/bpsk_manchester' isn't boolean";
					ret = -1;
					goto out;
				}

				LOG_V("BPSK Manchester: [ %d ]", !!sat->bpskManchester);

				if (!json_object_object_get_ex(jsatellitePart, "bpsk_differential", &valObj)) {
					*error = "'/observation/satellite/bpsk_differential' object is missing";
					ret = -1;
					goto out;
				}

				if (json_object_get_type(valObj) == json_type_boolean) {
					sat->bpskDifferential = json_object_get_boolean(valObj);
				} else {
					*error = "'/observation/satellite/bpsk_differential' isn't boolean";
					ret = -1;
					goto out;
				}

				LOG_V("BPSK Differential: [ %d ]", !!sat->bpskDifferential);

				if (!json_object_object_get_ex(jsatellitePart, "bpsk_short_preamble", &valObj)) {
					*error = "'/observation/satellite/bpsk_short_preamble' object is missing";
					ret = -1;
					goto out;
				}

				if (json_object_get_type(valObj) == json_type_boolean) {
					sat->bpskShort = json_object_get_boolean(valObj);
				} else {
					*error = "'/observation/satellite/bpsk_short_preamble' isn't boolean";
					ret = -1;
					goto out;
				}

				LOG_V("BPSK Short Preamble: [ %d ]", !!sat->bpskShort);

				if (!json_object_object_get_ex(jsatellitePart, "bpsk_crc16", &valObj)) {
					*error = "'/observation/satellite/bpsk_crc16' object is missing";
					ret = -1;
					goto out;
				}

				if (json_object_get_type(valObj) == json_type_boolean) {
					sat->bpskCRC16 = json_object_get_boolean(valObj);
				} else {
					*error = "'/observation/satellite/bpsk_crc16' isn't boolean";
					ret = -1;
					goto out;
				}

				LOG_V("BPSK CRC16: [ %d ]", !!sat->bpskCRC16);

			} else if (streq(satModulation, "afsk")) {
				sat->modulation = MODULATION_AFSK;
				if (!json_get_int_by_key(jsatellitePart, "afsk_audio_freq_carrier", &sat->afskAFC)) {
					*error = "'/observation/satellite/afsk_audio_freq_carrier' not specified";
					ret = -1;
					goto out;
				}

				LOG_V("AFSK AFC: [ %d ]", sat->afskAFC);

				if (!json_get_int_by_key(jsatellitePart, "afsk_deviation", &sat->afskDeviation)) {
					*error = "'/observation/satellite/afsk_deviation' not specified";
					ret = -1;
					goto out;
				}

				LOG_V("AFSK Deviation: [ %d ]", sat->afskDeviation);

				if (!json_object_object_get_ex(jsatellitePart, "afsk_g3ruh", &valObj)) {
					*error = "'/observation/satellite/afsk_g3ruh' object is missing";
					ret = -1;
					goto out;
				}

				if (json_object_get_type(valObj) == json_type_boolean) {
					sat->afskG3RUH = json_object_get_boolean(valObj);
				} else {
					*error = "'/observation/satellite/afsk_g3ruh' isn't boolean";
					ret = -1;
					goto out;
				}

				LOG_V("FSK G3RUH: [ %d ]", !!sat->afskG3RUH);

			} else if (streq(satModulation, "fsk")) {
				sat->modulation = MODULATION_FSK;

				if (!json_object_object_get_ex(jsatellitePart, "fsk_sub_audio", &valObj)) {
					*error = "'/observation/satellite/fsk_sub_audio' object is missing";
					ret = -1;
					goto out;
				}

				if (json_object_get_type(valObj) == json_type_boolean) {
					sat->fskSubAudio = json_object_get_boolean(valObj);
				} else {
					*error = "'/observation/satellite/fsk_sub_audio' isn't boolean";
					ret = -1;
					goto out;
				}

				LOG_V("FSK subaudio: [ %d ]", !!sat->fskSubAudio);

				if (!json_object_object_get_ex(jsatellitePart, "fsk_g3ruh", &valObj)) {
					*error = "'/observation/satellite/fsk_g3ruh' object is missing";
					ret = -1;
					goto out;
				}

				if (json_object_get_type(valObj) == json_type_boolean) {
					sat->fskG3RUH = json_object_get_boolean(valObj);
				} else {
					*error = "'/observation/satellite/fsk_g3ruh' isn't boolean";
					ret = -1;
					goto out;
				}

				LOG_V("FSK G3RUH: [ %d ]", !!sat->fskG3RUH);
			}
			else {
				*error = "'/observation/satellite/modulation' only 'bpsk', 'fsk' and 'afsk' are supported";
				ret = -1;
				goto out;
			}

			LOG_V("satellite modulation: [ %s ]", satModulation);

			strncpy(sat->name, satName, sizeof(sat->name) - 1);

			if (!json_get_int_by_key(jsatellitePart, "frequency", &sat->frequency)) {
				*error = "'/observation/satellite/frequency' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("satellite frequency: [ %d ]", sat->frequency);

			if (!json_get_double_by_key(jsatellitePart, "min_elevation", &sat->min_elevation)) {
				*error = "'/observation/satellite/min_elevation' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("satellite min. elevation: [ %f ]", sat->min_elevation);

			if (!json_get_int_by_key(jsatellitePart, "priority", &sat->priority)) {
				*error = "'/observation/satellite/priority' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("satellite priority: [ %d ]", sat->priority);

			if (sat_setup(sat) == -1) {
				*error = "Satellite not found";
				ret = -1;
				goto out;
			}

		}
	} else {
		*error = "'/observation/satellite' object is missing";
		ret = -1;
		goto out;
	}

out:
	json_object_put(jObj);
	return ret;
}

static int rest_api_get_calibration(char *payload, char **reply_buf, const char **error)
{
	return rest_api_get_status(payload, reply_buf, error);
}

static int rest_api_set_calibration(char *payload, char **reply_buf, const char **error)
{
	int ret;
	bool azimuth = false;
	bool elevation = false;
	struct json_object *jObj;
	struct json_object *calibrationObj;
	struct json_object *valObj;
	observation_t *observation;

	ret = 0;

	LOG_I("Calibration REST API started");
	observation = sat_get_observation();
	if (!observation) {
		*error = "No observation is set up";
		return -1;
	}

	LOG_V("parsing JSON request...");
	jObj = json_tokener_parse(payload);
	if (!jObj) {
		*error = "Couldn't parse JSON request";
		return -1;
	}

	if (!json_object_object_get_ex(jObj, "calibration", &calibrationObj)) {
		*error = "'/calibration' object is missing";
		ret = -1;
		goto out;
	}

	if (!json_object_object_get_ex(calibrationObj, "azimuth", &valObj)) {
		*error = "'/azimuth' object is missing";
		ret = -1;
		goto out;
	}

	if (json_object_get_type(valObj) == json_type_boolean) {
		azimuth = json_object_get_boolean(valObj);
	}

	if (!json_object_object_get_ex(calibrationObj, "elevation", &valObj)) {
		*error = "'/elevation' object is missing";
		ret = -1;
		goto out;
	}

	if (json_object_get_type(valObj) == json_type_boolean) {
		elevation = json_object_get_boolean(valObj);
	}

	LOG_V("Calibration for: az=%d, el=%d", !!azimuth, !!elevation);
	rotctl_calibrate(observation, azimuth, elevation);
	LOG_I("Calibration done");

out:
	json_object_put(jObj);
	return ret;
}

static rest_api_t rest_api[] = {
	{ "/status", REST_API_TYPE_GET, rest_api_get_status },
	{ "/observation", REST_API_TYPE_GET, rest_api_get_observation },
	{ "/observation", REST_API_TYPE_POST, rest_api_set_observation },
	{ "/calibration", REST_API_TYPE_GET, rest_api_get_calibration},
	{ "/calibration", REST_API_TYPE_POST, rest_api_set_calibration},
};

rest_api_type_t rest_api_get_type(const char *type_str)
{
	if (!strncmp(type_str, "POST", 4))
		return REST_API_TYPE_POST;
	else if (!strncmp(type_str, "GET", 3))
		return REST_API_TYPE_GET;
	else
		return REST_API_TYPE_NOT_SUPP;
}

rest_api_action_t rest_api_find_action(const char *api, rest_api_type_t type)
{
	int i;

	if (!api)
		return NULL;

	for (i = 0; i < sizeof(rest_api) / sizeof(rest_api_t); i++) {
		if (!strncmp(api, rest_api[i].name, strlen(rest_api[i].name))) {
			if (type == rest_api[i].type) {
				return rest_api[i].action;
			}
		}
	}

	return NULL;
}

int rest_api_prepare_error(const char *error, char **reply_buf)
{
	json_object *errorObj;
	json_object *replyObj = json_object_new_object();

	if (!replyObj)
		return -1;

	if (error) {
		errorObj = json_object_new_string(error);
	} else {
		errorObj = json_object_new_string("Unknown error");
	}
	if (!errorObj)
		return -1;

	json_object_object_add(replyObj, "Error", errorObj);

	*reply_buf = strdup(json_object_to_json_string(replyObj));
	if (!reply_buf)
		return -1;

	return 0;
}
