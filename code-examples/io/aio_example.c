#define _GNU_SOURCE

#include <assert.h>
#include <fcntl.h>
#include <libaio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_EVENTS 10
#define PAGE_SIZE 4096

int main() {
  int r;
  int fd;
  int i;
  io_context_t ioctx;
  struct iocb cb;
  struct iocb *p_cb = &cb;
  void *buffer;
  struct io_event events[MAX_EVENTS];

  memset(&cb, 0, sizeof(cb));
  memset(&ioctx, 0, sizeof(ioctx));
  memset(&buffer, 0, sizeof(buffer));
  r = posix_memalign(&buffer, PAGE_SIZE, PAGE_SIZE);
  assert(r == 0);

  fd = open("data.txt", O_RDONLY | O_DIRECT);
  assert(fd > 0);

  r = io_setup(MAX_EVENTS, &ioctx);
  assert(r == 0);

  io_prep_pread(&cb, fd, buffer, 100, 0);

  r = io_submit(ioctx, 1, &p_cb);
  assert(r == 1);

  r = io_getevents(ioctx, 0, MAX_EVENTS, events, NULL);

  printf("got %d events\n", r);

  for (i = 0; i < r; i++) {
    printf("result is %d\n", events[i].res);
    printf("sec result is %d\n", events[i].res2);
  }

  printf("buffer is %s\n", buffer);

  r = io_destroy(ioctx);
  assert(r == 0);

  close(fd);
}