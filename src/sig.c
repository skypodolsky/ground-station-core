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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "sig.h"
#include "sat.h"
#include "log.h"

static void sig_handle_sigint(int sig)
{
	log_save();
	exit(0);
}

/** Used for simulation to set needed time */
static void sig_handle_sigusr1(int sig)
{
#if 0
	time_t current_time;
	struct tm timeval = {0};
	timeval.tm_year = 2021-1900;
	timeval.tm_mon = 1;
	timeval.tm_mday = 26;
	timeval.tm_hour = 6;
	timeval.tm_min = 48;

	time(&current_time);
	sat_simul_time_set(mktime(&timeval) - current_time);
#endif
	sat_move_to_observation();
}

/** Used for simulation to increment time by time step */
static void sig_handle_sigusr2(int sig)
{
	sat_simul_time_step(30);
}

static void sig_handle_sighup(int sig)
{
	sat_reschedule_all();
}

int sig_register(void)
{
	int ret = 0;

	if (signal(SIGUSR1, sig_handle_sigusr1) == SIG_ERR) {
		LOG_E("Could not register signal handler: SIGUSR1");
		ret = -1;
	}

	if (signal(SIGUSR2, sig_handle_sigusr2) == SIG_ERR) {
		LOG_E("Could not register signal handler: SIGUSR2");
		ret = -1;
	}

	if (signal(SIGINT, sig_handle_sigint) == SIG_ERR) {
		LOG_E("Could not register signal handler: SIGINT");
		ret = -1;
	}

	if (signal(SIGHUP, sig_handle_sighup) == SIG_ERR) {
		LOG_E("Could not register signal handler: SIGHUP");
		ret = -1;
	}

	return ret;
}
