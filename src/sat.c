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

#define AZ_OFFSET 	5.0f
#define AZ_MAX 		352.0f // 354?

static observation_t *_observation;

void sat_simul_time_step(time_t timestep)
{
	if (_observation) {
		_observation->sim_time += timestep;
		LOG_V("Time travel +%ld sec: %ld", timestep, _observation->sim_time);
	}
}

void sat_simul_time_set(time_t val)
{
	if (_observation)
		_observation->sim_time = val;

	LOG_V("Time travel to: %ld", _observation->sim_time);
}

void sat_move_to_observation()
{
	time_t target = 0xFFFFFFFF;
	time_t current_time;
	satellite_t *sat;
	observation_t *obs;

	obs = _observation;
	if (obs == NULL)
		return;

	current_time = time(NULL);

	if (obs->active) {
		target = obs->active->next_los;
		LOG_I("Tracking active, moving to LOS");
		goto out;
	}

	if (LIST_EMPTY(&obs->satellites_list))
		return;

	LIST_FOREACH(sat, &obs->satellites_list, entries) {
		if (sat->next_aos < target)
			target = sat->next_aos;
	}

	LOG_V("Closest AOS found: %d", target);

out:
	target -= 120;
	obs->sim_time = (target - current_time);

	LOG_V("Time travel to: %ld", target);
}

static void newline_guillotine(char *str)
{
	char *pos;

	if ((pos = strchr(str, '\r')) != NULL)
		*pos = '\0';

	if ((pos = strchr(str, '\n')) != NULL)
		*pos = '\0';
}

