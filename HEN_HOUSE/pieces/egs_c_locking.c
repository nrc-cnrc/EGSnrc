/*
 * EGSnrc control-file locking.
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright (C) 2015 National Research Council Canada.
 * See LICENCE for the full license text.
 *
 * Authors and contributors: see Git history.
 */

#if !defined(WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "egs_c_utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef EGS_LOCK_WAIT_UNBLOCKING
#include <time.h>
#endif
#endif

static int __my_fd = -1;
static int __is_locked = 0;

#ifdef WIN32

#define EGS_CONTROL_FILE_LOCK_LENGTH 1000000

static volatile LONG __control_file_temp_sequence = 0;
static char *__pending_control_temp = NULL;
static char *__pending_control_final = NULL;

static int win32_error_to_errno(DWORD error) {
    switch (error) {
        case ERROR_FILE_EXISTS:
        case ERROR_ALREADY_EXISTS:
            return EEXIST;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return ENOENT;
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:
            return EACCES;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
            return ENOMEM;
        case ERROR_FILENAME_EXCED_RANGE:
            return ENAMETOOLONG;
        default:
            return EIO;
    }
}

static void discard_pending_control_file(void) {
    int saved_errno = errno;
    if (__pending_control_temp) {
        DeleteFileA(__pending_control_temp);
    }
    free(__pending_control_temp);
    free(__pending_control_final);
    __pending_control_temp = NULL;
    __pending_control_final = NULL;
    errno = saved_errno;
}

static int create_control_file_temp(const char *fname) {
    size_t fname_len = strlen(fname);
    size_t temp_capacity;
    char *temp_name;
    char *final_name;
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    int lock_status;
    int attempt;

    if (fname_len > (size_t)-1 - 64) {
        errno = ENAMETOOLONG;
        return -1;
    }
    temp_capacity = fname_len + 64;
    temp_name = (char *)malloc(temp_capacity);
    final_name = (char *)malloc(fname_len + 1);
    if (!temp_name || !final_name) {
        free(temp_name);
        free(final_name);
        errno = ENOMEM;
        return -1;
    }
    memcpy(final_name, fname, fname_len + 1);

    for (attempt = 0; attempt < 128; ++attempt) {
        LONG sequence = InterlockedIncrement(&__control_file_temp_sequence);
        int path_len = snprintf(temp_name, temp_capacity, "%s.tmp.%lu.%ld",
                                fname, (unsigned long)GetCurrentProcessId(),
                                (long)sequence);
        if (path_len < 0 || (size_t)path_len >= temp_capacity) {
            errno = ENAMETOOLONG;
            goto fail;
        }
        file_handle = CreateFileA(temp_name, GENERIC_READ | GENERIC_WRITE | DELETE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                  NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file_handle != INVALID_HANDLE_VALUE) {
            break;
        }
        DWORD error = GetLastError();
        if (error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS) {
            errno = win32_error_to_errno(error);
            goto fail;
        }
    }
    if (file_handle == INVALID_HANDLE_VALUE) {
        errno = EEXIST;
        goto fail;
    }

    __my_fd = _open_osfhandle((intptr_t)file_handle, _O_RDWR);
    if (__my_fd < 0) {
        int saved_errno = errno;
        CloseHandle(file_handle);
        DeleteFileA(temp_name);
        errno = saved_errno;
        goto fail;
    }
    egsLockControlFile(&lock_status);
    if (lock_status != 0) {
        int lock_errno = lock_status > 0 ? lock_status : errno;
        _close(__my_fd);
        __my_fd = -1;
        DeleteFileA(temp_name);
        __is_locked = 0;
        errno = lock_errno;
        goto fail;
    }

    __pending_control_temp = temp_name;
    __pending_control_final = final_name;
    return 0;

fail:
    {
        int saved_errno = errno;
        free(temp_name);
        free(final_name);
        errno = saved_errno;
    }
    return -1;
}

static int publish_control_file_temp(void) {
    if (!__pending_control_temp) {
        return 0;
    }
    if (!MoveFileExA(__pending_control_temp, __pending_control_final, 0)) {
        return win32_error_to_errno(GetLastError());
    }
    free(__pending_control_temp);
    free(__pending_control_final);
    __pending_control_temp = NULL;
    __pending_control_final = NULL;
    return 0;
}

