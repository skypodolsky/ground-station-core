#pragma once

typedef enum log_levels_e {
  LVL_CRIT,
  LVL_ERR,
  LVL_INFO,
  LVL_VERB
} log_levels_e;

int log_save(void);
int log_init(FILE *fd, int level);
int log_print(int level, const char *fmt, ...);

#define LOG_C( ... ) log_print(LVL_CRIT, "[critical] " __VA_ARGS__)
#define LOG_E( ... ) log_print(LVL_ERR, "[error] " __VA_ARGS__)
#define LOG_I( ... ) log_print(LVL_INFO, "[info] " __VA_ARGS__)
#define LOG_V( ... ) log_print(LVL_VERB, "[debug] " __VA_ARGS__)

