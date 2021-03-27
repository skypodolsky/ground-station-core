#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "sdr.h"
#include "log.h"

int sdr_set_freq(double freq)
{
	int sockfd; 
	char buf[64] = { 0 };
	struct sockaddr_in servaddr; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		LOG_E("Failed to create socket");
		return -1;
	} 

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(2256); 

connect:
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) { 
		LOG_E("Failed to connect to SDR server");
		sleep(2);
		goto connect;
	}

	snprintf(buf, sizeof(buf), "%.0f", freq);

	send(sockfd, buf, strlen(buf), 0);
	close(sockfd);
	return 0;
}

int sdr_start(satellite_t *sat, const char *filename)
{
	if (!sat) {
		return -1;
	}

	observation_t *obs = sat->obs;
	obs->sdr_pid = fork();

	if (obs->sdr_pid == 0) {
		char *program_name = "rx_fm";
		char mod[32];
		char freq[32];

		snprintf(mod, sizeof(mod), "%s", "wbfm");
		snprintf(freq, sizeof(freq), "%d", sat->frequency);

		char *args[] = { program_name, "-M", mod, "-s", "190k", "-f", freq, "-p", "55", "-g", "LNA=40,VGA=40,AMP=14", "-E", "wav", "-E", "deemp", "-F", "9", "-d", "driver=hackrf", (char *) filename,  NULL };

		execvp(program_name, args);
	} else if (obs->sdr_pid == -1) {
		LOG_E("Error on fork()\n");
		return -1;
	}
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
