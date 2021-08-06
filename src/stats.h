#pragma once

#include <time.h>

typedef struct global_stats_t {
	int satellites_scheduled;
	int satellites_preempted;
	int satellites_tracked;
	time_t last_updated;
	time_t observation_uptime; 	/** how long the current observation works */
} global_stats_t;

global_stats_t *stats_get_instance(void);
