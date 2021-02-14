#pragma once

#include "sat.h"

#define JSON_FINS_STR "fins"
#define JSON_NOSE_CONE_STR "nose cone"
#define JSON_CONICAL_SHOULDER_STR "conical shoulder"
#define JSON_CONICAL_BOATTAIL_STR "conical boattail"

#define JSON_NOSE_FORM_OGIVE_STR "ogive"
#define JSON_NOSE_FORM_CONICAL_STR "conical"
#define JSON_NOSE_FORM_PARABOLIC_STR "parabolic"

int json_parse(const char *json, observation_t *rocket, const char **error);
int json_prepare_reply(observation_t *rocket, const char *error, char **reply_buf);
