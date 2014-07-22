dnl Process this file with autoconf to produce a configure script.

AC_INIT(libsdl_item, 0.1.0)
dnl Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE

dnl AC_CONFIG_MACRO_DIR([./])

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

AC_PROG_RANLIB

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AM_PROG_CC_C_O

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h )


dnl jm_CHECK_TYPE_STRUCT_UTIMBUF
AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS(mkfifo)
AC_CHECK_FUNC(mknod)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.
PKG_CHECK_MODULES(LIBAVFORMAT, [libavformat >= 53.21.1])
PKG_CHECK_MODULES(LIBAVCODEC, [libavcodec >= 53.35.0])
PKG_CHECK_MODULES(LIBAVUTIL, [libavutil])
dnl PKG_CHECK_MODULES(LIBAVFILTER, [libavfilter >= 2.15.0])
PKG_CHECK_MODULES(LIBSWSCALE, [libswscale >= 2.1.0])
PKG_CHECK_MODULES(LIBSDL2_TTF, [SDL2_ttf >= 2.0.12])
PKG_CHECK_MODULES(LIBPNG, [libpng])

CFLAGS+="$SDL2_CFLAGS"
CFLAGS+="$LIBAVFORMAT_CFLAGS"
CFLAGS+="$LIBAVCODEC_CFLAGS"
CFLAGS+="$LIBSWSCALE_CFLAGS"
CFLAGS+="$LIBSDL2_TTF_CFLAGS"
CFLAGS+="$LIBAVUTIL_CFLAGS"
CFLAGS+="$LIBPNG_CFLAGS"

ADD_LIB="$SDL2_LIBS"
ADD_LIB+="$LIBAVFORMAT_LIBS"
ADD_LIB+="$LIBAVCODEC_LIBS"
ADD_LIB+="$LIBSWSCALE_LIBS"
ADD_LIB+="$LIBSDL2_TTF_LIBS"
ADD_LIB+="$LIBAVUTIL_LIBS"
ADD_LIB+="$LIBPNG_LIBS"
ADD_LIB+="-lgif"

AC_SUBST(SET_MAKE)
AC_SUBST(ADD_LIB)

dnl expand ${datadir} and write it out
conftemp="${datadir}"
eval conftemp=\"${conftemp}\"
eval conftemp=\"${conftemp}\"
AC_DEFINE_UNQUOTED(DATADIR, "${conftemp}",[Default path for data files])

AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(DATADIR)
AC_SUBST(LIBCONFIG_CFLAGS)
AC_SUBST(LIBCONFIG_LIBS)
AC_SUBST(LIBSDL2_CFLAGS)
AC_SUBST(LIBSDL2_LIBS)

AC_OUTPUT(Makefile)
