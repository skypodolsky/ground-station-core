#pragma once

#include "sat.h"

const char *json_get_string_by_key(json_object *parent, const char *key);
bool json_get_int_by_key(json_object *parent, const char *key, int *ret);
bool json_get_double_by_key(json_object *parent, const char *key, double *ret);
