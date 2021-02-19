#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#include "ev.h"
#include "sat.h"
#include "log.h"

void handle_sigint(int sig)
{
  log_save();
  exit(0);
}

/** Used for simulation to set needed time */
void handle_sigusr1(int sig)
{
	time_t current_time;
	struct tm timeval = {0};
	timeval.tm_year = 2021-1900;
	timeval.tm_mon = 1;
	timeval.tm_mday = 21;
	timeval.tm_hour = 6;
	timeval.tm_min = 50;

	time(&current_time);
	sat_simul_time_set(mktime(&timeval) - current_time);
}

/** Used for simulation to increment time by time step */
void handle_sigusr2(int sig)
{
	sat_simul_time_step(30);
}

int main(int argc, char **argv)
{
  int opt;
  int port;
  int log_level;
  FILE *log_file;

  port = 8080;
  log_file = stdout;
  log_level = LVL_CRIT;

  while((opt = getopt(argc, argv, "v:p:f:")) != -1) {
    switch(opt) {
      case 'p':
        port = strtol(optarg, NULL, 10);
        break;
      case 'v':
        log_level = strtol(optarg, NULL, 10);
        break;
      case 'f':
        log_file = fopen(optarg, "a");
        if (!log_file) {
          perror("Couldn't open log file");
          return -1;
        }
        break;
    }
  }

  log_init(log_file, log_level);
  signal(SIGINT, handle_sigint);
  signal(SIGUSR1, handle_sigusr1);
  signal(SIGUSR2, handle_sigusr2);

  if (ev_probe(port) == -1)
    return -1;
  
  ev_process_loop();
  return 0;
}
