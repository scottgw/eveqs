AC_INIT([libeveqs], [1.0], [scott.west@inf.ethz.ch])

AM_INIT_AUTOMAKE([foreign silent-rules -Wall])
dnl AM_SILENT_RULES([yes])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_CONFIG_MACRO_DIR([m4])
LT_INIT
AC_PROG_CC
AC_PROG_CXX

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

PKG_CHECK_MODULES([DEPS], [glib-2.0 >= 2.20, ck, tbb])

AC_OUTPUT
