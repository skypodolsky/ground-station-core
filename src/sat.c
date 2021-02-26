#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "log.h"
#include "sat.h"
#include "sdr.h"
#include "rotctl.h"
#include "helpers.h"

static observation_t *_observation;

void sat_simul_time_step(time_t timestep)
{
	if (_observation) {
		_observation->sim_time += timestep;
		LOG_V("Time increment: %ld", _observation->sim_time);
	}
}

void sat_simul_time_set(time_t val)
{
	if (_observation)
		_observation->sim_time = val;

	LOG_V("Manual time set: %ld", _observation->sim_time);
}


static void newline_guillotine(char *str)
{
	char *pos;

	if ((pos = strchr(str, '\r')) != NULL)
		*pos = '\0';

	if ((pos = strchr(str, '\n')) != NULL)
		*pos = '\0';
}

static bool sat_is_crossing_zero(double aos_az, double los_az)
{
	if (los_az > 180.0) {
		if (los_az - (aos_az + 180.0) > 0)
			return true;
		else
			return false;
	} else {
		if (los_az - (aos_az - 180.0) < 0)
			return true;
		else
			return false;
	}
}

static double sat_reverse_azimuth(double az)
{
	return (az  > 180.0) ? az - 180.0 : az + 180.0;
}

static void *sat_tracking_az(void *opt)
{
	int time_delay = 100000;
	time_t current_time;
	observation_t *obs;
	struct predict_position orbit;
	struct predict_observation observation;

	obs = (observation_t *) opt;
	if (!obs)
		return NULL;

	LOG_I("Started azimuth control thread.");
	LOG_V("Current time=%d, %s LOS time %d", current_time, obs->active->name, obs->active->next_los);

	do {
		time(&current_time);

		/** FIXME: DELETE ME! */
		current_time += 20;

		if (obs->sim_time)
			current_time += obs->sim_time;

		predict_orbit(obs->active->orbital_elements, &orbit, predict_to_julian(current_time));
		predict_observe_orbit(obs->observer, &orbit, &observation);

		double az = rad_to_deg(observation.azimuth);

		if (obs->active->zero_transition) {
			az = sat_reverse_azimuth(az);
		}

		LOG_V("Az: %.02f", az);

		rotctl_send_az(obs, az);
		usleep(time_delay);
	} while (current_time < obs->active->next_los);

	LOG_V("Azimuth thread done.");
	/** FIXME */
	return NULL;
}

static void *sat_tracking_el(void *opt)
{
	int time_delay = 100000;
	time_t current_time;
	observation_t *obs;
	struct predict_position orbit;
	struct predict_observation observation;

	obs = (observation_t *) opt;
	if (!obs)
		return NULL;

	LOG_I("Started elevation control thread.");
	LOG_V("Current time=%d, %s LOS time %d", current_time, obs->active->name, obs->active->next_los);

	do {
		time(&current_time);

		/** FIXME: DELETE ME! */
		current_time += 20;

		if (obs->sim_time)
			current_time += obs->sim_time;

		predict_orbit(obs->active->orbital_elements, &orbit, predict_to_julian(current_time));
		predict_observe_orbit(obs->observer, &orbit, &observation);

		double el = rad_to_deg(observation.elevation);
		double shift = predict_doppler_shift(&observation, obs->active->frequency);

		if (obs->active->zero_transition) {
			el = 180 - el;
		}

		LOG_V("El: %.02f, Doppler shift = %f", el, shift);
		sdr_set_freq(obs->active->frequency + shift);

		rotctl_send_el(obs, el);
		usleep(time_delay);
	} while (current_time < obs->active->next_los);

	LOG_V("Elevation thread done.");
	/** FIXME */
	return NULL;
}

