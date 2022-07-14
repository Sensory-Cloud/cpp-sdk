/*
 * Arc4 random number generator for OpenBSD.
 * Copyright 1996 David Mazieres <dm@lcs.mit.edu>.
 *
 * Modification and redistribution in source and binary forms is
 * permitted provided that due credit is given to the author and the
 * OpenBSD project (for instance by leaving this copyright notice
 * intact).
 *
 * This code is derived from section 17.1 of Applied Cryptography,
 * second edition, which describes a stream cipher allegedly
 * compatible with RSA Labs "RC4" cipher (the actual description of
 * which is a trade secret).  The same algorithm is used as a stream
 * cipher called "arcfour" in Tatu Ylonen's ssh package.
 *
 * Here the stream cipher has been modified always to include the time
 * when initializing the state.  That makes it impossible to
 * regenerate the same random sequence twice, so this can't be used
 * for encryption, but will generate good random numbers.
 *
 * RC4 is a registered trademark of RSA Laboratories.
 *
 * Derived from:
 * https://opensource.apple.com/source/Libc/Libc-594.9.4/gen/FreeBSD/arc4random.c
 * by Sensory, Inc. 2022
 */

#ifndef ARC4RANDOM_HPP_
#define ARC4RANDOM_HPP_

// #include <sys/cdefs.h>
// #include <sys/types.h>
#include <sys/time.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <unistd.h>

#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <mutex>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

// Windows-base systems don't define the POSIX `gettimeofday` function.
// https://stackoverflow.com/questions/10905892/equivalent-of-gettimeday-for-windows
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
/// @brief A structure for holding date-time data components.
typedef struct timeval {
    /// The number of elapsed seconds since the epoch.
    long tv_sec;
    /// The number of elapsed microseconds since the epoch.
    long tv_usec;
} timeval;

/// @brief Get the current time of the day.
///
/// @param tp The timeval pointer to populate with the current time data.
/// @param tzp The timezone defining the locale of the time of day.
/// @returns The time of day based on system clock and specified time zone.
///
int gettimeofday(struct timeval* tp, struct timezone* tzp) {
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif  // defined(_WIN32) ...

struct arc4_stream {
    uint8_t i;
    uint8_t j;
    uint8_t s[256];
};

static std::mutex arc4random_mtx;

#define RANDOMDEV   "/dev/urandom"

static struct arc4_stream rs;
static int rs_initialized;
static int rs_stired;

static inline uint8_t arc4_getbyte(struct arc4_stream *);
static void arc4_stir(struct arc4_stream *);

static inline void arc4_init(struct arc4_stream *as) {
    int     n;

    for (n = 0; n < 256; n++)
        as->s[n] = n;
    as->i = 0;
    as->j = 0;
}

static inline void arc4_addrandom(struct arc4_stream *as, unsigned char *dat, int datlen) {
    int     n;
    uint8_t si;

    as->i--;
    for (n = 0; n < 256; n++) {
        as->i = (as->i + 1);
        si = as->s[as->i];
        as->j = (as->j + si + dat[n % datlen]);
        as->s[as->i] = as->s[as->j];
        as->s[as->j] = si;
    }
}

static void arc4_stir(struct arc4_stream *as) {
    int     fd, n;
    struct {
        struct timeval tv;
        pid_t pid;
        uint8_t rnd[128 - sizeof(struct timeval) - sizeof(pid_t)];
    }       rdat;

    gettimeofday(&rdat.tv, NULL);
    rdat.pid = getpid();
    fd = open(RANDOMDEV, O_RDONLY, 0);
    if (fd >= 0) {
        (void) read(fd, rdat.rnd, sizeof(rdat.rnd));
        close(fd);
    }
    /* fd < 0?  Ah, what the heck. We'll just take whatever was on the
     * stack... */

    arc4_addrandom(as, (unsigned char *) &rdat, sizeof(rdat));

    /*
     * Throw away the first N bytes of output, as suggested in the
     * paper "Weaknesses in the Key Scheduling Algorithm of RC4"
     * by Fluher, Mantin, and Shamir.  N=1024 is based on
     * suggestions in the paper "(Not So) Random Shuffles of RC4"
     * by Ilya Mironov.
     */
    for (n = 0; n < 1024; n++)
        arc4_getbyte(as);
}

static inline uint8_t arc4_getbyte(struct arc4_stream *as) {
    uint8_t si, sj;

    as->i = (as->i + 1);
    si = as->s[as->i];
    as->j = (as->j + si);
    sj = as->s[as->j];
    as->s[as->i] = sj;
    as->s[as->j] = si;

    return (as->s[(si + sj) & 0xff]);
}

static inline uint32_t arc4_getword(struct arc4_stream *as) {
    uint32_t val;

    val = arc4_getbyte(as) << 24;
    val |= arc4_getbyte(as) << 16;
    val |= arc4_getbyte(as) << 8;
    val |= arc4_getbyte(as);

    return (val);
}

static void arc4_check_init(void) {
    if (!rs_initialized) {
        arc4_init(&rs);
        rs_initialized = 1;
    }
}

static void arc4_check_stir(void) {
    if (!rs_stired) {
        arc4_stir(&rs);
        rs_stired = 1;
    }
}

void arc4random_stir() {
    std::lock_guard<std::mutex> lock(arc4random_mtx);
    arc4_check_init();
    arc4_stir(&rs);
}

void arc4random_addrandom(unsigned char *dat, int datlen) {
    std::lock_guard<std::mutex> lock(arc4random_mtx);
    arc4_check_init();
    arc4_check_stir();
    arc4_addrandom(&rs, dat, datlen);
}

uint32_t arc4_getword() {
    std::lock_guard<std::mutex> lock(arc4random_mtx);
    arc4_check_init();
    arc4_check_stir();
    return arc4_getword(&rs);
}

uint8_t arc4_getbyte() {
    std::lock_guard<std::mutex> lock(arc4random_mtx);
    arc4_check_init();
    arc4_check_stir();
    return arc4_getbyte(&rs);
}

void arc4random_buf(unsigned char* buffer, size_t n) {
    arc4_check_init();
    arc4_check_stir();
    while (n--) buffer[n] = arc4_getbyte(&rs);
}

}  // namespace token_manager

}  // namespace sensory

#endif  // ARC4RANDOM_HPP_
