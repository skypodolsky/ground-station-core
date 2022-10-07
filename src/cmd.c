#include "cmd.h"

int request_cmd(const char *program_name, char *args[])
{
	int pid;
	
	pid = fork();

	if (pid == 0) {
		execvp(program_name, args);
		exit(0);
	} else if (pid == -1) {
		LOG_E("Error on fork()");
		return -1;
	}

	return pid;
}

int pre_doit(observation_t *obs)
{
	char freq[32] = { 0 };

	if (obs->active)
		snprintf(freq, sizeof(freq), "%llu", (long long)obs->active->frequency);

	char *args[] = { DEF_CONF_PYTHON, (char *) obs->cfg->cmd_script, "pre_doit", freq, NULL };
	return request_cmd(DEF_CONF_PYTHON, args);
}

int post_doit(observation_t *obs)
{
	char *args[] = { DEF_CONF_PYTHON, (char *) obs->cfg->cmd_script, "post_doit", NULL };
	return request_cmd(DEF_CONF_PYTHON, args);
}