static int lock_control_file(int unlock) {
    HANDLE file_handle = (HANDLE)_get_osfhandle(__my_fd);
    OVERLAPPED overlapped = {0};
    BOOL result;

    if (file_handle == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return errno;
    }
    if (unlock) {
        result = UnlockFileEx(file_handle, 0, EGS_CONTROL_FILE_LOCK_LENGTH, 0,
                              &overlapped);
    }
    else {
        result = LockFileEx(file_handle, LOCKFILE_EXCLUSIVE_LOCK, 0,
                            EGS_CONTROL_FILE_LOCK_LENGTH, 0, &overlapped);
    }
    if (result) {
        return 0;
    }

    switch (GetLastError()) {
        case ERROR_ACCESS_DENIED:
        case ERROR_LOCK_VIOLATION:
            errno = EACCES;
            break;
        case ERROR_INVALID_HANDLE:
            errno = EBADF;
            break;
        default:
            errno = EIO;
            break;
    }
    return errno;
}

void egsCreateControlFile(const char *fname, int *status, int len) {
    (void)len;
    discard_pending_control_file();
    if (__my_fd >= 0) {
        _close(__my_fd);
        __my_fd = -1;
    }
    __is_locked = 0;
    if (create_control_file_temp(fname) != 0) {
        *status = errno;
        perror("egs_create_control_file: temporary file creation failed ");
        return;
    }
    *status = 0;
}

void egsOpenControlFile(const char *fname, int *status, int len) {
    int attempt;
    (void)len;
    *status = 0;
    discard_pending_control_file();
    if (__my_fd >= 0) {
        _close(__my_fd);
        __my_fd = -1;
    }
    __is_locked = 0;
    for (attempt = 0; attempt < 15; ++attempt) {
        __my_fd = _open(fname, _O_RDWR, _S_IREAD | _S_IWRITE);
        if (__my_fd >= 0) {
            break;
        }
        Sleep(1000);
    }
    if (__my_fd < 0) {
        *status = __my_fd;
        perror("egs_open_control_file: _open failed ");
    }
    else {
        *status = 0;
    }
}

void egsCloseControlFile(int *status) {
    discard_pending_control_file();
    if (__my_fd >= 0) {
        int fd = __my_fd;
        __my_fd = -1;
        *status = _close(fd);
    }
    else {
        *status = 0;
    }
    __is_locked = 0;
}

void egsLockControlFile(int *status) {
    long position;
    if (__is_locked) {
        *status = 0;
        return;
    }
    if (__my_fd < 0) {
        *status = -1;
        return;
    }
    position = _lseek(__my_fd, 0L, SEEK_SET);
    if (position != 0) {
        *status = -1;
        perror("egs_lock_control_file: lseek failed ");
        return;
    }
    *status = lock_control_file(0);
    if (*status == 0) {
        __is_locked = 1;
    }
    else {
        fprintf(stderr, "LockFileEx failed with status %d\n", *status);
        perror("LockFileEx failed: ");
    }
}

void egsUnlockControlFile(int *status) {
    long position;
    int publish_status;
    if (!__is_locked) {
        *status = 0;
        return;
    }
    if (__my_fd < 0) {
        *status = -1;
        return;
    }
    position = _lseek(__my_fd, 0L, SEEK_SET);
    if (position != 0) {
        *status = -1;
        perror("egs_unlock_control_file: failed to rewind file ");
        return;
    }
    publish_status = publish_control_file_temp();
    if (publish_status != 0) {
        discard_pending_control_file();
        _close(__my_fd);
        __my_fd = -1;
        __is_locked = 0;
        *status = publish_status;
        errno = publish_status;
        return;
    }
    *status = lock_control_file(1);
    if (*status == 0) {
        __is_locked = 0;
    }
    else {
        fprintf(stderr, "UnlockFileEx failed with status %d\n", *status);
        perror("UnlockFileEx failed: ");
    }
}

void egsRewindControlFile(int *status) {
    long position;
    if (__my_fd < 0) {
        *status = -1;
        return;
    }
    position = _lseek(__my_fd, 0L, SEEK_SET);
    if (position != 0) {
        *status = -1;
        perror("egs_rewind_control_file: lseek failed ");
        return;
    }
    if (!__is_locked) {
        egsLockControlFile(status);
    }
    else {
        *status = 0;
    }
}

