#pragma once

#include <stdbool.h>

#define DEF_LISTEN_PORT		25565

typedef struct netcli_t {
	int azimuth_port;		/** port of the azimuth rotctld  	*/
	int elevation_port;		/** port of the elevation rotctld 	*/
	int azimuth_conn_fd;	/** azimuth rotctld socket  		*/
	int elevation_conn_fd;	/** elevation rotctld socket 		*/
	char remote_ip[32];		/** remote IP of rotctld daemons 	*/
} netcli_t;

typedef struct cfg_t {
	int listen_port;		/** port to listen JSON requests 	*/
	char version[64];		/** utility version 				*/
	int log_level;			/** log level 						*/
	FILE *log_file;			/** file to log output in 			*/
	netcli_t cli;			/** data for the remote connection	*/
} cfg_t;

cfg_t *alloc_cfg(void);
void destroy_cfg(cfg_t *cfg);
bool verify_ip(const char *ip);
cfg_t *cfg_global_get(void);
