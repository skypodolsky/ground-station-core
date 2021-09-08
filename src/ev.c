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
#include <netinet/in.h>
#include <ev.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <json-c/json.h>

#include "ev.h"
#include "log.h"
#include "sat.h"
#include "rest_api.h"

char *reply = 
"HTTP/1.1 200 OK\n"
"Content-Type: application/json\n"
"Content-Length: %d\n"
"Accept-Ranges: bytes\n"
"Connection: close\n\n%s";

static struct event_handler ev_handler;

void _ev_read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	ssize_t read;
	char *reply_buf;
	const char *error;
	/* observation_t *observation; */

	error = NULL;
	reply_buf = NULL;

	if (EV_ERROR & revents) {
		LOG_E("got invalid event");
		return;
	}

	/** receive and append */
	read = recv(watcher->fd, ev_handler.buf + ev_handler.buf_idx, 
			EV_BUFFER_SIZE - ev_handler.buf_idx - 1, 0);

	ev_handler.buf_idx += read;

	if (read < 0) {
		LOG_E("recv error");
		return;
	}

	if (read == 0) {
		ev_handler.buf_idx = 0;
		ev_handler.cont_length = 0;
		ev_io_stop(ev_handler.loop, watcher);
		free(watcher);
		return;
	}

	/** null-terminate to avoid
	 * potential stack corruption
	 */
	ev_handler.buf[ev_handler.buf_len - 1] = 0;

	/** 'Content-Length' is _NOT_ guaranteed to come in the first chunk
	 * but if we have managed to find it, then we may know payload size
	 */
/* LOG_E("[%s]", ev_handler.buf); */
	if (ev_handler.cont_length == 0) {
		const char *cont_len_str = strstr(ev_handler.buf, "Content-Length: ");
		if (cont_len_str) {
			/** at this point we are absolutely sure that 'Content-Length'
			 * isn't corrupted, so we may safely convert it to integer
			 */
			if (sscanf(cont_len_str, "Content-Length: %d\r\n", &ev_handler.cont_length) != 1) {
				LOG_V("'Content-Length' not found");
				/** setting to zero explicitly */
				ev_handler.cont_length = 0;
				return;
			}
		}
		/* LOG_V("'Content-Length' is %d bytes", ev_handler.cont_length); */
	}

	char *payload = strstr(ev_handler.buf, "\r\n\r\n");
	if (payload) {
		payload += 4;
	} else {
		return;
	}

	/** at this point we are absolutely sure that we are already
	 * on the payload data. So let's get the number of rest bytes
	 */
	int bytes_of_payload = &ev_handler.buf[ev_handler.buf_idx] - payload;
	int bytes_left = ev_handler.cont_length - bytes_of_payload;

	if (bytes_left)
		LOG_V("Got fragment: bytes left = %d", bytes_left);

	if (bytes_left != 0)
		return;

	char *api_str = NULL;
	char *type_str = strtok(ev_handler.buf, " ");
	if (type_str != NULL) {
		api_str = strtok(NULL, " ");
		if (api_str == NULL)
			return; /** should never happen */
	}

	rest_api_type_t type = rest_api_get_type(type_str);
	rest_api_action_t action = rest_api_find_action(api_str, type);
	if (action == NULL) {
		error = "REST API not found";
		rest_api_prepare_error(error, &reply_buf);
		goto reply;
	}

	if (action(payload, &reply_buf, &error) == -1) {
		LOG_E("error during the request processing");
		rest_api_prepare_error(error, &reply_buf);
		goto reply;
	}

	/** send the reply */
	if (type == REST_API_TYPE_POST) {
		action = rest_api_find_action(api_str, REST_API_TYPE_GET);
		if (action && (action(payload, &reply_buf, &error) == -1)) {
			LOG_E("error while creating a reply on POST request");
			goto reply;
		}
	}

reply:
	if (reply_buf) {
		snprintf(ev_handler.buf, ev_handler.buf_len, reply, strlen(reply_buf), reply_buf);

		send(watcher->fd, ev_handler.buf, strlen(ev_handler.buf), 0);
		bzero(ev_handler.buf, ev_handler.buf_idx);

		free(reply_buf);
		reply_buf = NULL;
	}
}

void _ev_accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct ev_io *w_client = (struct ev_io*) malloc(sizeof(struct ev_io));

	if (EV_ERROR & revents) {
		LOG_I("got invalid event");
		return;
	}

	ev_handler.client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);
	if (ev_handler.client_sd < 0) {
		LOG_E("accept error");
		return;
	}
	/* fcntl(ev_handler.client_sd, F_SETFL, O_NONBLOCK); */

	ev_io_init(w_client, _ev_read_cb, ev_handler.client_sd, EV_READ);
	ev_io_start(ev_handler.loop, w_client);
}

int ev_probe(int port)
{
	int reuse;
	struct sockaddr_in addr;

	if ((ev_handler.main_sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		LOG_C("socket error");
		return -1;
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	reuse = 1;
	if (setsockopt(ev_handler.main_sd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		LOG_C("setsockopt failed");
		exit(1);
	}

	if (bind(ev_handler.main_sd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
		LOG_C("bind error");
		return -1;
	}

	if (listen(ev_handler.main_sd, 2) < 0) {
		LOG_C("listen error");
		return -1;
	}

	LOG_I("Listening port %d", port);

	return 0;
}

int ev_process_loop(void)
{
	struct ev_io w_accept;

	ev_handler.buf_idx = 0;
	ev_handler.buf_len = EV_BUFFER_SIZE;

	ev_handler.buf = calloc(1, ev_handler.buf_len);
	if (!ev_handler.buf) {
		LOG_C("couldn't allocate message buffer");
		return -1;
	}

	ev_handler.loop = ev_default_loop(0);
	ev_io_init(&w_accept, _ev_accept_cb, ev_handler.main_sd, EV_READ);
	ev_io_start(ev_handler.loop, &w_accept);

	while (true) {
		ev_loop(ev_handler.loop, 0);
	}

	return 0;
}

int ev_release(void)
{
	if (ev_handler.buf)
		free(ev_handler.buf);

	return 0;
}
