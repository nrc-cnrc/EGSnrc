/*
###############################################################################
#
#  EGSnrc file locking functions for unix
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
*/


/* Build usage:
 * HEN_HOUSE/scripts/configure copies this file to the machine build directory
 * as egs_c_utils.c. HEN_HOUSE/gui/egs_configure and HEN_HOUSE/cutils/Makefile
 * use the separate HEN_HOUSE/cutils/egs_c_utils.c implementation.
 */
/* Select with -DEGS_LOCK_WAIT_BLOCKING or -DEGS_LOCK_WAIT_UNBLOCKING; blocking is the default. */
#if !defined(EGS_LOCK_WAIT_BLOCKING) && !defined(EGS_LOCK_WAIT_UNBLOCKING)
#define EGS_LOCK_WAIT_BLOCKING
#endif

#ifdef EGS_LOCK_WAIT_UNBLOCKING
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include "egs_c_utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#ifdef EGS_LOCK_WAIT_UNBLOCKING
#include <time.h>
#endif

static int __my_fd = -1;
static int __is_locked = 0;
static int __is_initialized = 0;
struct flock fl_write, fl_unlock;

#ifdef EGS_LOCK_WAIT_UNBLOCKING
#define EGS_LOCK_WAIT_MAX_MS 50
#define EGS_LOCK_WAIT_TIMEOUT_MS 60000

static unsigned int lock_wait_random(unsigned int upper_bound) {
  static pid_t seed_pid = (pid_t)-1;
  static unsigned int state;
  pid_t current_pid = getpid();

  if (seed_pid != current_pid) {
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == 0) {
      state = (unsigned int)now.tv_sec ^ (unsigned int)now.tv_nsec ^
              (unsigned int)current_pid;
    }
    else {
      state = (unsigned int)time(NULL) ^ (unsigned int)current_pid;
    }
    if (state == 0) state = 1;
    seed_pid = current_pid;
  }

  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state % upper_bound;
}

#endif

/*! Init the lock/unlock structures */
void __init_locking() {
#ifdef DEBUG
  fprintf( stderr,"__init_locking()\n");
#endif
  fl_write.l_type = F_WRLCK;
  fl_write.l_whence = SEEK_SET;
  fl_write.l_start = 0;
  fl_write.l_len = 0;
  fl_unlock.l_type = F_UNLCK;
  fl_unlock.l_whence = SEEK_SET;
  fl_unlock.l_start = 0;
  fl_unlock.l_len = 0;
  __is_initialized = 1;
}

