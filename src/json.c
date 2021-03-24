#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <json-c/json.h>

#include "log.h"
#include "json.h"
#include "entry.h"
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

