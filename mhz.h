#ifndef _MHZ_H
#define _MHZ_H

#ifdef WIN32
#include <windows.h>
typedef unsigned char bool_t;
#endif

#include        <assert.h>
#include        <ctype.h>
#include        <stdio.h>
#include        <math.h>
#ifndef WIN32
#include        <unistd.h>
#endif
#include        <stdlib.h>
#include        <fcntl.h>
#include        <signal.h>
#include        <errno.h>
#ifndef WIN32
#include        <strings.h>
#endif
#include        <sys/types.h>
#ifndef WIN32
#include        <sys/mman.h>
#endif
#include        <sys/stat.h>
#ifndef WIN32
#include        <sys/wait.h>
#include        <time.h>
#include        <sys/time.h>
#include        <sys/socket.h>
#include        <sys/un.h>
#include        <sys/resource.h>
#define PORTMAP
//#include        <rpc/rpc.h>
#endif
#ifdef HAVE_pmap_clnt_h
#include        <rpc/pmap_clnt.h>
#endif
//#include        <rpc/types.h>
#ifdef HAVE_pmap_clnt_h
#include        <rpc/pmap_clnt.h>
#endif

#include        <stdarg.h>
#ifndef HAVE_uint
typedef unsigned int uint;
#endif

#ifndef S_IREAD
#define S_IREAD S_IRUSR
#endif
#ifndef S_IWRITE
#define S_IWRITE S_IWUSR
#endif
#ifndef S_IEXEC
#define S_IEXEC S_IXUSR
#endif

#ifndef HAVE_uint64
#ifdef HAVE_uint64_t
typedef uint64_t uint64;
#else /* HAVE_uint64_t */
typedef unsigned long long uint64;
#endif /* HAVE_uint64_t */
#endif /* HAVE_uint64 */

#ifndef HAVE_int64
#ifdef HAVE_int64_t
typedef int64_t int64;
#else /* HAVE_int64_t */
typedef long long int64;
#endif /* HAVE_int64_t */
#endif /* HAVE_int64 */

//#ifndef HAVE_socklen_t
//typedef int socklen_t;
//#endif

#ifndef HAVE_off64_t
typedef int64 off64_t;
#endif

#define NO_PORTMAPPER

#include        "stats.h"
#include        "timing.h"
//#include        "lib_debug.h"
//#include        "lib_tcp.h"
//#include        "lib_udp.h"
//#include        "lib_unix.h"

//////
#if defined(SYS5) || defined(WIN32)
#define bzero(b, len)   memset(b, 0, len)
#define bcopy(s, d, l)  memcpy(d, s, l)
#define rindex(s, c)    strrchr(s, c)
#endif
#define gettime         usecs_spent
#define streq           !strcmp
#define ulong           unsigned long


//////

#ifdef WIN32
#include <process.h>
#define getpid _getpid
int     gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

#define SMALLEST_LINE   32              /* smallest cache line size */
#define TIME_OPEN2CLOSE

#define GO_AWAY signal(SIGALRM, exit); alarm(60 * 60);
#define REAL_SHORT         50000
#define SHORT            1000000
#define MEDIUM           2000000
#define LONGER           7500000        /* for networking data transfers */
#define ENOUGH          REAL_SHORT

#define TRIES           11

typedef struct {
        uint64 u;
        uint64 n;
} value_t;

typedef struct {
        int     N;
        value_t v[TRIES];
} result_t;
int     sizeof_result(int N);
void    insertinit(result_t *r);
void    insertsort(uint64, uint64, result_t *);
void    save_median();
void    save_minimum();
void    set_results(result_t *r);
result_t* get_results();


