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

typedef enum rest_api_type_t {
	REST_API_TYPE_GET,
	REST_API_TYPE_POST,
	REST_API_TYPE_NOT_SUPP
} rest_api_type_t;

/** reply_buf: for GET requests only, not used with POST requests
 *  error: for POST requests only, not used with GET requests*/
typedef int (*rest_api_action_t)(char *payload, char **reply_buf, const char **error);

typedef struct rest_api_t {
	char *name;
	rest_api_type_t type;
	rest_api_action_t action;
} rest_api_t;

rest_api_type_t rest_api_get_type(const char *type_str);
rest_api_action_t rest_api_find_action(const char *api, rest_api_type_t type);
int rest_api_prepare_error(const char *error, char **reply_buf);
