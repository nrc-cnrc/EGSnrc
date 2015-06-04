/*
###############################################################################
#
#  EGSnrc file locking functions for windows
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


#include "egs_c_utils.h"

#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/locking.h>

static int __my_fd = -1;
static int __is_locked = 0;

void  egsCreateControlFile(const char *fname, int *status, int len) {
  if( __my_fd > 0 ) { _close(__my_fd); __my_fd = -1; }
  __my_fd = _open(fname,_O_CREAT | _O_EXCL | _O_RDWR, _S_IREAD | _S_IWRITE);
  if( __my_fd < 0 ) {
    *status = __my_fd;
    perror("egs_create_control_file: _open failed ");
  } else *status = 0;
}

#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_create_control_file_(const char *fname, int *status, int len) {
  egs_create_control_file(fname,status,len);
}
void C_CONVENTION egs_create_control_file__(const char *fname, int *status, int len) {
  egs_create_control_file(fname,status,len);
}
void C_CONVENTION EGS_CREATE_CONTROL_FILE(const char *fname, int *status, int len) {
  egs_create_control_file(fname,status,len);
}
void C_CONVENTION EGS_CREATE_CONTROL_FILE_(const char *fname, int *status, int len) {
  egs_create_control_file(fname,status,len);
}
void C_CONVENTION EGS_CREATE_CONTROL_FILE__(const char *fname, int *status, int len) {
  egs_create_control_file(fname,status,len);
}
#endif

void  egsOpenControlFile(const char *fname, int *status, int len) {
  int t; *status = 0;
  if( __my_fd > 0 ) { _close(__my_fd); __my_fd = -1; }
  for(t=0; t<15; t++) {
    __my_fd = _open(fname,_O_RDWR,_S_IREAD | _S_IWRITE);
    if( __my_fd > 0 ) break;
    _sleep(1000);
  }
  if( __my_fd < 0 ) {
    *status = __my_fd;
    perror("egs_open_control_file: _open failed ");
  } else *status = 0;
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_open_control_file_(const char *fname, int *status, int len) {
  egs_open_control_file(fname,status,len);
}
void C_CONVENTION egs_open_control_file__(const char *fname, int *status, int len) {
  egs_open_control_file(fname,status,len);
}
void C_CONVENTION EGS_OPEN_CONTROL_FILE(const char *fname, int *status, int len) {
  egs_open_control_file(fname,status,len);
}
void C_CONVENTION EGS_OPEN_CONTROL_FILE_(const char *fname, int *status, int len) {
  egs_open_control_file(fname,status,len);
}
void C_CONVENTION EGS_OPEN_CONTROL_FILE__(const char *fname, int *status, int len) {
  egs_open_control_file(fname,status,len);
}
#endif

void  egsCloseControlFile(int *status) {
  if( __my_fd > 0 ) *status = _close(__my_fd); else *status = 0;
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_close_control_file_(int *status) {
  egs_close_control_file(status);
}
void C_CONVENTION egs_close_control_file__(int *status) {
  egs_close_control_file(status);
}
void C_CONVENTION EGS_CLOSE_CONTROL_FILE(int *status) {
  egs_close_control_file(status);
}
void C_CONVENTION EGS_CLOSE_CONTROL_FILE_(int *status) {
  egs_close_control_file(status);
}
void C_CONVENTION EGS_CLOSE_CONTROL_FILE__(int *status) {
  egs_close_control_file(status);
}
#endif

void  egsLockControlFile(int *status) {
  long np;
  /*fprintf(stderr,"egs_lock_control_file: %d\n",__is_locked);*/
  if( __is_locked == 1 ) { *status = 0; return; }
  if( __my_fd < 0 ) { *status = -1; return; }
  np = _lseek(__my_fd,0L,SEEK_SET);
  if( np ) {
    fprintf(stderr,"egs_lock_control_file: lseek returned %d\n",np);
    *status = -1; perror("perror: ");
    return;
  }
  *status = _locking(__my_fd,_LK_LOCK,1000000L);
  if( *status = 0 ) __is_locked = 1;
  else {
    fprintf(stderr,"_locking returned %d\n",*status);
    perror("perror: ");
  }
}
#ifdef MAKE_WIN_DISTRIBUTION
void  C_CONVENTION egs_lock_control_file_(int *status) {
  egs_lock_control_file(status);
}
void C_CONVENTION egs_lock_control_file__(int *status) {
  egs_lock_control_file(status);
}
void C_CONVENTION EGS_LOCK_CONTROL_FILE(int *status) {
  egs_lock_control_file(status);
}
void C_CONVENTION EGS_LOCK_CONTROL_FILE_(int *status) {
  egs_lock_control_file(status);
}
void C_CONVENTION EGS_LOCK_CONTROL_FILE__(int *status) {
  egs_lock_control_file(status);
}
#endif

void  egsUnlockControlFile(int *status) {
  long np;
  if( __is_locked == 0 ) { *status = 0; return; }
  if( __my_fd < 0 ) { *status = -1; return; }
  np = _lseek(__my_fd,0L,SEEK_SET);
  if( np ) {
    *status = -1; perror("egs_unlock_control_file: failed to rewind file ");
    return;
  }
  *status = _locking(__my_fd,_LK_UNLCK,1000000L);
  if( *status == 0 ) __is_locked = 0;
  else {
    fprintf(stderr,"egs_unlock: _locking returned %d\n",*status);
    perror("perror: ");
  }
}
#ifdef MAKE_WIN_DISTRIBUTION
void  C_CONVENTION egs_unlock_control_file_(int *status) {
  egs_unlock_control_file(status);
}
void C_CONVENTION egs_unlock_control_file__(int *status) {
  egs_unlock_control_file(status);
}
void C_CONVENTION EGS_UNLOCK_CONTROL_FILE(int *status) {
  egs_unlock_control_file(status);
}
void C_CONVENTION EGS_UNLOCK_CONTROL_FILE_(int *status) {
  egs_unlock_control_file(status);
}
void C_CONVENTION EGS_UNLOCK_CONTROL_FILE__(int *status) {
  egs_unlock_control_file(status);
}
#endif

