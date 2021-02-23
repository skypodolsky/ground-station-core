#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

bool verify_ip(const char *ip)
{
	struct sockaddr_in addr_in;

	if (inet_pton(AF_INET, ip, &addr_in.sin_addr) != 1) {
		LOG_E("Error on inet_pton");
		return false;
	}
	return true;
}

cfg_t *cfg_global_get(void)
{
	return _cfg;
}
