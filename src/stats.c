#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "sat.h"
#include "stats.h"

global_stats_t stats;

global_stats_t *stats_get_instance(void)
{
	return &stats;
}
