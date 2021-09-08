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
#include <libconfig.h>

#include "log.h"
#include "cfg.h"

cfg_t *_cfg;

cfg_t *alloc_cfg(void)
{
	_cfg = calloc(sizeof(cfg_t), 1);
	if (!_cfg)
		return NULL;

	return _cfg;
}

void destroy_cfg(cfg_t *cfg)
{
	if (cfg) {
		free(cfg);
		cfg = NULL;
	}
}

cfg_t *cfg_global_get(void)
{
	return _cfg;
}

int cfg_parse(config_t *file_cfg, cfg_t *gsc_cfg)
{
	int ret;

	ret = 0;

	if (!config_read_file(file_cfg, DEF_CONFIG_NAME)) {
		LOG_E("Couldn't read file: %s", config_error_text(file_cfg));
		return -1;
	}

	if (!config_lookup_string(file_cfg, "gnuradio-config", &gsc_cfg->grc_config)) {
		LOG_E("No 'gnuradio-config' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_string(file_cfg, "gnuradio-flowgraph", &gsc_cfg->grc_flowgraph)) {
		LOG_E("No 'gnuradio-flowgraph' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_int(file_cfg, "azimuth-port", &gsc_cfg->cli.azimuth_port)) {
		LOG_E("No 'azimuth-port' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_int(file_cfg, "elevation-port", &gsc_cfg->cli.elevation_port)) {
		LOG_E("No 'elevation-port' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_string(file_cfg, "remote-addr", &gsc_cfg->cli.remote_ip)) {
		LOG_E("No 'remote-ip' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_int(file_cfg, "request-port", &gsc_cfg->listen_port)) {
		LOG_E("No 'request-port' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_int(file_cfg, "verbosity", &gsc_cfg->log_level)) {
		LOG_E("No 'verbosity' setting in configuration file");
		ret = -1;
	}

	/** FIXME: not mandatory? */
	if (!config_lookup_int(file_cfg, "calibrate", &gsc_cfg->calibrate)) {
		LOG_E("No 'calibrate' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_float(file_cfg, "latitude", &gsc_cfg->latitude)) {
		LOG_E("No 'latitude' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_float(file_cfg, "longitude", &gsc_cfg->longitude)) {
		LOG_E("No 'longitude' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_int(file_cfg, "sdr_bb_gain", &gsc_cfg->bb_gain)) {
		LOG_E("No 'sdr_bb_gain' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_int(file_cfg, "sdr_if_gain", &gsc_cfg->if_gain)) {
		LOG_E("No 'sdr_if_gain' setting in configuration file");
		ret = -1;
	}

	if (!config_lookup_int(file_cfg, "sdr_lna_gain", &gsc_cfg->lna_gain)) {
		LOG_E("No 'sdr_lna_gain' setting in configuration file");
		ret = -1;
	}

	return ret;
}
