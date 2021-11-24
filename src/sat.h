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

#include <pthread.h>
#include <sys/queue.h>
#include <predict/predict.h>

#include "cfg.h"

#define MAX_TLE_LEN 			256
#define MAX_TLE_SAT_NAME_LEN 	25 /** 24 chars + null */
#define MAX_GS_NAME_LEN 		16

#define LOW_VHF_BAND			30000000 /** 30 MHZ */
#define HIGH_VHF_BAND			300000000 /** 300 MHZ */
#define LOW_UHF_BAND			300000000 /** 300 MHZ */
#define HIGH_UHF_BAND			3000000000 /** 3000 MHZ */

typedef enum modulation_t {
	MODULATION_BPSK,
	MODULATION_AFSK,
	MODULATION_FSK,
	MODULATION_FM
} modulation_t;

typedef enum deframer_t {
	DEFRAMER_AO_40,
	DEFRAMER_GOMSPACE_U482C,
	DEFRAMER_AX25,
	DEFRAMER_FOSSASAT,
	DEFRAMER_GOMSPACE_AX100_RS,
	DEFRAMER_GOMSPACE_AX100_ASM_GOLAY,
	DEFRAMER_DUMMY_FM
} deframer_t;

typedef struct observation_t observation_t;

typedef struct satellite_t {
	char name[MAX_TLE_SAT_NAME_LEN]; 
	char tle1[MAX_TLE_LEN];
	char tle2[MAX_TLE_LEN];

  	modulation_t modulation;
	deframer_t deframer;

	const char *network_addr;
	int network_port;
	int baudRate;

	/** bpsk */
	bool bpskManchester;
	bool bpskDifferential;

	/** afsk */
	int afskDeviation;
	int afskAFC; /** audio frequency carrier */

	/** fsk */
	bool fskSubAudio;

	/** deframing */
	int syncwordThreshold;
	bool shortFrames;
	bool g3ruh;
	bool crc16;

	double min_elevation;
	double max_elevation;
  	int frequency;
  	int bandwidth;
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
satellite_t *sat_find_next(void);
void sat_simul_time_step(time_t timestep);
void sat_simul_time_set(time_t val);
