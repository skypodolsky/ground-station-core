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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sdr.h"
#include "log.h"
#include "cmd.h"
#include "gnuradio.h"

int sdr_start(satellite_t *sat, const char *filename)
{
	if (!sat || !sat->obs || !filename)
		return -1;

	observation_t *obs = sat->obs;
	char *args[] = { DEF_CONF_PYTHON, (char *) obs->cfg->grc_flowgraph, NULL };

	if (sdr_prepare_config(obs->cfg, sat, filename) == -1) {
		return -1;
	}

	obs->sdr_pid = request_cmd(DEF_CONF_PYTHON, args);
	if (obs->sdr_pid == -1)
		return -1;

	LOG_I("Request program: pid %d", obs->sdr_pid);

	return 0;
}

int sdr_stop(observation_t *obs)
{
	if (!obs)
		return -1;

	if (obs->sdr_pid) {
		kill(obs->sdr_pid, SIGINT);
		obs->sdr_pid = 0;
	}

	return 0;
}
