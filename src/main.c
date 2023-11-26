//
// Created by Александр Ковель on 26.11.2023.
//

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "server/server.h"
#include "logger/logger.h"

static const size_t num_threads = 10;
static const size_t num_items   = 100;

void worker(void *arg)
{
  int *val = arg;
  int  old = *val;

  *val += 1000;
  LOG_INFO("tid=%p, old=%d, val=%d\n", pthread_self(), old, *val);

  if (*val%2)
	usleep(100000);
}

int main(int argc, char **argv)
{
  tpool_t *tm;
  int     *vals;
  size_t   i;

  tm   = tpool_create(num_threads);
  vals = calloc(num_items, sizeof(*vals));

  for (i=0; i<num_items; i++) {
	vals[i] = i;
	tpool_add_work(tm, worker, vals+i);
  }

  tpool_wait(tm);

  for (i=0; i<num_items; i++) {
	LOG_TRACE("%d\n", vals[i]);
  }

  free(vals);
  tpool_destroy(tm);
  return 0;
}
