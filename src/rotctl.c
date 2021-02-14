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

int rotctl_open(observation_t *obs)
{
	int ret = 0;
	struct sockaddr_in server_addr = { 0 };

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(obs->cli.port);
	if ((ret = inet_pton(AF_INET, obs->cli.addr, &server_addr.sin_addr)) != 1) {
		LOG_E("Error on inet_pton");
		return -1;
	}

	if ((obs->cli.conn_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOG_E("Error on socket");
		return -1;
	}

	if ((ret = connect(obs->cli.conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1) {
		LOG_E("Error on connect");
		return -1;
	}

	return ret;
}

int rotctl_close(observation_t *obs)
{
	int ret;

	if (obs == NULL)
		return -1;

	if ((ret = shutdown(obs->cli.conn_fd, SHUT_RDWR)) == -1) {
		LOG_E("Error on shutdown");
		return -1;
	}

	if ((ret = close(obs->cli.conn_fd)) == -1) {
		LOG_E("Error on shutdown");
		return -1;
	}

	return 0;
}

int rotctl_send(observation_t *obs, double az, double el)
{
	char buf[32];

	if (obs == NULL)
		return -1;

	snprintf(buf, sizeof(buf), "w A%f", az);
	write(obs->cli.conn_fd, buf, strlen(buf)); 

	snprintf(buf, sizeof(buf), "w E%f", el);
	write(obs->cli.conn_fd, buf, strlen(buf)); 

	return 0;
}
