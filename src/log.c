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

#include <time.h>
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
	time_t current_time;
	struct tm timeval;

	current_time = time(NULL);
	timeval = *localtime(&current_time);

  if (log_level >= level) {
    va_list args;
    va_start(args, fmt);
	fprintf(log_file, "[%02d:%02d:%02d]", timeval.tm_hour, timeval.tm_min, timeval.tm_sec);
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
