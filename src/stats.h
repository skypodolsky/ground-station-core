#pragma once

#include <time.h>

typedef struct global_stats_t {
	int satellites_scheduled;	/** scheduled during the current observation */
	int satellites_preempted;	/** preempted during the current observation */
	int satellites_tracked;		/** tracked during the current observation */
	int satellites_s;			/** not supported now */
	int satellites_vhf;			/** how many VHF satellites tracked */
	int satellites_uhf;			/** how many UHF satellites tracked */
	float last_azimuth;			/** last antenna azimuth */
	float last_elevation;		/** last antenna elevation */
	time_t last_updated;		/** last scheduler timestamp to calculate uptime */
	time_t observation_uptime; 	/** for how long the current observation works */
} global_stats_t;

typedef enum gsc_state {
	GSC_STATE_WAITING,
	GSC_STATE_TRACKING,
	GSC_STATE_NOT_CONFIGURED
} gsc_state;

global_stats_t *stats_get_instance(void);
