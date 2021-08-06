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
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#include "ev.h"
#include "sig.h"
#include "sat.h"
#include "log.h"
#include "cfg.h"
#include "helpers.h"

void print_help(void)
{
	fprintf(stderr, "Groung station core v1\n\n"
					" -h, --help                    :  print this help\n"
					" -v, --verbosity <0..3>        :  log verbosity\n"
					" -f, --log-file <filename>     :  redirect logs into a file\n"
					" -r, --remote-addr <ip>        :  remote IP of rotctld daemons\n"
					" -a, --azimuth-port <port>     :  port of the azimuth rotctld\n"
					" -e, --elevation-port <port>   :  port of the elevation rotctld\n"
					" -p, --request-port <port num> :  port number to listen JSON requests\n"
					" -l, --latitude <value>        :  latitude of the GS\n"
					" -o, --longitude <value>       :  longitude of the GS\n"
					" -g, --grc-file <path>         :  path to the gnuradio flowgraph\n"
					" -d, --dry-run                 :  do not invoke an SDR record\n"
		  			);
}

static struct option long_options[] =
{
	{"config-file",			required_argument,	0, 	'c'},
	{"dry-run",   			no_argument,		0, 	'd'},
	{"help",				no_argument,		0, 	'h'},
	{0, 0, 0, 0}
};

int main(int argc, char **argv)
{
	cfg_t *cfg;
	int ret;
	int options;
	int option_index;
	config_t file_cfg;

	/** internal gsc config */
	cfg = alloc_cfg();
	if (!cfg) {
		fprintf(stderr, "%s(): error during init\n", __func__);
		return -1;
	}

	cfg->log_file = stdout;
	cfg->log_level = LVL_VERB;
	strncpy(cfg->version, "v0.1-20022021", sizeof(cfg->version));

	log_init(cfg->log_file, cfg->log_level);

	config_init(&file_cfg);

	ret = cfg_parse(&file_cfg, cfg);
	if (ret == -1) {
		LOG_E("Couldn't parse the config\n");
		config_destroy(&file_cfg);
		return -1;
	}

	do {
		if ((options = getopt_long(argc, argv, "p:r:v:e:a:f:h:g:c:d",
						long_options, &option_index)) == -1) {
			break;
		}

		switch (options) {
			case 'c':
				cfg->dry_run = true;
				break;
			case 'd':
				cfg->dry_run = true;
				break;
			case 'h':
			default:
				print_help();
				goto err;
		}
	} while (options != -1);

	if (!cfg->cli.azimuth_port ||
		!cfg->cli.elevation_port ||
		!strcmp(cfg->cli.remote_ip, "")) {
		fprintf(stderr, "Please specify remote parameters to connect\n");
		goto err;
	}

	if (!cfg->latitude || !cfg->longitude) {
		fprintf(stderr, "Please specify latitude and longitude\n");
		goto err;
	}

	if (sig_register() == -1)
		return -1;

	LOG_V("Dry run:              %s", !!cfg->dry_run ? "true" : "false");
	LOG_V("Latitude:             %f", cfg->latitude);
	LOG_V("Longitude:            %f", cfg->longitude);
	LOG_V("Azimuth port:         %d", cfg->cli.azimuth_port);
	LOG_V("Elevation port:       %d", cfg->cli.elevation_port);
	LOG_V("Controller IP:        %s", cfg->cli.remote_ip);
	LOG_V("Request port:         %d", cfg->listen_port);
	LOG_V("GNU Radio config:     %s", cfg->grc_config);
	LOG_V("GNU Radio flowgraph:  %s", cfg->grc_flowgraph);

	if (ev_probe(cfg->listen_port) == -1)
		return -1;

	ev_process_loop();
	return 0;

err:
	config_destroy(&file_cfg);
	destroy_cfg(cfg);
	return -1;
}
