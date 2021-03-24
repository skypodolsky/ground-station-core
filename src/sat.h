#pragma once

#include <pthread.h>
#include <predict/predict.h>

#include "cfg.h"
#include "entry.h"

#define MAX_TLE_LEN 			256
#define MAX_TLE_SAT_NAME_LEN 	25 /** 24 chars + null */
#define MAX_GS_NAME_LEN 		16

typedef enum modulation_t {
	MODULATION_FM,
	MODULATION_AFSK
} modulation_t;

typedef struct observation_t observation_t;

typedef struct satellite_t {
	char name[MAX_TLE_SAT_NAME_LEN]; 
	char tle1[MAX_TLE_LEN];
	char tle2[MAX_TLE_LEN];
  	modulation_t modulation;
	double min_elevation;
  	int frequency;
  	int priority;
	time_t next_aos;
	time_t next_los;
	double aos_az;
	double los_az;
	bool zero_transition;
	bool parked;
	observation_t *obs;
	predict_orbital_elements_t *orbital_elements;
  	LIST_ENTRY(satellite_t) entries;
} satellite_t;

typedef struct observation_t {
	float latitude;
	float longitude;
	satellite_t *active;
	char gs_name[MAX_GS_NAME_LEN];
	time_t sim_time; /** used for simulation */
	bool sch_terminate;
	pthread_t sch_thread;
	cfg_t *cfg;
	int sdr_pid;
	predict_observer_t *observer;
	LIST_HEAD(satellites_list_head, satellite_t) satellites_list;
} observation_t;

int sat_reschedule_all(void);
int sat_setup(satellite_t *sat);
observation_t *sat_get_observation(void);
observation_t *sat_setup_observation(void);

void sat_move_to_observation(void);
void sat_simul_time_step(time_t timestep);
void sat_simul_time_set(time_t val);