void egsWriteControlFile(const char *buf, const int *n, int *status, int len) {
    (void)len;
    if (__my_fd < 0) {
        *status = 0;
        return;
    }
    *status = _write(__my_fd, buf, (unsigned int)*n);
}

void egsReadControlFile(char *buf, const int *n, int *status, int len) {
    (void)len;
    if (__my_fd < 0) {
        *status = 0;
        return;
    }
    *status = _read(__my_fd, buf, (unsigned int)*n);
}

#else

static int __is_initialized = 0;
static struct flock fl_write, fl_unlock;
static char *__pending_control_temp = NULL;
static char *__pending_control_final = NULL;

static void init_locking(void) {
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

static void discard_pending_control_file(void) {
    int saved_errno = errno;
    if (__pending_control_temp) {
        unlink(__pending_control_temp);
    }
    free(__pending_control_temp);
    free(__pending_control_final);
    __pending_control_temp = NULL;
    __pending_control_final = NULL;
    errno = saved_errno;
}

static int create_control_file_temp(const char *fname) {
    static const char suffix[] = ".tmp.XXXXXX";
    size_t fname_len = strlen(fname);
    char *temp_name;
    char *final_name;
    int lock_status;

    if (fname_len > (size_t)-1 - sizeof(suffix)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    temp_name = (char *)malloc(fname_len + sizeof(suffix));
    final_name = (char *)malloc(fname_len + 1);
    if (!temp_name || !final_name) {
        free(temp_name);
        free(final_name);
        errno = ENOMEM;
        return -1;
    }
    memcpy(temp_name, fname, fname_len);
    memcpy(temp_name + fname_len, suffix, sizeof(suffix));
    memcpy(final_name, fname, fname_len + 1);

    __my_fd = mkstemp(temp_name);
    if (__my_fd < 0) {
        int saved_errno = errno;
        free(temp_name);
        free(final_name);
        errno = saved_errno;
        return -1;
    }
    egsLockControlFile(&lock_status);
    if (lock_status != 0) {
        int lock_errno = lock_status > 0 ? lock_status : errno;
        close(__my_fd);
        __my_fd = -1;
        unlink(temp_name);
        free(temp_name);
        free(final_name);
        __is_locked = 0;
        errno = lock_errno;
        return -1;
    }
    __pending_control_temp = temp_name;
    __pending_control_final = final_name;
    return 0;
}

static int publish_control_file_temp(void) {
    if (!__pending_control_temp) {
        return 0;
    }
    if (link(__pending_control_temp, __pending_control_final) != 0) {
        return errno;
    }
    if (unlink(__pending_control_temp) != 0) {
        fprintf(stderr, "egsCreateControlFile: could not remove temporary file '%s': %s\n",
                __pending_control_temp, strerror(errno));
    }
    free(__pending_control_temp);
    free(__pending_control_final);
    __pending_control_temp = NULL;
    __pending_control_final = NULL;
    return 0;
}

#define EGS_CONTROL_FILE_OPEN_TIMEOUT_DEFAULT 15

static int get_control_file_open_timeout(long *timeout) {
    const char *value = getenv("EGS_CONTROL_FILE_OPEN_TIMEOUT");
    char *end;
    long parsed;
    if (!value) {
        *timeout = EGS_CONTROL_FILE_OPEN_TIMEOUT_DEFAULT;
        return 0;
    }
    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno == ERANGE || end == value || *end != '\0' || parsed < 0) {
        return EINVAL;
    }
    *timeout = parsed;
    return 0;
}

#ifdef EGS_LOCK_WAIT_UNBLOCKING
#define EGS_LOCK_WAIT_BACKOFF_DEFAULT_MS 50
#define EGS_LOCK_WAIT_BACKOFF_MIN_MS 10
#define EGS_LOCK_WAIT_BACKOFF_LIMIT_MS 1000
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
        if (state == 0) {
            state = 1;
        }
        seed_pid = current_pid;
    }
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state % upper_bound;
}

static int get_lock_wait_backoff_max_ms(unsigned int *max_backoff_ms) {
    const char *value = getenv("EGS_LOCK_WAIT_BACKOFF_MAX_MS");
    char *end;
    long parsed;
    if (!value) {
        *max_backoff_ms = EGS_LOCK_WAIT_BACKOFF_DEFAULT_MS;
        return 0;
    }
    errno = 0;
    parsed = strtol(value, &end, 10);
    if (errno == ERANGE || end == value || *end != '\0' ||
        parsed < EGS_LOCK_WAIT_BACKOFF_MIN_MS ||
        parsed > EGS_LOCK_WAIT_BACKOFF_LIMIT_MS) {
        return EINVAL;
    }
    *max_backoff_ms = (unsigned int)parsed;
    return 0;
}
#endif