static void space_replace(char *str)
{
	int i;

	for (i = 0; i < strlen(str); i++)
		if (str[i] == ' ')
			str[i] = '_';
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

static double sat_apply_azimuth_offset(double az, double offset)
{
	return fmod(az + offset, 360);
}

static double sat_fix_azimuth(double az)
{
	if (az > AZ_MAX) {
		LOG_V("Cannot set azimuth %f, restricting (wrong range)", az);
		az = AZ_MAX;
	}

	return az;
}

static void sat_send_notification(const char *name, const char *filename)
{
	char buf[512]; /** filename can be up to 256 bytes */

	snprintf(buf, sizeof(buf), "isu_gs_send_email.sh '%s' '%s'", name, filename);
	system(buf);
}

static void *sat_tracking_az(void *opt)
{
	int time_delay = 50000;
	time_t current_time;
	observation_t *obs;
	struct predict_position orbit;
	struct predict_observation observation;

	obs = (observation_t *) opt;
	if (!obs)
		return NULL;

	time(&current_time);

	if (obs->sim_time)
			current_time += obs->sim_time;

	LOG_I("Started azimuth control thread.");
	LOG_V("Current time=%d, %s LOS time %d", current_time, obs->active->name, obs->active->next_los);

	do {
		time(&current_time);

		if (obs->sim_time)
			current_time += obs->sim_time;

		predict_orbit(obs->active->orbital_elements, &orbit, predict_to_julian(current_time));
		predict_observe_orbit(obs->observer, &orbit, &observation);

		double az = rad_to_deg(observation.azimuth);

		az = sat_apply_azimuth_offset(az, AZ_OFFSET);

		if (obs->active->zero_transition) {
			az = sat_reverse_azimuth(az);
		}

		if (az != sat_fix_azimuth(az)) {
			LOG_V("Finishing azimuth thread due to reaching azimuth limit");
			break;
		}

		LOG_V("Az: %.02f", az);

		rotctl_send_az(obs, az);
		usleep(time_delay);
	} while (current_time < obs->active->next_los);

	LOG_V("Azimuth thread done.");
	LOG_V("Current time=%ld", current_time);
	/** FIXME */
	return NULL;
}

static void *sat_tracking_el(void *opt)
{
	int time_delay = 50000;
	time_t current_time;
	observation_t *obs;
	struct predict_position orbit;
	struct predict_observation observation;

	obs = (observation_t *) opt;
	if (!obs)
		return NULL;

	time(&current_time);

	if (obs->sim_time)
			current_time += obs->sim_time;

	LOG_I("Started elevation control thread.");
	LOG_V("Current time=%d, %s LOS time %d", current_time, obs->active->name, obs->active->next_los);

	do {
		time(&current_time);

		if (obs->sim_time)
			current_time += obs->sim_time;

		predict_orbit(obs->active->orbital_elements, &orbit, predict_to_julian(current_time));
		predict_observe_orbit(obs->observer, &orbit, &observation);

		double el = rad_to_deg(observation.elevation);

		if (obs->active->zero_transition) {
			el = 180 - el;
		}

		LOG_V("El: %.02f", el);
		rotctl_send_el(obs, el);
		usleep(time_delay);
	} while (current_time < obs->active->next_los);

	LOG_V("Elevation thread done.");
	LOG_V("Current time=%ld", current_time);
	/** FIXME */
	return NULL;
}

static void *sat_tracking_doppler(void *opt)
{
	int time_delay = 5000000 + (rand() % 500000);
	time_t current_time;
	observation_t *obs;
	struct predict_position orbit;
	struct predict_observation observation;

	obs = (observation_t *) opt;
	if (!obs)
		return NULL;

	time(&current_time);

	if (obs->sim_time)
			current_time += obs->sim_time;

	LOG_I("Started Doppler control thread.");

	do {
		time(&current_time);

		if (obs->sim_time)
			current_time += obs->sim_time;

		predict_orbit(obs->active->orbital_elements, &orbit, predict_to_julian(current_time));
		predict_observe_orbit(obs->observer, &orbit, &observation);

		double shift = predict_doppler_shift(&observation, obs->active->frequency);

		if (obs->cfg->dry_run == false) {
			sdr_set_freq(shift);
		}

		usleep(time_delay);
	} while (current_time < obs->active->next_los);

	LOG_V("Doppler thread done.");
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
						LOG_I("Parking antenna for the receiving of %s", sat->name);
						LOG_V("curr time = %ld, aos time = %ld", current_time, sat->next_aos);
						rotctl_stop(obs);
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
						LOG_V("curr time = %ld, aos time = %ld", current_time, sat->next_aos);
						break;
					}
				}
			}
		} else {
			char filename[256];
			struct tm timeval_aos;
			struct tm timeval_los;
			pthread_t az_thread;
			pthread_t el_thread;
			pthread_t doppler_thread;

			timeval_aos = *localtime(&obs->active->next_aos);
			timeval_los = *localtime(&obs->active->next_los);

			snprintf(filename, sizeof(filename), "%s_%.0f_deg_%.02d_%.02d_%.04d-%.02d%.02d%.02d_%.02d%.02d%.02d_GMT.wav",
					obs->active->name, obs->active->max_elevation, timeval_aos.tm_mday, timeval_aos.tm_mon + 1, timeval_aos.tm_year + 1900,
					timeval_aos.tm_hour, timeval_aos.tm_min, timeval_aos.tm_sec, timeval_los.tm_hour, timeval_los.tm_min, timeval_los.tm_sec);

			space_replace(filename);

			if (obs->cfg->dry_run == false) {
				if (sdr_start(obs->active, filename) == -1) {
					LOG_E("Couldn't start SDR");
				}
			}

			pthread_create(&az_thread, NULL, sat_tracking_az, obs);
			pthread_create(&el_thread, NULL, sat_tracking_el, obs);
			pthread_create(&doppler_thread, NULL, sat_tracking_doppler, obs);
			pthread_join(az_thread, NULL);
			pthread_join(el_thread, NULL);
			pthread_join(doppler_thread, NULL);

			if (obs->cfg->dry_run == false) {
				if (sdr_stop(obs) == -1) {
					LOG_E("Couldn't stop SDR");
				}

				sat_send_notification(obs->active->name, filename);
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
	struct predict_observation elev;

	ret = 0;

	if (!sat)
		return -1;

	time(&current_time);

	if (sat->obs->sim_time)
		current_time += sat->obs->sim_time;

	LOG_I("SCHEDULE: current time=%ld", current_time);

	predict_julian_date_t start_time = predict_to_julian(current_time);

reschedule:

	do {
		elev = predict_at_max_elevation(sat->obs->observer, sat->orbital_elements, start_time);
		aos = predict_next_aos(sat->obs->observer, sat->orbital_elements, start_time);
		los = predict_next_los(sat->obs->observer, sat->orbital_elements, aos.time);
		start_time = los.time;
		max_elev = rad_to_deg(elev.elevation);
	} while (max_elev < sat->min_elevation);

	if (max_elev > 0) {
		struct tm tm_aos = { 0 };
		struct tm tm_los = { 0 };
		char aos_buf[32] = { 0 };
		char los_buf[32] = { 0 };
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
		LOG_V("Adjusting azimuths according to the restrictions...");

		sat->max_elevation = max_elev;
		sat->aos_az = sat_apply_azimuth_offset(sat->aos_az, AZ_OFFSET);
		sat->los_az = sat_apply_azimuth_offset(sat->los_az, AZ_OFFSET);

		LOG_V("New AOS azimuth: %f", sat->aos_az);
		LOG_V("New LOS azimuth: %f", sat->los_az);
		LOG_I("aos_time=%ld ::: los_time=%ld", aos_time_t, los_time_t);

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

		sat->aos_az = sat_fix_azimuth(sat->aos_az);
		sat->los_az = sat_fix_azimuth(sat->los_az);
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

			if (sat->priority < iter->priority) {
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

	strncpy(obs->gs_name, "ISU GS", sizeof(obs->gs_name));

	setenv("TZ", "GMT", 1);
	tzset();

	LIST_INIT(&(obs)->satellites_list);

	obs->sch_terminate = false;
	obs->cfg = cfg_global_get();

	obs->latitude = obs->cfg->latitude; /* 48.31237f = ISU GS */
	obs->longitude = obs->cfg->longitude; /* 7.44126f = ISU GS */

	if (rotctl_open(obs, ROT_TYPE_AZ) == -1) {
		LOG_C("Couldn't establish connection with azimuth rotctld\n");
	}

	if (rotctl_open(obs, ROT_TYPE_EL) == -1) {
		LOG_C("Couldn't establish connection with elevation rotctld\n");
	}

	LOG_I("Creating observer at %f %f", obs->latitude, obs->longitude);
	obs->observer = predict_create_observer("ISU GS", obs->latitude * M_PI / 180.0, obs->longitude * M_PI / 180.0, 20);

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

observation_t *sat_get_observation(void)
{
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
