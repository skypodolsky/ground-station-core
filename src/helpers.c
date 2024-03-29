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

#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "log.h"
#include "helpers.h"

double rad_to_deg(double rad)
{
	double deg = rad * 180.0 / M_PI;
	return deg;
}

inline bool streq(const char *str1, const char *str2)
{
  return !strcmp(str1, str2);
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