void egsCreateControlFile(const char *fname, int *status, int len) {
    (void)len;
    if (!__is_initialized) {
        init_locking();
    }
    *status = 0;
    discard_pending_control_file();
    if (__my_fd > 0) {
        close(__my_fd);
        __my_fd = -1;
    }
    __is_locked = 0;
    if (create_control_file_temp(fname) != 0) {
        *status = errno;
    }
}

void egsOpenControlFile(const char *fname, int *status, int len) {
    int open_error, sleep_remaining;
    long timeout, waited = 0;
    (void)len;
    *status = get_control_file_open_timeout(&timeout);
    if (*status != 0) {
        fprintf(stderr, "Invalid EGS_CONTROL_FILE_OPEN_TIMEOUT: expected seconds >= 0.\n");
        return;
    }
    if (!__is_initialized) {
        init_locking();
    }
    if (__my_fd > 0) {
        close(__my_fd);
        __my_fd = -1;
    }
    __is_locked = 0;
    for (;;) {
        __my_fd = open(fname, O_RDWR);
        if (__my_fd > 0) {
            break;
        }
        open_error = errno;
        if (open_error != ENOENT && open_error != EINTR) {
            *status = open_error;
            return;
        }
        if (timeout != 0 && waited >= timeout) {
            *status = open_error;
            return;
        }
        if (waited >= 60 && waited % 60 == 0) {
            fprintf(stderr, "egsOpenControlFile: still waiting for control file '%s' after %ld seconds.\n",
                    fname, waited);
        }
        sleep_remaining = 1;
        while (sleep_remaining > 0) {
            sleep_remaining = sleep(sleep_remaining);
        }
        ++waited;
    }
}

void egsCloseControlFile(int *status) {
    if (__my_fd > 0) {
        int fd = __my_fd;
        discard_pending_control_file();
        __my_fd = -1;
        *status = close(fd);
    }
    else {
        *status = 0;
    }
    __is_locked = 0;
}

void egsLockControlFile(int *status) {
    if (__is_locked) {
        *status = 0;
        return;
    }
    if (__my_fd < 0) {
        *status = -1;
        return;
    }
#ifdef EGS_LOCK_WAIT_UNBLOCKING
    unsigned int backoff_ms = 1;
    unsigned int max_backoff_ms;
    struct timespec wait_start, now, delay;
    *status = get_lock_wait_backoff_max_ms(&max_backoff_ms);
    if (*status != 0) {
        fprintf(stderr, "Invalid EGS_LOCK_WAIT_BACKOFF_MAX_MS: expected 10-1000 milliseconds.\n");
        return;
    }
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
        if (remaining_ns <= 0) {
            break;
        }
        *status = fcntl(__my_fd, F_SETLK, &fl_write);
        if (*status == 0) {
            __is_locked = 1;
            return;
        }
        unsigned int delay_ms = 1 + lock_wait_random(backoff_ms);
        long long delay_ns = (long long)delay_ms * 1000000LL;
        if (delay_ns > remaining_ns) {
            delay_ns = remaining_ns;
        }
        delay.tv_sec = delay_ns / 1000000000LL;
        delay.tv_nsec = delay_ns % 1000000000LL;
        if (nanosleep(&delay, NULL) == -1 && errno != EINTR) {
            *status = errno;
            return;
        }
        if (backoff_ms < max_backoff_ms) {
            backoff_ms *= 2;
            if (backoff_ms > max_backoff_ms) {
                backoff_ms = max_backoff_ms;
            }
        }
    }
#else
    do {
        *status = fcntl(__my_fd, F_SETLKW, &fl_write);
    } while (*status != 0 && errno == EINTR);
    if (*status == 0) {
        __is_locked = 1;
        return;
    }
#endif
#ifdef EGS_LOCK_WAIT_UNBLOCKING
    printf("egsLockControlFile: failed to lock file after 60 second wait!\n");
#else
    printf("egsLockControlFile: blocking lock failed!\n");
#endif
}

