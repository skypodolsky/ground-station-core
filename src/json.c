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

int json_parse(const char *json, observation_t *observation, const char **error)
{
  int ret;
  struct json_object *jObj;
  struct json_object *observationObj;

  ret = 0;

  LOG_V("Parsing JSON request...");
  jObj = json_tokener_parse(json);
  if (!jObj) {
    *error = "Couldn't parse JSON request";
    return -1;
  }

  if (!json_object_object_get_ex(jObj, "observation", &observationObj)) {
    *error = "'/observation' object is missing";
    ret = -1;
    goto out;
  }

  json_object *satellite;
  json_object_object_get_ex(observationObj, "satellite", &satellite);

  if (satellite) {
    int i;

    for (i = 0; i < json_object_array_length(satellite); i++) {
      json_object *jsatellitePart;

      satellite_t *sat = calloc(1, sizeof(satellite_t));
      if (!satellite) {
        *error = "Out of memory";
        ret = -1;
        goto out;
      }

	  sat->obs = observation;
      LIST_INSERT_HEAD(&observation->satellites_list, sat, entries);

      jsatellitePart = json_object_array_get_idx(satellite, i);
      if (!jsatellitePart) {
        *error = "'/observation/satellite': couldn't get CG array index";
        ret = -1;
        goto out;
      }

      const char *satName = json_get_string_by_key(jsatellitePart, "name");
      if (!satName) {
        *error = "'/observation/satellite/name' string is not specified!";
        ret = -1;
        goto out;
      }

	  LOG_V("Satellite name: [ %s ]", satName);

      const char *satModulation = json_get_string_by_key(jsatellitePart, "modulation");
      if (!satModulation) {
        *error = "'/observation/satellite/modulation' string is not specified!";
        ret = -1;
		goto out;
	  }

	  if (streq(satModulation, "fm")) {
		  sat->modulation = MODULATION_FM;
	  } else if (streq(satModulation, "afsk")) {
		  sat->modulation = MODULATION_AFSK;
	  } else {
		  *error = "'/observation/satellite/modulation' only 'fm' and 'afsk' are supported";
		  ret = -1;
		  goto out;
	  }

	  LOG_V("Satellite modulation: [ %s ]", satModulation);

      strncpy(sat->name, satName, sizeof(sat->name));

      if (!json_get_int_by_key(jsatellitePart, "frequency", &sat->frequency)) {
        *error = "'/observation/satellite/frequency' not specified";
        ret = -1;
        goto out;
      }

	  LOG_V("Satellite frequency: [ %d ]", sat->frequency);

      if (!json_get_double_by_key(jsatellitePart, "min_elevation", &sat->min_elevation)) {
        *error = "'/observation/satellite/min_elevation' not specified";
        ret = -1;
        goto out;
      }

	  LOG_V("Satellite min. elevation: [ %f ]", sat->min_elevation);


      if (!json_get_int_by_key(jsatellitePart, "bandwidth", &sat->bandwidth)) {
        *error = "'/observation/satellite/bandwidth' not specified";
        ret = -1;
        goto out;
      }

	  LOG_V("Satellite bandwidth: [ %d ]", sat->bandwidth);

      if (!json_get_int_by_key(jsatellitePart, "priority", &sat->priority)) {
        *error = "'/observation/satellite/priority' not specified";
        ret = -1;
        goto out;
      }

	  LOG_V("Satellite priority: [ %d ]", sat->priority);

	  if (sat_setup(sat) == -1) {
		  *error = "Satellite not found";
		  goto out;
	  }

    }
  } else {
	  *error = "'/observation/satellite' object is missing";
	  ret = -1;
	  goto out;
  }

out:
  json_object_put(jObj);
  return ret;
}

int json_prepare_reply(observation_t *observation, const char *error, char **buf)
{
  int ret = 0;

  json_object *replyObj = json_object_new_object();
  if (!replyObj)
    return -1;

  if (error == NULL) {
      json_object *cgObj = json_object_new_string("OK");
      if (!cgObj) {
        ret = -1;
        goto out;
      }

      json_object_object_add(replyObj, "status", cgObj);
   } else {
    json_object *errorObj = json_object_new_string(error);
    if (!errorObj) {
      ret = -1;
      goto out;
    }

    json_object_object_add(replyObj, "error", errorObj);
  }

  *buf = strdup(json_object_to_json_string(replyObj));
  if (!buf)
    return -1;

out:
  json_object_put(replyObj);

  return ret;
}
