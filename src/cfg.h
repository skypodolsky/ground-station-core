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

#include <stdbool.h>
#include <libconfig.h>
#include <linux/limits.h>

#define DEF_LISTEN_PORT		25565
#define DEF_CONFIG_NAME		"default.cfg"

typedef struct netcli_t {
	int azimuth_port;			/** port of the azimuth rotctld  	*/
	int elevation_port;			/** port of the elevation rotctld 	*/
	int azimuth_conn_fd;		/** azimuth rotctld socket  		*/
	int elevation_conn_fd;		/** elevation rotctld socket 		*/
	const char *remote_ip;		/** remote IP of rotctld daemons 	*/
} netcli_t;

typedef struct cfg_t {
	int listen_port;			/** port to listen JSON requests 	*/
	char version[64];			/** utility version 				*/
	const char *grc_config;		/** GNU Radio config				*/
	const char *grc_flowgraph;	/** GNU Radio flowgraph				*/
	int log_level;				/** log level 						*/
	FILE *log_file;				/** file to log output in 			*/
	netcli_t cli;				/** data for the remote connection	*/
	double latitude;			/** GS latitude */
	double longitude;			/** GS longitude */
	bool dry_run;
	int calibrate;
} cfg_t;

cfg_t *alloc_cfg(void);
void destroy_cfg(cfg_t *cfg);
cfg_t *cfg_global_get(void);
int cfg_parse(config_t *file_cfg, cfg_t *gsc_cfg);
