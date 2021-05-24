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
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <json-c/json.h>

#include "log.h"
#include "json.h"
#include "helpers.h"

const char *json_get_string_by_key(json_object *parent, const char *key)
{
  json_object *obj;

  if (!parent || !key) {
    return NULL;
  }

  if (!json_object_object_get_ex(parent, key, &obj)) {
    return NULL;
  }

  if (json_object_get_type(obj) != json_type_string)
    return NULL;

  return json_object_get_string(obj);
}

bool json_get_int_by_key(json_object *parent, const char *key, int *ret)
{
  json_object *obj;

  if (!parent || !key || !ret) {
    return false;
  }

  if (!json_object_object_get_ex(parent, key, &obj)) {
    return false;
  }

  if (json_object_get_type(obj) != json_type_int)
    return false;

  *ret = json_object_get_int(obj);
  return true;
}

bool json_get_double_by_key(json_object *parent, const char *key, double *ret)
{
  json_object *obj;

  if (!parent || !key || !ret) {
    return false;
  }

  if (!json_object_object_get_ex(parent, key, &obj)) {
    return false;
  }

  if (json_object_get_type(obj) != json_type_double)
    return false;

  *ret = json_object_get_double(obj);
  return true;
}

