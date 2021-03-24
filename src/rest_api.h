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
