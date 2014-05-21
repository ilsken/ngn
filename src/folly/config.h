/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */
#include <boost/config.hpp>
/* Define to "final" if the compiler supports C++11 "final" */
#define FINAL final


/* Define to 1 if you have the `getdelim' function. */
#define HAVE_GETDELIM 1

/* Define to 1 if you have the `gettimeofday' function. */
#ifdef BOOST_HAS_GETTIMEOFDAY
#define HAVE_GETTIMEOFDAY 1
#else
#define HAVE_GETTIMEOFDAY 0
#endif

/* Define to 0 if the compiler doesn't support ifunc */
#define HAVE_IFUNC 0

/* Define if __int128 does not exist */
#define HAVE_INT128_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

/* Define to 1 if you have the `malloc_size' function. */
#define HAVE_MALLOC_SIZE 1

/* Define to 1 if you have the `malloc_usable_size' function. */
/* #undef HAVE_MALLOC_USABLE_SIZE */

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memrchr' function. */
/* #undef HAVE_MEMRCHR */

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <mutex.h> header file. */
/* #undef HAVE_MUTEX_H */

/* Define to 1 if you have the `pow' function. */
#define HAVE_POW 1

/* Define to 1 if you have the `pthread_yield' function. */
/* #undef HAVE_PTHREAD_YIELD */

/* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T 1

/* Define to 1 if you have the `rallocm' function. */
/* #undef HAVE_RALLOCM */

#ifdef BOOST_HAS_SCHED_YIELD
#define HAVE_SCHED_H 1
#define HAVE_SCHED_YIELD 1
#else 
#define HAVE_SCHED_H 0
#define HAVE_SCHED_YIELD 0
#endif

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define if g++ supports C++0x features. */
#define HAVE_STDCXX_0X /**/

/* Define to 1 if you have the <stdint.h> header file. */
#ifdef BOOST_HAS_STDINT_H
#define HAVE_STDINT_H 1
#else
#define HAVE_STDINT_H 0
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if we have a usable std::is_trivially_copyable<T>
   implementation. */
#define HAVE_STD__IS_TRIVIALLY_COPYABLE 1

/* Define to 1 if std::this_thread::sleep_for() is defined. */
#define HAVE_STD__THIS_THREAD__SLEEP_FOR 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#ifdef BOOST_HAS_GETTIMEOFDAY
#define HAVE_SYS_TIME_H 1
#else
#define HAS_SYS_TIME_H 0
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if the linker supports weak symbols. */
/* #undef HAVE_WEAK_SYMBOLS */

/* Define to 1 if the system has the type `_Bool'. */
/* #undef HAVE__BOOL */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define to "override" if the compiler supports C++11 "override" */
#define OVERRIDE override

/* Name of package */
#define PACKAGE "folly"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "folly@fb.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "folly"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "folly 0.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "folly"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.1"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if we're using libc++. */
#if defined(_LIBCPP_VERSION)
#define USE_LIBCPP 1
#else
#define USE_LIBCPP 0
#endif
/* Version number of package */
#define VERSION "0.1"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */

