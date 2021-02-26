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

#define LOG_C( ... ) log_print(LVL_CRIT, "\x1b[1;31m[critical] \x1b[1;0m" __VA_ARGS__)
#define LOG_E( ... ) log_print(LVL_ERR, "\x1b[1;33m[error] \x1b[1;0m" __VA_ARGS__)
#define LOG_I( ... ) log_print(LVL_INFO, "\x1b[1;32m[info] \x1b[1;0m" __VA_ARGS__)
#define LOG_V( ... ) log_print(LVL_VERB, "\x1b[1;0m[debug] \x1b[1;0m" __VA_ARGS__)

