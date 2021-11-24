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

#include <math.h>
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

		json_object *lastAzObj = json_object_new_double(roundf(stats->last_azimuth * 100) / 100);
		if (!lastAzObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "azimuth", lastAzObj);


		json_object *lastElObj = json_object_new_double(roundf(stats->last_elevation * 100) / 100);
		if (!lastElObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "elevation", lastElObj);

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

		json_object *stateCodeObj = json_object_new_int(stats->state);
		if (!stateCodeObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "state_code", stateCodeObj);

		char *state = NULL;
		int progress = 0;

		switch (stats->state) {
			satellite_t *active;
			time_t current_time;

			case GSC_STATE_CALIBRATING:
				state = "calibrating";
				break;
			case GSC_STATE_PARKING:
				state = "parking";
				break;
			case GSC_STATE_TRACKING:
				active = obs->active;
				current_time = time(NULL) + obs->sim_time;
				progress = (current_time - active->next_aos) * 100 / (active->next_los - active->next_aos);
				state = "tracking";
				break;
			default:
				state = "waiting";
				break;
		}

		json_object *stateObj = json_object_new_string(state);
		if (!stateObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "state", stateObj);

		/** also add a progress */
		if (stats->state == GSC_STATE_TRACKING) {
			json_object *progressObj = json_object_new_int(progress);
			if (!progressObj) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			json_object_object_add(statusObj, "progress", progressObj);
		}
	} else {
		json_object *stateCodeObj = json_object_new_int(GSC_STATE_NOT_CONFIGURED);
		if (!stateCodeObj) {
			*error = "Out of memory";
			ret = -1;
			goto out;
		}

		json_object_object_add(statusObj, "state_code", stateCodeObj);

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

static int rest_api_parse_modulation(satellite_t *sat, json_object *modulationObj, const char **error)
{
	int ret = 0;
	json_object *valObj;

	if (!json_get_int_by_key(modulationObj, "bandwidth", &sat->bandwidth)) {
		*error = "'/observation/satellite/modulation/bandwidth' not specified";
		ret = -1;
		goto out;
	}

	LOG_V("Bandwidth: [ %d ]", sat->bandwidth);

	const char *satModulation = json_get_string_by_key(modulationObj, "type");
	if (!satModulation) {
		*error = "'/observation/satellite/modulation/type' string is not specified!";
		ret = -1;
		goto out;
	}

	LOG_V("Modulation [ %s ]", satModulation);

	if (streq(satModulation, "bpsk")) {
		sat->modulation = MODULATION_BPSK;
		if (!json_object_object_get_ex(modulationObj, "bpsk_manchester", &valObj)) {
			*error = "'/observation/satellite/modulation/bpsk_manchester' object is missing";
			ret = -1;
			goto out;
		}

		if (json_object_get_type(valObj) == json_type_boolean) {
			sat->bpskManchester = json_object_get_boolean(valObj);
		} else {
			*error = "'/observation/satellite/modulation/bpsk_manchester' isn't boolean or doesn't exist";
			ret = -1;
			goto out;
		}

		LOG_V("BPSK Manchester: [ %d ]", !!sat->bpskManchester);

		if (!json_object_object_get_ex(modulationObj, "bpsk_differential", &valObj)) {
			*error = "'/observation/satellite/modulation/bpsk_differential' object is missing";
			ret = -1;
			goto out;
		}

		if (json_object_get_type(valObj) == json_type_boolean) {
			sat->bpskDifferential = json_object_get_boolean(valObj);
		} else {
			*error = "'/observation/satellite/modulation/bpsk_differential' isn't boolean or doesn't exist";
			ret = -1;
			goto out;
		}

		LOG_V("BPSK Differential: [ %d ]", !!sat->bpskDifferential);

	} else if (streq(satModulation, "afsk")) {
		sat->modulation = MODULATION_AFSK;
		if (!json_get_int_by_key(modulationObj, "afsk_audio_freq_carrier", &sat->afskAFC)) {
			*error = "'/observation/satellite/modulation/afsk_audio_freq_carrier' not specified";
			ret = -1;
			goto out;
		}

		LOG_V("AFSK AFC: [ %d ]", sat->afskAFC);

		if (!json_get_int_by_key(modulationObj, "afsk_deviation", &sat->afskDeviation)) {
			*error = "'/observation/satellite/modulation/afsk_deviation' not specified";
			ret = -1;
			goto out;
		}

		LOG_V("AFSK Deviation: [ %d ]", sat->afskDeviation);

	} else if (streq(satModulation, "fsk")) {
		sat->modulation = MODULATION_FSK;

		if (!json_object_object_get_ex(modulationObj, "fsk_sub_audio", &valObj)) {
			*error = "'/observation/satellite/modulation/fsk_sub_audio' object is missing";
			ret = -1;
			goto out;
		}

		if (json_object_get_type(valObj) == json_type_boolean) {
			sat->fskSubAudio = json_object_get_boolean(valObj);
		} else {
			*error = "'/observation/satellite/modulation/fsk_sub_audio' isn't boolean or doesn't exist";
			ret = -1;
			goto out;
		}

		LOG_V("FSK subaudio: [ %d ]", !!sat->fskSubAudio);
	} else if (streq(satModulation, "fm")) {
		sat->modulation = MODULATION_FM;
		/** do nothing here */
	}
	else {
		*error = "'/observation/satellite/modulation/type' only 'bpsk', 'fsk', 'afsk' and 'fm' are supported";
		ret = -1;
		goto out;
	}

	if (sat->modulation != MODULATION_FM) {
		if (!json_get_int_by_key(modulationObj, "baud_rate", &sat->baudRate)) {
			*error = "'/observation/satellite/modulation/baud_rate' not specified";
			ret = -1;
			goto out;
		}
	}

	LOG_V("Baud rate: [ %d ]", sat->baudRate);

out:
	return ret;
}

static int rest_api_parse_deframer(satellite_t *sat, json_object *deframerObj, const char **error)
{
	int ret = 0;
	json_object *valObj;

	const char *deframerType = json_get_string_by_key(deframerObj, "type");
	if (!deframerType) {
		*error = "'/observation/satellite/deframer/type' string is not specified!";
		ret = -1;
		goto out;
	}

	LOG_V("Deframer type: [ %s ]", deframerType);

	if (streq(deframerType, "ao-40")) {
		sat->deframer = DEFRAMER_AO_40;

		json_object_object_get_ex(deframerObj, "short_frames", &valObj);

		if (json_object_get_type(valObj) == json_type_boolean) {
			sat->shortFrames = json_object_get_boolean(valObj);
		} else {
			*error = "'/observation/satellite/deframer/short_frames' isn't boolean or doesn't exist";
			ret = -1;
			goto out;
		}

		LOG_V("Short Frames: [ %d ]", !!sat->shortFrames);

		json_object_object_get_ex(deframerObj, "crc16", &valObj);

		if (json_object_get_type(valObj) == json_type_boolean) {
			sat->crc16 = json_object_get_boolean(valObj);
		} else {
			*error = "'/observation/satellite/deframer/short_frames' isn't boolean or doesn't exist";
			ret = -1;
			goto out;
		}

		LOG_V("CRC16: [ %d ]", !!sat->crc16);
	} else if (streq(deframerType, "gomspace-u482c")) {
		sat->deframer = DEFRAMER_GOMSPACE_U482C;
	} else if (streq(deframerType, "gomspace-ax100-rs")) {
		sat->deframer = DEFRAMER_GOMSPACE_AX100_RS;
	} else if (streq(deframerType, "gomspace-ax100-asm-golay")) {
		sat->deframer = DEFRAMER_GOMSPACE_AX100_ASM_GOLAY;
	} else if (streq(deframerType, "ax-25")) {
		sat->deframer = DEFRAMER_AX25;

		json_object_object_get_ex(deframerObj, "g3ruh", &valObj);

		if (json_object_get_type(valObj) == json_type_boolean) {
			sat->g3ruh = json_object_get_boolean(valObj);
		} else {
			*error = "'/observation/satellite/deframer/g3ruh' isn't boolean or doesn't exist";
			ret = -1;
			goto out;
		}

		LOG_V("G3RUH: [ %d ]", !!sat->g3ruh);
	} else if (streq(deframerType, "fossasat")) {
		sat->deframer = DEFRAMER_FOSSASAT;
	} else if (streq(deframerType, "fm")) {
		sat->deframer = DEFRAMER_DUMMY_FM;
	} else {
		*error = "Unsupported deframer, please refer to the valid list";
		ret = -1;
		goto out;
	}

	/** optional */
	if (json_get_int_by_key(deframerObj, "syncword_threshold", &sat->syncwordThreshold)) {
		LOG_V("Syncword Threshold: [ %d ]", sat->syncwordThreshold);
	} else {
		/** TODO: change to define */
		sat->syncwordThreshold = 4;
		LOG_E("Syncword threshold not found, use defaults: [ %d ]", sat->syncwordThreshold);
	}

out:
	return ret;
}

static int rest_api_set_observation(char *payload, char **reply_buf, const char **error)
{
	int ret;
	struct json_object *jObj;
	struct json_object *observationObj;
	observation_t *observation;
	global_stats_t *stats = stats_get_instance();

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

	stats->satellites_scheduled = 0;
	if ((observation = sat_setup_observation()) == NULL) {
		ret = -1;
		goto out;
	}

	json_object *satellite;
	json_object_object_get_ex(observationObj, "satellite", &satellite);

	if (satellite) {
		int i;

		for (i = 0; i < json_object_array_length(satellite); i++) {
			json_object *deframerObj;
			json_object *satelliteObj;
			json_object *modulationObj;

			satellite_t *sat = calloc(1, sizeof(satellite_t));
			if (!satellite) {
				*error = "Out of memory";
				ret = -1;
				goto out;
			}

			sat->obs = observation;
			LIST_INSERT_HEAD(&observation->satellites_list, sat, entries);

			satelliteObj = json_object_array_get_idx(satellite, i);
			if (!satelliteObj) {
				*error = "'/observation/satellite': couldn't get CG array index";
				ret = -1;
				goto out;
			}

			const char *satName = json_get_string_by_key(satelliteObj, "name");
			if (!satName) {
				*error = "'/observation/satellite/name' string is not specified!";
				ret = -1;
				goto out;
			}

			LOG_V("satellite name: [ %s ]", satName);

			sat->network_addr = strdup(json_get_string_by_key(satelliteObj, "network_addr"));
			if (!sat->network_addr) {
				*error = "'/observation/satellite/network_addr' string is not specified!";
				ret = -1;
				goto out;
			}

			LOG_V("Network stream address: [ %s ]", sat->network_addr);

			if (!json_get_int_by_key(satelliteObj, "network_port", &sat->network_port)) {
				*error = "'/observation/satellite/network_port' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("Network stream port: [ %d ]", sat->network_port);

			json_object_object_get_ex(satelliteObj, "modulation", &modulationObj);
			if (!modulationObj) {
				*error = "/observation/satellite/modulation object not found!";
				ret = -1;
				goto out;
			}

			if (rest_api_parse_modulation(sat, modulationObj, error) == -1) {
				ret = -1;
				goto out;
			}

			/* if (sat->modulation != MODULATION_FM) { */
			json_object_object_get_ex(satelliteObj, "deframer", &deframerObj);
			if (!deframerObj) {
				*error = "/observation/satellite/deframer object is required for analog modulation!";
				ret = -1;
				goto out;
			}

			if (rest_api_parse_deframer(sat, deframerObj, error) == -1) {
				ret = -1;
				goto out;
			}
			/* } */

			strncpy(sat->name, satName, sizeof(sat->name) - 1);

			if (!json_get_int_by_key(satelliteObj, "frequency", &sat->frequency)) {
				*error = "'/observation/satellite/frequency' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("satellite frequency: [ %d ]", sat->frequency);

			if (!json_get_double_by_key(satelliteObj, "min_elevation", &sat->min_elevation)) {
				*error = "'/observation/satellite/min_elevation' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("satellite min. elevation: [ %f ]", sat->min_elevation);

			if (!json_get_int_by_key(satelliteObj, "priority", &sat->priority)) {
				*error = "'/observation/satellite/priority' not specified";
				ret = -1;
				goto out;
			}

			LOG_V("satellite priority: [ %d ]", sat->priority);

			stats->satellites_scheduled++;

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

static int rest_api_get_antenna_position(char *payload, char **reply_buf, const char **error)
{
	return rest_api_get_status(payload, reply_buf, error);
}

static int rest_api_set_antenna_position(char *payload, char **reply_buf, const char **error)
{
	int ret;
	double azimuth = false;
	double elevation = false;
	struct json_object *jObj;
	struct json_object *positionObj;
	global_stats_t *stats;
	observation_t *observation;

	ret = 0;
	stats = stats_get_instance();
	observation = sat_get_observation();
	if (!observation) {
		*error = "No observation is set up";
		return -1;
	}

	if (stats->state != GSC_STATE_WAITING) {
		*error = "Calibration, parking, or tracking in progress";
		return -1;
	}

	LOG_V("Set antenna position REST API started");

	LOG_V("Parsing JSON request...");
	jObj = json_tokener_parse(payload);
	if (!jObj) {
		*error = "Couldn't parse JSON request";
		return -1;
	}

	if (!json_object_object_get_ex(jObj, "position", &positionObj)) {
		*error = "'/position' object is missing";
		ret = -1;
		goto out;
	}

	if (!json_get_double_by_key(positionObj, "azimuth", &azimuth)) {
		*error = "'/position/azimuth' not specified";
		ret = -1;
		goto out;
	}

	if (!json_get_double_by_key(positionObj, "elevation", &elevation)) {
		*error = "'/position/elevation' not specified";
		ret = -1;
		goto out;
	}

	stats->last_azimuth = azimuth;
	stats->last_elevation = elevation;

	LOG_I("Moving antenna to az: %f, el: %f", azimuth, elevation);

	rotctl_park_and_wait(observation, azimuth, elevation);
	LOG_V("Set antenna position done");

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
	global_stats_t *stats = stats_get_instance();

	ret = 0;

	LOG_I("Calibration REST API started");
	observation = sat_get_observation();
	if (!observation) {
		*error = "No observation is set up";
		return -1;
	}

	if (stats->state != GSC_STATE_WAITING) {
		*error = "Calibration, parking, or tracking in progress";
		return -1;
	}

	LOG_V("Parsing JSON request...");
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
	{ "/antenna", REST_API_TYPE_POST, rest_api_set_antenna_position},
	{ "/antenna", REST_API_TYPE_GET, rest_api_get_antenna_position}
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
