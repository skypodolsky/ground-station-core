#pragma once

#include <stdbool.h>

#define EV_BUFFER_SIZE   8192

struct event_handler {
  int main_sd;
  int client_sd;
  char *buf;
  int buf_len;
  int buf_idx;
  int cont_length;
  struct ev_loop *loop;
};

int ev_probe(int port);
int ev_release(void);
int ev_process_loop(void);
