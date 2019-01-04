#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.5)
AC_INIT(x5250, [0.5.4], [james@eaerich.com])
AC_CONFIG_SRCDIR([src/alloccolors.c])
AC_CONFIG_HEADERS([config.h])

#
# Automake
#
AM_INIT_AUTOMAKE

#
# Automake
#
AH_TEMPLATE([SOCKET_TYPE], [Define to your system's socket descriptor type.])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_PATH_X
AC_PATH_XTRA

# Get various type sizes
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)

# True for anything other than Windoze.
AC_DEFINE_UNQUOTED(SOCKET_TYPE,int)

# 
# Look for lib5250
#
#PKG_CHECK_MODULES(TN5250, tn5250, [have_tn5250="yes"], [have_tn5250="no"])
#if test x$have_tn5250 = xno ; then
#   AC_MSG_ERROR(Can't find tn5250)
#fi
AC_CHECK_LIB(5250, tn5250_terminal_init, TN5250_LIB=-l5250)
if test "$ac_cv_lib_5250_tn5250_terminal_init" != "yes"
then
	AC_MSG_ERROR([** Unable to find 5250 libraries!])
fi

AC_SUBST([TN5250_LIB])
#AC_SUBST(TN5250_CFLAGS)
#CFLAGS="$CFLAGS $TN5250_CFLAGS"
#LIBS="$TN5250_LIBS $X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS"
LIBS="$X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS"

# Checks for header files.
AC_HEADER_STDC
#AC_CHECK_HEADERS(locale.h stdlib.h unistd.h getopt.h tn5250.h,,,
#[[#if HAVE_TN5250_H
## include <tn5250.h>
##endif
#]])
AC_CHECK_HEADERS([locale.h stdlib.h unistd.h getopt.h tn5250.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
# Really we should check malloc but danged if I can figure out how to
# get the replacement function to work (AC_LIBOBJ(malloc)).  Since this
# just seems to make things harder forget it for now (and AC_LIBOBJ below).
#AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([gethostname memset select setlocale inet_aton])
#AC_LIBOBJ(malloc)

# Check for features in lib5250
AC_MSG_CHECKING([for macro support in lib5250])
tn5250macro=no
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <tn5250.h>
#include <tn5250/macro.h>]])],
	 [AC_DEFINE([MACRO_SUPPORT], [], [tn5250 macros]) tn5250macro=yes])
AC_MSG_RESULT([$tn5250macro])

AC_MSG_CHECKING([for enhanced 5250 support in lib5250])
tn5250enhanced=no
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <tn5250.h>
#include <tn5250/window.h>]])],
	 [AC_DEFINE([ENHANCED], [], [tn5250 enhanced]) tn5250enhanced=yes])
AC_MSG_RESULT([$tn5250enhanced])

# Check for Sun keysyms
AC_MSG_CHECKING([for Sun keysyms])
sunkeys=no
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <X11/Sunkeysym.h>]])],
	 [AC_DEFINE([SUN], [], [sun keysyms]) sunkeys=yes])
AC_MSG_RESULT([$sunkeys])


AC_CONFIG_FILES([Makefile
		 attic/Makefile
		 bitmaps/Makefile
		 src/Makefile])
AC_OUTPUT
