
## va_copy check
##

dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/acx_pthread.html
dnl
AC_DEFUN([ACX_PTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_SAVE
AC_LANG_C
acx_pthread_ok=no

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
        AC_TRY_LINK_FUNC(pthread_join, acx_pthread_ok=yes)
        AC_MSG_RESULT($acx_pthread_ok)
        if test x"$acx_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
fi

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all, and "pthread-config"
# which is a program returning the flags for the Pth emulation library.

acx_pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config"

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
# pthread: Linux, etcetera
# --thread-safe: KAI C++
# pthread-config: use pthread-config program (for GNU Pth library)

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthread or
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        acx_pthread_flags="-pthread -pthreads pthread -mt $acx_pthread_flags"
        ;;
esac

if test x"$acx_pthread_ok" = xno; then
for flag in $acx_pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

		pthread-config)
		AC_CHECK_PROG(acx_pthread_config, pthread-config, yes, no)
		if test x"$acx_pthread_config" = xno; then continue; fi
		PTHREAD_CFLAGS="`pthread-config --cflags`"
		PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
		;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_TRY_LINK([#include <pthread.h>],
                    [pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ],
                    [acx_pthread_ok=yes])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        AC_MSG_RESULT($acx_pthread_ok)
        if test "x$acx_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi

# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: threads are created detached by default
        # and the JOINABLE attribute has a nonstandard name (UNDETACHED).
        AC_MSG_CHECKING([for joinable pthread attribute])
        AC_TRY_LINK([#include <pthread.h>],
                    [int attr=PTHREAD_CREATE_JOINABLE;],
                    ok=PTHREAD_CREATE_JOINABLE, ok=unknown)
        if test x"$ok" = xunknown; then
                AC_TRY_LINK([#include <pthread.h>],
                            [int attr=PTHREAD_CREATE_UNDETACHED;],
                            ok=PTHREAD_CREATE_UNDETACHED, ok=unknown)
        fi
        if test x"$ok" != xPTHREAD_CREATE_JOINABLE; then
                AC_DEFINE(PTHREAD_CREATE_JOINABLE, $ok,
                          [Define to the necessary symbol if this constant
                           uses a non-standard name on your system.])
        fi
        AC_MSG_RESULT(${ok})
        if test x"$ok" = xunknown; then
                AC_MSG_WARN([we do not know how to create joinable pthreads])
        fi

        AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case "${host_cpu}-${host_os}" in
                *-aix* | *-freebsd* | *-darwin*) flag="-D_THREAD_SAFE";;
                *solaris* | *-osf* | *-hpux*) flag="-D_REENTRANT";;
        esac
        AC_MSG_RESULT(${flag})
        if test "x$flag" != xno; then
                PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        # More AIX lossage: must compile with cc_r
        AC_CHECK_PROG(PTHREAD_CC, cc_r, cc_r, ${CC})
else
        PTHREAD_CC="$CC"
fi

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_CC)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pthread_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.]),[$1])
        :
else
        acx_pthread_ok=no
        $2
fi
AC_LANG_RESTORE
])dnl ACX_PTHREAD



## va_copy check
##

dnl ##
dnl ##  NAME:
dnl ##    AC_CHECK_VA_COPY -- Check for C99 va_copy
dnl ##
dnl ##  COPYRIGHT
dnl ##    Copyright (c) 2006 Ralf S. Engelschall <rse@engelschall.com>
dnl ##
dnl ##  DESCRIPTION:
dnl ##    This macro checks for C99 va_copy() implementation and
dnl ##    provides fallback implementation if neccessary. Notice: this
dnl ##    check is rather complex because first because we really have
dnl ##    to try various possible implementations in sequence and
dnl ##    second, we cannot define a macro in config.h with parameters
dnl ##    directly.
dnl ##
dnl ##  USAGE:
dnl ##    configure.in:
dnl ##      AC_CHECK_VA_COPY
dnl ##    foo.c:
dnl ##      #include "config.h"
dnl ##      [...]
dnl ##      va_copy(d,s)
dnl ##

