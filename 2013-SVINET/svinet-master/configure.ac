#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([svinet], [0.9], [prem.gopalan@gmail.com])
AM_INIT_AUTOMAKE([svinet], [0.9])
AC_CONFIG_SRCDIR([src/env.hh])
# AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([gslcblas], [cblas_sdot], [], [AC_MSG_ERROR([gslcblas library was not found])])
AC_CHECK_LIB([pthread], [pthread_self], [], [AC_MSG_ERROR([pthread library was not found])])
AC_CHECK_LIB([gsl], [gsl_sf_lngamma], [], [AC_MSG_ERROR([gsl library was not found])])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h sys/file.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
# AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gettimeofday localtime_r memset mkdir pow sqrt strerror strtol])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
