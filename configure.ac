AC_PREREQ(2.60)
AC_INIT([GNU Aris], [2.0], [bug-aris@gnu.org])
AC_CONFIG_HEADER(src/config.h)
AC_CONFIG_SRCDIR(src/aris.c)
AM_INIT_AUTOMAKE
AC_ARG_PROGRAM

AC_CANONICAL_HOST

PKG_PROG_PKG_CONFIG

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

# Check for gtk+-2.0

PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.18.0])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Check for libxml-2.0

PKG_CHECK_MODULES([XML], [libxml-2.0 >= 2.6])
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

AC_DEFINE_UNQUOTED([HELP_FILE], "file://`pwd`/doc/aris/index.html", [The help HTML file path])

if test "$WIN32" = "yes"; then
   AC_DEFINE([WIN32])
fi

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
