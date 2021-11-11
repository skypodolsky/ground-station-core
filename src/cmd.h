#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "sat.h"
#include "log.h"
#include "gnuradio.h"

int pre_doit(observation_t *obs);
int post_doit(observation_t *obs);
int request_cmd(const char *program_name, char *args[]);