dnl #   test program for va_copy() implementation
changequote(<<,>>)
m4_define(__va_copy_test, <<[
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define DO_VA_COPY(d, s) $1
void test(char *str, ...)
{
    va_list ap, ap2;
    int i;
    va_start(ap, str);
    DO_VA_COPY(ap2, ap);
    for (i = 1; i <= 9; i++) {
        int k = (int)va_arg(ap, int);
        if (k != i)
            abort();
    }
    DO_VA_COPY(ap, ap2);
    for (i = 1; i <= 9; i++) {
        int k = (int)va_arg(ap, int);
        if (k != i)
            abort();
    }
    va_end(ap);
}
int main(int argc, char *argv[])
{
    test("test", 1, 2, 3, 4, 5, 6, 7, 8, 9);
    exit(0);
}
]>>)
changequote([,])

dnl #   test driver for va_copy() implementation
m4_define(__va_copy_check, [
    AH_VERBATIM($1,
[/* Predefined possible va_copy() implementation (id: $1) */
#define __VA_COPY_USE_$1(d, s) $2])
    if test ".$ac_cv_va_copy" = .; then
        AC_TRY_RUN(__va_copy_test($2), [ac_cv_va_copy="$1"])
    fi
])

dnl #   Autoconf check for va_copy() implementation checking
AC_DEFUN([AC_CHECK_VA_COPY],[
  dnl #   provide Autoconf display check message
  AC_MSG_CHECKING(for va_copy() function)
  dnl #   check for various implementations in priorized sequence   
  AC_CACHE_VAL(ac_cv_va_copy, [
    ac_cv_va_copy=""
    dnl #   1. check for standardized C99 macro
    __va_copy_check(C99, [va_copy((d), (s))])
    dnl #   2. check for alternative/deprecated GCC macro
    __va_copy_check(GCM, [VA_COPY((d), (s))])
    dnl #   3. check for internal GCC macro (high-level define)
    __va_copy_check(GCH, [__va_copy((d), (s))])
    dnl #   4. check for internal GCC macro (built-in function)
    __va_copy_check(GCB, [__builtin_va_copy((d), (s))])
    dnl #   5. check for assignment approach (assuming va_list is a struct)
    __va_copy_check(ASS, [do { (d) = (s); } while (0)])
    dnl #   6. check for assignment approach (assuming va_list is a pointer)
    __va_copy_check(ASP, [do { *(d) = *(s); } while (0)])
    dnl #   7. check for memory copying approach (assuming va_list is a struct)
    __va_copy_check(CPS, [memcpy((void *)&(d), (void *)&(s)), sizeof((s))])
    dnl #   8. check for memory copying approach (assuming va_list is a pointer)
    __va_copy_check(CPP, [memcpy((void *)(d), (void *)(s)), sizeof(*(s))])
    if test ".$ac_cv_va_copy" = .; then
        AC_ERROR([no working implementation found])
    fi
  ])
  dnl #   optionally activate the fallback implementation
  if test ".$ac_cv_va_copy" = ".C99"; then
      AC_DEFINE(HAVE_VA_COPY, 1, [Define if va_copy() macro exists (and no fallback implementation is required)])
  fi
  dnl #   declare which fallback implementation to actually use
  AC_DEFINE_UNQUOTED([__VA_COPY_USE], [__VA_COPY_USE_$ac_cv_va_copy],
      [Define to id of used va_copy() implementation])
  dnl #   provide activation hook for fallback implementation
  AH_VERBATIM([__VA_COPY_ACTIVATION],
[/* Optional va_copy() implementation activation */
#ifndef HAVE_VA_COPY
#define va_copy(d, s) __VA_COPY_USE(d, s)
#define HAVE_VA_COPY 1
#endif
])
  dnl #   provide Autoconf display result message
  if test ".$ac_cv_va_copy" = ".C99"; then
      AC_MSG_RESULT([yes])
  else
      AC_MSG_RESULT([no (using fallback implementation)])
  fi
])



## Referenceable va_list
##

# http://autoconf-archive.cryp.to/ax_c_referenceable_passed_va_list.html



AC_DEFUN([AX_C_REFERENCEABLE_PASSED_VA_LIST], [
  AC_CACHE_CHECK([whether f(va_list va){ &va; } works as expected],
    [ax_cv_c_referenceable_passed_va_list],
    [AC_LINK_IFELSE([[
#include <stdarg.h>

volatile va_list g_va;

void
vf(va_list callee_va)
{
  /* typeof(callee_va) differs from typeof(caller_va) by a compiler trick, if
   * va_list is implemented as an array. On such environment, this copy
   * operation fails. */
  g_va = callee_va;
}

void
f(int last, ...)
{
  va_list caller_va;

  va_start(caller_va, last);
  vf(caller_va);  /* passed as &caller_va[0] if va_list is an array type */
  va_end(caller_va);
}

int
main(int argc, char *argv[])
{
  f(0xdeadbeef, 0xfedbeef, 0xfeedee);

  return 0;
}
      ]],
      [ax_cv_c_referenceable_passed_va_list=yes],
      [ax_cv_c_referenceable_passed_va_list=no])])
  if test "x$ax_cv_c_referenceable_passed_va_list" = xyes; then
    AC_DEFINE([HAVE_REFERENCEABLE_PASSED_VA_LIST], [1],
              [Define to 1 if f(va_list va){ &va; } works as expected.])
  fi
])





AC_DEFUN([AC_LIB_READLINE], [
  AC_CACHE_CHECK([for the readline library],
                 ac_cv_lib_readline, [
    ac_cv_lib_readline="no"
    ORIG_LIBS=$LIBS
    LIBS="$ORIG_LIBS -lreadline -ltermcap"
    AC_TRY_LINK_FUNC(readline, ac_cv_lib_readline="yes")
    if test "$ac_cv_lib_readline" = "no"; then
      LIBS=$ORIG_LIBS
    fi
  ])

  enable_readline="no"
  if test "$ac_cv_lib_readline" = "yes"; then
    AC_CHECK_HEADERS(readline.h readline/readline.h)
    AC_CACHE_CHECK([whether readline supports history],
                   ac_cv_lib_readline_history, [
      ac_cv_lib_readline_history="no"
      AC_TRY_LINK_FUNC(add_history, ac_cv_lib_readline_history="yes")
    ])
    if test "$ac_cv_lib_readline_history" = "yes"; then
      AC_CHECK_HEADERS(history.h readline/history.h)
      AC_CACHE_CHECK([whether readline supports completion],
                     ac_cv_lib_readline_completion, [
        ac_cv_lib_readline_completion="no"
        AC_TRY_LINK_FUNC(rl_completion_matches,
                        ac_cv_lib_readline_completion="yes")
      ])
      if test "$ac_cv_lib_readline_completion" = "yes"; then
   enable_readline="yes"
      fi
    fi
  else
    AC_MSG_RESULT([readline or termcap library is missing])
  fi
])

AC_DEFUN([AC_WITH_READLINE], [
    AC_ARG_WITH(readline,
   [  --with-readline=DIR     Readline installation directory],
   with_readline=$withval)

    if test "$with_readline" != ""; then
        CPPFLAGS="-I$with_readline/include $CPPFLAGS"
        LIBS="-L$with_readline/lib $LIBS"
    fi
])

AC_DEFUN([AC_MMX_OPTION_READLINE], [
    AC_ARG_ENABLE(readline,
    AC_HELP_STRING(
   [--enable-readline],
   [use the readline library [[yes]]]),
    [], [enable_readline="yes"])

    if test "$enable_readline" = "yes"; then
      AC_WITH_READLINE
      AC_LIB_READLINE
    fi
   
    if test "$enable_readline" = "no"; then
      AC_MSG_RESULT([disabling readline])
      AC_DEFINE(HAVE_READLINE, 0,
               [Define if you have the readline library])
    else
      AC_MSG_RESULT([enabling readline])
      AC_DEFINE(HAVE_READLINE, 1,
               [Define if you have the readline library])
    fi

    AM_CONDITIONAL([READLINE_OPT], [test "$enable_readline" = "yes"])
])