void egsRewindControlFile(int *status) {
  long np;
  fprintf(stderr,"egs_rewind_control_file: %d\n",__is_locked);
  if( __my_fd < 0 ) { *status = -1; return; }
  np = _lseek(__my_fd,0L,SEEK_SET);
  if( np ) {
    fprintf(stderr,"failed to rewind file: np = %d\n",np);
    *status = -1; perror("perror: ");
    return;
  }
  if( __is_locked == 0 ) {
    *status = _locking(__my_fd,_LK_LOCK,1000000L);
    if( *status == 0 ) __is_locked = 1;
    else perror("_locking failed: ");
  } else *status = 0;
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_rewind_control_file_(int *status) {
  egs_rewind_control_file(status);
}
void C_CONVENTION egs_rewind_control_file__(int *status) {
  egs_rewind_control_file(status);
}
void C_CONVENTION EGS_REWIND_CONTROL_FILE(int *status) {
  egs_rewind_control_file(status);
}
void C_CONVENTION EGS_REWIND_CONTROL_FILE_(int *status) {
  egs_rewind_control_file(status);
}
void C_CONVENTION EGS_REWIND_CONTROL_FILE__(int *status) {
  egs_rewind_control_file(status);
}
#endif

void egsWriteControlFile(const char *buf, const int *n, int *status, int len) {
  unsigned int count = *n;
  if( __my_fd < 0 ) { *status = 0; return; }
  *status = _write(__my_fd,buf,count);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_write_control_file_(const char *buf, const int *n, int *status, int len) {
  egs_write_control_file(buf,n,status,len);
}
void C_CONVENTION egs_write_control_file__(const char *buf, const int *n, int *status, int len) {
  egs_write_control_file(buf,n,status,len);
}
void C_CONVENTION EGS_WRITE_CONTROL_FILE(const char *buf, const int *n, int *status, int len) {
  egs_write_control_file(buf,n,status,len);
}
void C_CONVENTION EGS_WRITE_CONTROL_FILE_(const char *buf, const int *n, int *status, int len) {
  egs_write_control_file(buf,n,status,len);
}
void C_CONVENTION EGS_WRITE_CONTROL_FILE__(const char *buf, const int *n, int *status, int len) {
  egs_write_control_file(buf,n,status,len);
}
#endif

void egsReadControlFile(char *buf, const int *n, int *status, int len) {
  unsigned int count = *n;
  if( __my_fd < 0 ) { *status = 0; return; }
  *status = _read(__my_fd,buf,count);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_read_control_file_(char *buf, const int *n, int *status, int len) {
  egs_read_control_file(buf,n,status,len);
}
void C_CONVENTION egs_read_control_file__(char *buf, const int *n, int *status, int len) {
  egs_read_control_file(buf,n,status,len);
}
void C_CONVENTION EGS_READ_CONTROL_FILE(char *buf, const int *n, int *status, int len) {
  egs_read_control_file(buf,n,status,len);
}
void C_CONVENTION EGS_READ_CONTROL_FILE_(char *buf, const int *n, int *status, int len) {
  egs_read_control_file(buf,n,status,len);
}
void C_CONVENTION EGS_READ_CONTROL_FILE__(char *buf, const int *n, int *status, int len) {
  egs_read_control_file(buf,n,status,len);
}
#endif

void egsSleep(const int *secs) {
  unsigned int msecs = *secs * 1000; _sleep(msecs);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_sleep_(const int *secs){
     egs_sleep(secs);
}
void C_CONVENTION egs_sleep__(const int *secs){
     egs_sleep(secs);
}
void C_CONVENTION EGS_SLEEP(const int *secs){
     egs_sleep(secs);
}
void C_CONVENTION EGS_SLEEP_(const int *secs){
     egs_sleep(secs);
}
void C_CONVENTION EGS_SLEEP__(const int *secs){
     egs_sleep(secs);
}
#endif
void egsPerror(const char *str, int len) { perror(str); }
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_perror_(const char *str, int len) { perror(str); }
void C_CONVENTION egs_perror__(const char *str, int len) { perror(str); }
void C_CONVENTION EGS_PERROR(const char *str, int len) { perror(str); }
void C_CONVENTION EGS_PERROR_(const char *str, int len) { perror(str); }
void C_CONVENTION EGS_PERROR__(const char *str, int len) { perror(str); }
#endif

void egsRemoveFile(const char *fname, int *status, int len) {
  *status = _unlink(fname);
}
#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_remove_file_(const char *fname, int *status, int len) {
     egs_remove_file(fname,status,len);
}
void C_CONVENTION egs_remove_file__(const char *fname, int *status, int len) {
     egs_remove_file(fname,status,len);
}
void C_CONVENTION EGS_REMOVE_FILE(const char *fname, int *status, int len) {
     egs_remove_file(fname,status,len);
}
void C_CONVENTION EGS_REMOVE_FILE_(const char *fname, int *status, int len) {
     egs_remove_file(fname,status,len);
}
void C_CONVENTION EGS_REMOVE_FILE__(const char *fname, int *status, int len) {
     egs_remove_file(fname,status,len);
}
#endif
