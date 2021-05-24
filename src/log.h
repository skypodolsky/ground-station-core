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

