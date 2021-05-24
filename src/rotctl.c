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
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>

#include "log.h"
#include "sat.h"
#include "rotctl.h"

int rotctl_open(observation_t *obs, rot_type_t type)
{
	int ret = 0;
	int *fd = NULL;
	struct sockaddr_in server_addr = { 0 };

	server_addr.sin_family = AF_INET;

	if (type == ROT_TYPE_AZ) {
		fd = &obs->cfg->cli.azimuth_conn_fd;
		server_addr.sin_port = htons(obs->cfg->cli.azimuth_port);
	} else {
		fd = &obs->cfg->cli.elevation_conn_fd;
		server_addr.sin_port = htons(obs->cfg->cli.elevation_port);
	}

	if ((ret = inet_pton(AF_INET, obs->cfg->cli.remote_ip, &server_addr.sin_addr)) != 1) {
		LOG_C("Error on inet_pton");
		return -1;
	}

	if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOG_C("Error on socket");
		return -1;
	}

	if ((ret = connect(*fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1) {
		LOG_C("Error on connect");
		return -1;
	}

	return ret;
}

int rotctl_close(observation_t *obs, rot_type_t type)
{
	int ret;
	int *fd = NULL;

	if (obs == NULL)
		return -1;

	fd = (type == ROT_TYPE_AZ) ? &obs->cfg->cli.azimuth_conn_fd : &obs->cfg->cli.elevation_conn_fd;

	if ((ret = shutdown(*fd, SHUT_RDWR)) == -1) {
		LOG_E("Error on shutdown");
		return -1;
	}

	if ((ret = close(*fd)) == -1) {
		LOG_E("Error on shutdown");
		return -1;
	}

	/** FIXME: replace the return codes to defines */
	return 0;
}

int rotctl_stop(observation_t *obs)
{
	int ret;
	char buf[32];
	char rxbuf[4096];

	ret = 0;

	if (obs == NULL)
		return -1;

	snprintf(buf, sizeof(buf), "w S\n");

	write(obs->cfg->cli.azimuth_conn_fd, buf, strlen(buf));
	read(obs->cfg->cli.azimuth_conn_fd, rxbuf, sizeof(rxbuf));
	write(obs->cfg->cli.elevation_conn_fd, buf, strlen(buf));
	read(obs->cfg->cli.elevation_conn_fd, rxbuf, sizeof(rxbuf));

	LOG_V("rotctl_stop() command done");
	return ret;
}

int rotctl_calibrate(observation_t *obs, bool azimuth, bool elevation)
{
	int ret;
	char buf[32] = { 0 };
	char rxbuf[4096] = { 0 };

	ret = 0;

	if (obs == NULL)
		return -1;

	snprintf(buf, sizeof(buf), "w CAL\n");

	if (azimuth) {
		write(obs->cfg->cli.azimuth_conn_fd, buf, strlen(buf));
		read(obs->cfg->cli.azimuth_conn_fd, rxbuf, sizeof(rxbuf));
	}

	if (elevation) {
		write(obs->cfg->cli.elevation_conn_fd, buf, strlen(buf));
		read(obs->cfg->cli.elevation_conn_fd, rxbuf, sizeof(rxbuf));
	}

	LOG_V("rotctl_calibrate() command done");
	return ret;
}

int rotctl_send_and_wait(observation_t *obs, double az, double el)
{
	int ret;
	char az_buf[32] = { 0 };
	char el_buf[32] = { 0 };
	char rxbuf[4096] = { 0 };

	ret = 0;

	if (obs == NULL)
		return -1;

	snprintf(az_buf, sizeof(az_buf), "w A%.02f\n", az);
	snprintf(el_buf, sizeof(el_buf), "w E%.02f\n", el);

	/* while (read(obs->cfg->cli.azimuth_conn_fd, rxbuf, sizeof(rxbuf)) > 0) */

	write(obs->cfg->cli.azimuth_conn_fd, az_buf, strlen(az_buf));
	write(obs->cfg->cli.elevation_conn_fd, el_buf, strlen(el_buf));
	read(obs->cfg->cli.azimuth_conn_fd, rxbuf, sizeof(rxbuf));
	read(obs->cfg->cli.elevation_conn_fd, rxbuf, sizeof(rxbuf));

	LOG_V("rotctl_send_and_wait() command done");
	return ret;
}

static int rotctl_send(observation_t *obs, bool az, double val)
{
	int fd;
	int ret;
	char val_buf[32] = { 0 };
	char rxbuf[4096] = { 0 };

	ret = 0;

	if (obs == NULL)
		return -1;

	fd = az ? obs->cfg->cli.azimuth_conn_fd : obs->cfg->cli.elevation_conn_fd;
	snprintf(val_buf, sizeof(val_buf), "w " "%s" "%.02f\n", az ? "A" : "E", val);

	write(fd, val_buf, strlen(val_buf));
	read(fd, rxbuf, sizeof(rxbuf));

	LOG_V("rotctl_send() command done");
	return ret;
}

int rotctl_send_az(observation_t *obs, double az)
{
	return rotctl_send(obs, true, az);
}

int rotctl_send_el(observation_t *obs, double el)
{
	return rotctl_send(obs, false, el);
}