void egsCreateControlFile(const char *fname, int *status, int len) {
#ifdef DEBUG
  fprintf( stderr,"create_control_file: file name = %s\n",fname);
#endif
  if( !__is_initialized ) __init_locking();
  *status = 0;
  if( __my_fd > 0 ) { close(__my_fd); __my_fd = -1; }
  __my_fd = open(fname,O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if( __my_fd < 0 ) {
#ifdef DEBUG
    fprintf( stderr,"create_control_file: failed to open file %d\n",errno);
    perror("error is");
#endif
    *status = errno; return;
  }
  egsLockControlFile(status);
}

void egsOpenControlFile(const char *fname, int *status, int len) {
  int t; *status = 0;
  if( !__is_initialized ) __init_locking();
  if( __my_fd > 0 ) { close(__my_fd); __my_fd = -1; }
  /* This function is called from jobs > 1. Job 1 creates the control file.
     But if for some reasons it took job 1 longer than subsequent job,
     the control file may not be there yet => we try several times and
     sleep in between.
  */
  for(t=0; t<15; t++) {
    __my_fd = open(fname,O_RDWR); if( __my_fd > 0 ) break;
    sleep(1);
  }
  if( __my_fd < 0 ) *status = errno;
}

void egsCloseControlFile(int *status) {
  if( __my_fd > 0 ) *status = close(__my_fd); else *status = 0;
}

void egsLockControlFile(int *status) {
#ifdef DEBUG
  fprintf(stderr,"lock_control_file: fd = %d is_locked = %d\n",__my_fd,
       __is_locked);
#endif
  if( __is_locked == 1 ) { *status = 0; return; }
  if( __my_fd < 0 ) { *status = -1; return; }
#ifdef EGS_LOCK_WAIT_UNBLOCKING
  unsigned int backoff_ms = 1;
  struct timespec wait_start, now, delay;
  if (clock_gettime(CLOCK_MONOTONIC, &wait_start) == -1) {
    *status = errno;
    return;
  }
  while (1) {
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
      *status = errno;
      return;
    }
    long long elapsed_ns = (long long)(now.tv_sec - wait_start.tv_sec) * 1000000000LL +
                           (now.tv_nsec - wait_start.tv_nsec);
    long long remaining_ns = (long long)EGS_LOCK_WAIT_TIMEOUT_MS * 1000000LL - elapsed_ns;
    if (remaining_ns <= 0) break;

    *status = fcntl(__my_fd,F_SETLK,&fl_write);
    if (*status == 0) { __is_locked = 1; return; }

    unsigned int delay_ms = 1 + lock_wait_random(backoff_ms);
    long long delay_ns = (long long)delay_ms * 1000000LL;
    if (delay_ns > remaining_ns) delay_ns = remaining_ns;
    delay.tv_sec = delay_ns / 1000000000LL;
    delay.tv_nsec = delay_ns % 1000000000LL;
    if (nanosleep(&delay, NULL) == -1 && errno != EINTR) {
      *status = errno;
      return;
    }
    if (backoff_ms < EGS_LOCK_WAIT_MAX_MS) {
      backoff_ms *= 2;
      if (backoff_ms > EGS_LOCK_WAIT_MAX_MS) {
        backoff_ms = EGS_LOCK_WAIT_MAX_MS;
      }
    }
  }
#else
  do {
    *status = fcntl(__my_fd,F_SETLKW,&fl_write);
  } while( *status != 0 && errno == EINTR );
  if( *status == 0 ) { __is_locked = 1; return; }
#endif
#ifdef DEBUG
  if ( *status != 0 ) perror("error was");
#endif
#ifdef EGS_LOCK_WAIT_UNBLOCKING
  printf("egsLockControlFile: failed to lock file after 60 second wait!\n");
#else
  printf("egsLockControlFile: blocking lock failed!\n");
#endif
}

void egsUnlockControlFile(int *status) {
#ifdef DEBUG
  fprintf(stderr,"unlock_control_file: fd = %d is_locked = %d\n",__my_fd,
       __is_locked);
#endif
  if( __is_locked == 0 ) { *status = 0; return; }
  if( __my_fd < 0 ) { *status = -1; return; }
  *status = fcntl(__my_fd,F_SETLKW,&fl_unlock);
#ifdef DEBUG
  if ( *status != 0 ) perror("error was");
#endif
  if( *status == 0 ) __is_locked = 0;
}

void egsRewindControlFile(int *status) {
  if( __my_fd < 0 ) { *status = -1; return; }
  *status=0;
  if( __is_locked == 0 ) egsLockControlFile(status);
  if( *status != 0 ) return;
  *status = lseek(__my_fd,0,SEEK_SET);
}

void egsWriteControlFile(const char *buf, const int *n, int *status, int len) {
  if( __my_fd < 0 ) { *status = 0; return; }
  *status = write(__my_fd,buf,*n);
}

void egsReadControlFile(char *buf, const int *n, int *status, int len) {
  if( __my_fd < 0 ) { *status = 0; return; }
  *status = read(__my_fd,buf,*n);
}

void egsSleep(const int *secs) { sleep(*secs); }

void egsFtoString(const int *size, int *n, char *str,void *a, int len) {
  size_t x = *n;
  if( *size == 4 ) {
    float *tmp = (float *) a;
    /* Well, it seems snprintf is not supported by all Unixes (e.g. DEC).
       I'm too lazy to make yet another test during the installation =>
       just use sprintf instead.
    if( x > 0 ) *n = snprintf(str,x,"%g",*tmp);
    else *n = sprintf(str,"%g",*tmp); */
    *n = sprintf(str,"%g",*tmp);
  }
  else if( *size == 8 ) {
    double *tmp = (double *) a;
    /*
    if( x > 0 ) *n = snprintf(str,x,"%lg",*tmp);
    else *n = sprintf(str,"%lg",*tmp);
    */
    *n = sprintf(str,"%lg",*tmp);
  }
  else *n = 0;
}

void egsPerror(const char *str, int len) { perror(str); }

void egsRemoveFile(const char *fname, int *status, int len) {
   *status = unlink(fname);
}
