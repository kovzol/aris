AC_PREREQ(2.60)
AC_INIT([GNU Aris], [2.2], [bug-aris@gnu.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_SRCDIR([src/aris.c])

AC_ARG_PROGRAM

AC_CANONICAL_HOST

# Check for windows
case "${host}" in
        *mingw32*) WIN32="yes" ;;
        *cygwin*) WIN32="yes" ;;
        *) WIN32="no" ;;
esac
AM_CONDITIONAL([WIN32], test "$WIN32" = "yes")

AC_PROG_CC_C99

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_INSTALL

# Check for pkg-config

PKG_PROG_PKG_CONFIG

# Check for gtk+-3.0

PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.2])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Check for libxml-2.0

PKG_CHECK_MODULES([XML], [libxml-2.0 >= 2.6])
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

## This really only works when compiling from source.
AC_DEFINE_UNQUOTED([HELP_FILE], "file://`pwd`/doc/aris/index.html", [The help HTML file path])

if test "$WIN32" = "yes"; then
   AC_DEFINE([WIN32], [], ["Used when compiling for Windows"])
fi

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