static void *sat_scheduler(void *opt)
{
	time_t current_time;

	satellite_t *sat;
	observation_t *obs = (observation_t *) opt;

	if (obs == NULL)
		return NULL;

	LOG_I("Scheduler started");
	fflush(stdout); /** just to avoid mixing up the output: it happens sometimes */
	while (obs->sch_terminate == false) {

		time(&current_time);

		if (obs->sim_time)
			current_time += obs->sim_time;

		if (obs->active == NULL) {

			LIST_FOREACH(sat, &obs->satellites_list, entries) {

				if (sat->next_aos > 0) {
					if ((current_time > (sat->next_aos - 120)) && sat->parked == false) {
						LOG_V("Parking antenna for the receiving of %s", sat->name);
						if (!sat->zero_transition)
							rotctl_send_and_wait(obs, sat->aos_az, 0);
						else
							rotctl_send_and_wait(obs, sat->aos_az, 180);

						LOG_V("Parking done");
						sat->parked = true;
					}
					if (current_time > sat->next_aos) {

						obs->active = sat;

						LOG_I("Tracking started: %s", sat->name);
						break;
					}
				}
			}
		} else {
			pthread_t az_thread;
			pthread_t el_thread;

			if (sdr_start(obs->active) == -1) {
				LOG_E("Couldn't start SDR");
			}

			pthread_create(&az_thread, NULL, sat_tracking_az, obs);
			pthread_create(&el_thread, NULL, sat_tracking_el, obs);
			pthread_join(az_thread, NULL);
			pthread_join(el_thread, NULL);

			if (sdr_stop(obs) == -1) {
				LOG_E("Couldn't stop SDR");
			}

			if (sat_setup(obs->active) == -1) {
				LOG_E("Error while rescheduling %s", obs->active->name);
			}

			obs->active->parked = false;
			LOG_V("Rescheduled %s", obs->active->name);
			obs->active = NULL;
		}

	}
	LOG_I("Scheduler terminated");
	return NULL;
}

static int sat_fetch_tle(const char *name, char *tle1, char *tle2)
{
	int ret;
	int size;
	FILE *fd;
	char *buf;
	bool found;

	ret = 0;
	buf = NULL;
	found = false;

	if (tle1 == NULL || tle2 == NULL) {
		ret = -1;
		goto out;
	}

	fd = fopen("active.txt", "r");
	if (fd == NULL) {
		LOG_C("TLE file not found");
		ret = -1;
		goto out;
	}

	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	if ((buf = malloc(size + 1)) == NULL) {
		LOG_C("Error of malloc()");
		ret = -1;
		goto out;
	}

	buf[0] = 0;

	while(!feof(fd)) {
		fgets(buf, 32, fd);
		if (!strncmp(buf, name, strlen(name))) {
			fgets(tle1, MAX_TLE_LEN, fd);
			fgets(tle2, MAX_TLE_LEN, fd);
			newline_guillotine(tle1);
			newline_guillotine(tle2);
			found = true;
			break;
		}
	}

	if (!found) {
		LOG_E("Satellite %s not found", name);
		ret = -1;
		goto out;
	}

out:
	if (buf)
		free(buf);

	if (fd)
		fclose(fd);

	return ret;
}

int sat_predict(satellite_t *sat)
{
	int ret;
	float max_elev;
	time_t current_time;
	struct predict_observation aos;
	struct predict_observation los;
	/* struct predict_position orbit; */
	/* struct predict_observation observation; */
	struct predict_observation elev;

	ret = 0;

	if (!sat)
		return -1;

	time(&current_time);

	if (sat->obs->sim_time)
		current_time += sat->obs->sim_time;

	predict_julian_date_t start_time = predict_to_julian(current_time);

reschedule:

	do {
		elev = predict_at_max_elevation(sat->obs->observer, sat->orbital_elements, start_time);
		aos = predict_next_aos(sat->obs->observer, sat->orbital_elements, start_time);
		los = predict_next_los(sat->obs->observer, sat->orbital_elements, start_time);
		start_time = los.time;
		max_elev = rad_to_deg(elev.elevation);
	} while (max_elev < sat->min_elevation);

	if (max_elev > 0) {
		struct tm tm_aos;
		struct tm tm_los;
		char aos_buf[32];
		char los_buf[32];
		struct predict_position orbit;
		struct predict_observation observation;

		predict_orbit(sat->orbital_elements, &orbit, aos.time);
		predict_observe_orbit(sat->obs->observer, &orbit, &observation);

		time_t aos_time_t = predict_from_julian(aos.time);
		time_t los_time_t = predict_from_julian(los.time);

		tm_aos = *localtime(&aos_time_t);
		tm_los = *localtime(&los_time_t);

		strftime(aos_buf, sizeof(aos_buf), "%d %B %Y - %I:%M%p %Z", &tm_aos);
		strftime(los_buf, sizeof(los_buf), "%d %B %Y - %I:%M%p %Z", &tm_los);

		sat->next_aos = aos_time_t;
		sat->next_los = los_time_t;
		sat->aos_az = rad_to_deg(observation.azimuth);

		predict_orbit(sat->orbital_elements, &orbit, los.time);
		predict_observe_orbit(sat->obs->observer, &orbit, &observation);

		sat->los_az = rad_to_deg(observation.azimuth);
		LOG_V("Max elevation %f deg., AOS on %s (az. %f), LOS on %s (az. %f)", max_elev, aos_buf, sat->aos_az, los_buf, sat->los_az);

		sat->zero_transition = sat_is_crossing_zero(sat->aos_az, sat->los_az);
		if (sat->zero_transition) {
			sat->aos_az = sat_reverse_azimuth(sat->aos_az);
			sat->los_az = sat_reverse_azimuth(sat->los_az);
			LOG_V("Zero transition, reversing azimuths");
			LOG_V("New AOS azimuth: %f", sat->aos_az);
			LOG_V("New LOS azimuth: %f", sat->los_az);
		}
		else
			LOG_V("No zero transition.");

	} else {
		ret = -1;
		LOG_E("Couldn't find the needed elevation");
	}

	satellite_t *iter;
	observation_t *obs = (observation_t *) sat->obs;

	/** reschedule if overlapped */
	LIST_FOREACH(iter, &obs->satellites_list, entries) {

		if (iter == sat)
			continue;

		if ((sat->next_los > iter->next_aos && sat->next_los < iter->next_los) ||
				(sat->next_aos < iter->next_los && sat->next_aos > iter->next_aos)) {

			if (sat->priority > iter->priority) {
				LOG_V("Overlap with %s found, %s rescheduled", iter->name, iter->name);
				sat_predict(iter);
			} else {
				LOG_V("Overlap with %s found, %s rescheduled", iter->name, sat->name);
				goto reschedule;
			}
		}
	}

	return ret;
}