#define BENCHO(loop_body, overhead_body, enough) {                      \
        int             __i, __N;                                       \
        double          __oh;                                           \
        result_t        __overhead, __r;                                \
        insertinit(&__overhead); insertinit(&__r);                      \
        __N = (enough == 0 || get_enough(enough) <= 100000) ? TRIES : 1;\
        if (enough < LONGER) {loop_body;} /* warm the cache */          \
        for (__i = 0; __i < __N; ++__i) {                               \
                BENCH1(overhead_body, enough);                          \
                if (gettime() > 0)                                      \
                        insertsort(gettime(), get_n(), &__overhead);    \
                BENCH1(loop_body, enough);                              \
                if (gettime() > 0)                                      \
                        insertsort(gettime(), get_n(), &__r);           \
        }                                                               \
        for (__i = 0; __i < __r.N; ++__i) {                             \
                __oh = __overhead.v[__i].u / (double)__overhead.v[__i].n; \
                if (__r.v[__i].u > (uint64)((double)__r.v[__i].n * __oh)) \
                        __r.v[__i].u -= (uint64)((double)__r.v[__i].n * __oh);  \
                else                                                    \
                        __r.v[__i].u = 0;                               \
        }                                                               \
        *(get_results()) = __r;                                         \
}

#define BENCH(loop_body, enough) {                                      \
        long            __i, __N;                                       \
        result_t        __r;                                            \
        insertinit(&__r);                                               \
        __N = (enough == 0 || get_enough(enough) <= 100000) ? TRIES : 1;\
        if (enough < LONGER) {loop_body;} /* warm the cache */          \
        for (__i = 0; __i < __N; ++__i) {                               \
                BENCH1(loop_body, enough);                              \
                if (gettime() > 0)                                      \
                        insertsort(gettime(), get_n(), &__r);           \
        }                                                               \
        *(get_results()) = __r;                                         \
}

#define BENCH1(loop_body, enough) {                                     \
        double          __usecs;                                        \
        BENCH_INNER(loop_body, enough);                                 \
        __usecs = gettime();                                            \
        __usecs -= t_overhead() + get_n() * l_overhead();               \
        settime(__usecs >= 0. ? (uint64)__usecs : 0);                   \
}

#define BENCH_INNER(loop_body, enough) {                                \
        static iter_t   __iterations = 1;                               \
        int             __enough = get_enough(enough);                  \
        iter_t          __n;                                            \
        double          __result = 0.;                                  \
                                                                        \
        while(__result < 0.95 * __enough) {                             \
                start(0);                                               \
                for (__n = __iterations; __n > 0; __n--) {              \
                        loop_body;                                      \
                }                                                       \
                __result = stop(0,0);                                   \
                if (__result < 0.99 * __enough                          \
                    || __result > 1.2 * __enough) {                     \
                        if (__result > 150.) {                          \
                                double  tmp = __iterations / __result;  \
                                tmp *= 1.1 * __enough;                  \
                                __iterations = (iter_t)(tmp + 1);       \
                        } else {                                        \
                                if (__iterations > (iter_t)1<<27) {     \
                                        __result = 0.;                  \
                                        break;                          \
                                }                                       \
                                __iterations <<= 3;                     \
                        }                                               \
                }                                                       \
        } /* while */                                                   \
        save_n((uint64)__iterations); settime((uint64)__result);        \
}

/* getopt stuff */
#define getopt  mygetopt
#define optind  myoptind
#define optarg  myoptarg
#define opterr  myopterr
#define optopt  myoptopt
extern  int     optind;
extern  int     opterr;
extern  int     optopt;
extern  char    *optarg;
int     getopt(int ac, char **av, char *opts);
void    lmbench_usage(int argc, char *argv[], char* usage);
off64_t seekto(int fd, off64_t off, int whence);

typedef u_long iter_t;
typedef void (*benchmp_f)(iter_t iterations, void* cookie);

extern void benchmp(benchmp_f initialize,
                    benchmp_f benchmark,
                    benchmp_f cleanup,
                    int enough,
                    int parallel,
                    int warmup,
                    int repetitions,
                    void* cookie
        );


#endif //_MHZ_H
