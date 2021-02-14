#include <stdio.h>
#include <stdarg.h>

#include "log.h"

static int log_level;
static FILE *log_file;

int log_init(FILE *fd, int level)
{
  log_file = fd;
  log_level = level;
  LOG_I("Log init.");
  return 0;
}

int log_print(int level, const char *fmt, ...)
{
  if (log_level >= level) {
    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    fprintf(log_file, "\n");
    va_end(args);
  }
  return 0;
}

int log_save(void)
{
  LOG_I("Saving log file.");
  if (log_file)
    fclose(log_file);

  return 0;
}