void egsUnlockControlFile(int *status) {
    if (!__is_locked) {
        *status = 0;
        return;
    }
    if (__my_fd < 0) {
        *status = -1;
        return;
    }
    *status = publish_control_file_temp();
    if (*status != 0) {
        int publish_errno = *status;
        discard_pending_control_file();
        close(__my_fd);
        __my_fd = -1;
        __is_locked = 0;
        errno = publish_errno;
        return;
    }
    *status = fcntl(__my_fd, F_SETLKW, &fl_unlock);
    if (*status == 0) {
        __is_locked = 0;
    }
}

void egsRewindControlFile(int *status) {
    if (__my_fd < 0) {
        *status = -1;
        return;
    }
    if (!__is_locked) {
        egsLockControlFile(status);
        if (*status != 0) {
            return;
        }
    }
    *status = lseek(__my_fd, 0, SEEK_SET);
}

void egsWriteControlFile(const char *buf, const int *n, int *status, int len) {
    (void)len;
    if (__my_fd < 0) {
        *status = 0;
        return;
    }
    *status = write(__my_fd, buf, (size_t)*n);
}

void egsReadControlFile(char *buf, const int *n, int *status, int len) {
    (void)len;
    if (__my_fd < 0) {
        *status = 0;
        return;
    }
    *status = read(__my_fd, buf, (size_t)*n);
}

#endif

#if defined(WIN32) && defined(MAKE_WIN_DISTRIBUTION)
#define EGS_FILE_ALIASES(lower, upper, target) \
    void C_CONVENTION lower##_(const char *fname, int *status, int len) { target(fname, status, len); } \
    void C_CONVENTION lower##__(const char *fname, int *status, int len) { target(fname, status, len); } \
    void C_CONVENTION upper(const char *fname, int *status, int len) { target(fname, status, len); } \
    void C_CONVENTION upper##_(const char *fname, int *status, int len) { target(fname, status, len); } \
    void C_CONVENTION upper##__(const char *fname, int *status, int len) { target(fname, status, len); }

#define EGS_STATUS_ALIASES(lower, upper, target) \
    void C_CONVENTION lower##_(int *status) { target(status); } \
    void C_CONVENTION lower##__(int *status) { target(status); } \
    void C_CONVENTION upper(int *status) { target(status); } \
    void C_CONVENTION upper##_(int *status) { target(status); } \
    void C_CONVENTION upper##__(int *status) { target(status); }

#define EGS_BUFFER_ALIASES(lower, upper, target, buffer_type) \
    void C_CONVENTION lower##_(buffer_type *buf, const int *n, int *status, int len) { target(buf, n, status, len); } \
    void C_CONVENTION lower##__(buffer_type *buf, const int *n, int *status, int len) { target(buf, n, status, len); } \
    void C_CONVENTION upper(buffer_type *buf, const int *n, int *status, int len) { target(buf, n, status, len); } \
    void C_CONVENTION upper##_(buffer_type *buf, const int *n, int *status, int len) { target(buf, n, status, len); } \
    void C_CONVENTION upper##__(buffer_type *buf, const int *n, int *status, int len) { target(buf, n, status, len); }

EGS_FILE_ALIASES(egs_create_control_file, EGS_CREATE_CONTROL_FILE, egsCreateControlFile)
EGS_FILE_ALIASES(egs_open_control_file, EGS_OPEN_CONTROL_FILE, egsOpenControlFile)
EGS_STATUS_ALIASES(egs_close_control_file, EGS_CLOSE_CONTROL_FILE, egsCloseControlFile)
EGS_STATUS_ALIASES(egs_lock_control_file, EGS_LOCK_CONTROL_FILE, egsLockControlFile)
EGS_STATUS_ALIASES(egs_unlock_control_file, EGS_UNLOCK_CONTROL_FILE, egsUnlockControlFile)
EGS_STATUS_ALIASES(egs_rewind_control_file, EGS_REWIND_CONTROL_FILE, egsRewindControlFile)
EGS_BUFFER_ALIASES(egs_write_control_file, EGS_WRITE_CONTROL_FILE, egsWriteControlFile, const char)
EGS_BUFFER_ALIASES(egs_read_control_file, EGS_READ_CONTROL_FILE, egsReadControlFile, char)

#undef EGS_FILE_ALIASES
#undef EGS_STATUS_ALIASES
#undef EGS_BUFFER_ALIASES
#endif