/*
###############################################################################
#
#  EGSnrc C utility functions
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
#  Author:          Iwan Kawrakow, 2004
#  
#  Contributors:   
#
###############################################################################
#
#  Various C functions needed for the implementation of parallel processing 
#  in EGSnrc.
#
###############################################################################
*/


#ifndef EGS_C_UTILS_H_
#define EGS_C_UTILS_H_

#include "egs_config1.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The following set of functions are declared as void instead of returning 
 * an integer status because it appears that there are problems in correctly 
 * interpreting a return value from a C function in Fortran on 64 bit 
 * systems. In all cases, the status of the operation is returned in the 
 * integer pointed to by status (and so, for read/write operations 
 * is the number of bytes read/written, for open/close,lock, etc. 
 * it is 0 on success and errno of something goes wrong.
 */

/*! Create, open and lock the job control file. 
    This function is to be called from job number 1.
    fname is a pointer to a null-terminated string with the control file 
    name. 
 */
#define egsCreateControlFile F77_OBJ_(egs_create_control_file,EGS_CREATE_CONTROL_FILE)
void egsCreateControlFile(const char *fname, int *status, int len);

/*! Open the job control file.
    This function is to be called from all other jobs
 */
#define egsOpenControlFile F77_OBJ_(egs_open_control_file,EGS_OPEN_CONTROL_FILE)
void egsOpenControlFile(const char *fname, int *status, int len);

/*! Close the job control file.
 */
#define egsCloseControlFile F77_OBJ_(egs_close_control_file,EGS_CLOSE_CONTROL_FILE)
void egsCloseControlFile(int *status);

/*! Lock the control file.
    This function is to be called before performing I/O 
    operations on the job control file.
 */
#define egsLockControlFile F77_OBJ_(egs_lock_control_file,EGS_LOCK_CONTROL_FILE)
void egsLockControlFile(int *status);

/*! Unlock the control file.
    This function is to be called after I/O operations 
    on the job control file are done.
 */
#define egsUnlockControlFile F77_OBJ_(egs_unlock_control_file,EGS_UNLOCK_CONTROL_FILE)
void egsUnlockControlFile(int *status);

/*! Rewind the job control file 
    If the file is not locked, lock it first
 */
#define egsRewindControlFile F77_OBJ_(egs_rewind_control_file,EGS_REWIND_CONTROL_FILE)
void egsRewindControlFile(int *status);

/*! Write to the job control file.
    File should be locked prior to using this function.
    buf points to a char buffer to be written to the file, n to an integer
    with the length of the buffer.
 */
#define egsWriteControlFile F77_OBJ_(egs_write_control_file,EGS_WRITE_CONTROL_FILE)
void egsWriteControlFile(const char *buf, const int *n, 
                                            int *status, int len);
/*! Read from the job control file *n bytes into buf.
 */
#define egsReadControlFile F77_OBJ_(egs_read_control_file,EGS_READ_CONTROL_FILE)
void egsReadControlFile(char *buf, const int *n, int *status, int len);

/*! Remove a file */
#define egsRemoveFile F77_OBJ_(egs_remove_file,EGS_REMOVE_FILE)
void egsRemoveFile(const char *fname, int *status, int len);

/*! Sleep for *secs seconds */
#define egsSleep F77_OBJ_(egs_sleep,EGS_SLEEP)
void egsSleep(const int *secs);

/*! Print last error to stderr (just calls perror) */
#define egsPerror F77_OBJ_(egs_perror,EGS_PERROR)
void egsPerror(const char *msg, int len);

/**************************** other stuff **********************************/

/*! Convert float to ASCII string */
#define egsFtoString F77_OBJ_(egs_ftostring,EGS_FTOSTRING)
void egsFtoString(const int *size, int *n, char *str,void *a, int len);

#ifdef __cplusplus
}
#endif

#endif
