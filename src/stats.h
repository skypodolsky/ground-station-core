#pragma once

#include <time.h>

typedef struct global_stats_t {
	int satellites_scheduled;	/** scheduled during the current observation */
	int satellites_preempted;	/** preempted during the current observation */
	int satellites_tracked;		/** tracked during the current observation */
	int satellites_s;			/** not supported now */
	int satellites_vhf;			/** how many VHF satellites tracked */
	int satellites_uhf;			/** how many UHF satellites tracked */
	time_t last_updated;		/** last scheduler timestamp to calculate uptime */
	time_t observation_uptime; 	/** for how long the current observation works */
} global_stats_t;

global_stats_t *stats_get_instance(void);