int sat_setup(satellite_t *sat)
{
	int ret;
	char tle1[MAX_TLE_LEN] = { 0 };
	char tle2[MAX_TLE_LEN] = { 0 };

	ret = 0;

	if ((ret = sat_fetch_tle(sat->name, tle1, tle2)) == -1) {
		goto out;
	}

	strcpy(sat->tle1, tle1);
	strcpy(sat->tle2, tle2);
	sat->orbital_elements = predict_parse_tle(sat->tle1, sat->tle2);

	LOG_I("Satellite found in TLE: [%s]", sat->name);
	LOG_V("TLE1: [%s]", tle1);
	LOG_V("TLE2: [%s]", tle2);

	sat_predict(sat);

out:
	return ret;
}

static observation_t *sat_alloc_observation_data(void)
{
	observation_t *obs = calloc(1, sizeof(observation_t));
	if (obs == NULL) {
		LOG_E("Error on malloc");
		return NULL;
	}

	obs->latitude = 48.5833f; /** FIXME */
	obs->longitude = 7.75f; /** FIXME */
	strncpy(obs->gs_name, "ISU GS", sizeof(obs->gs_name));

	setenv("TZ", "GMT", 1);
	tzset();

	LIST_INIT(&(obs)->satellites_list);

	obs->sch_terminate = false;
	obs->cfg = cfg_global_get();

	if (rotctl_open(obs, ROT_TYPE_AZ) == -1) {
		LOG_E("Couldn't establish connection with azimuth rotctld\n");
	}

	if (rotctl_open(obs, ROT_TYPE_EL) == -1) {
		LOG_E("Couldn't establish connection with elevation rotctld\n");
	}

	obs->observer = predict_create_observer("ISU GS", obs->latitude * M_PI / 180.0, obs->longitude * M_PI / 180.0, 10);

	pthread_create(&obs->sch_thread, NULL, sat_scheduler, obs);
	return obs;
}

static int sat_clear_all(observation_t *obs)
{
	satellite_t *sat;

	obs->sch_terminate = true;
	pthread_join(obs->sch_thread, NULL);

	while (!LIST_EMPTY(&obs->satellites_list)) {
		sat = LIST_FIRST(&obs->satellites_list);
		predict_destroy_orbital_elements(sat->orbital_elements);
		LIST_REMOVE(sat, entries);
		free(sat);
	}

	rotctl_close(obs, ROT_TYPE_AZ);
	rotctl_close(obs, ROT_TYPE_EL);

	predict_destroy_observer(obs->observer);
	free(obs);

	return 0;
}

observation_t *sat_setup_observation()
{
	if (_observation) {
		LOG_V("Remove old observation entries");
		sat_clear_all(_observation);
	}

	if ((_observation = sat_alloc_observation_data()) == NULL) {
		LOG_V("Couldn't create a new observation entry");
	}

	return _observation;
}

int sat_reschedule_all()
{
	satellite_t *iter;
	observation_t *obs = _observation;

	if (_observation == NULL)
		return -1;

	LIST_FOREACH(iter, &obs->satellites_list, entries) {
		predict_destroy_orbital_elements(iter->orbital_elements);
		if (sat_setup(iter) == -1) {
			LOG_E("Error rescheduling satellite %s", iter->name);
			return -1;
		}
	}
	return 0;
}
