AC_PREREQ([2.65])
AC_INIT([Telechips demuxer], [1.0.0], [www.telechips.com])
AM_INIT_AUTOMAKE( [foreign -Wall -Werror] )

# Check if header files are present
AC_CONFIG_SRCDIR([include/tcc_demux_interface.h])

#telechips processor setup
AC_ARG_VAR(PROCESSOR, telechips processor setting (tcc892x, tcc8925s, tcc893x, tcc8935s, tcc896x, tcc897x, tcc898x, tcc899x, tcc802x, tcc803x or simulate))
AC_SUBST(PROCESSOR)

# Architecture setup
AC_ARG_VAR(TUNE_ARCH, if 64bit machine (aarch64 or null))
AC_SUBST(TUNE_ARCH)

# Float setup
AC_ARG_VAR(FLOAT_ABI, float compile option (hard, softfp or null))
AC_SUBST(FLOAT_ABI)

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.

# Check for libtool
AM_PROG_AR
AM_PROG_LIBTOOL

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([select])

# Set to 'm4' the directory where the extra autoconf macros are stored
AC_CONFIG_MACRO_DIR([m4])

#Check whether APE Demuxer library have been requested
AC_ARG_ENABLE(
    [apedmx],
    [AC_HELP_STRING(
        [--disable-apedmx],
        [whether to disable the apedmx library])],
    [with_apedmx=$enableval],
    [with_apedmx=yes])

#Check whether ASF Demuxer library have been requested
AC_ARG_ENABLE(
    [asfdmx],
    [AC_HELP_STRING(
        [--enable-asfdmx],
        [whether to enable the asfdmx library])],
    [with_asfdmx=$enableval],
    [with_asfdmx=no])

#Check whether AVI Demuxer library have been requested
AC_ARG_ENABLE(
    [avidmx],
    [AC_HELP_STRING(
        [--disable-avidmx],
        [whether to enable the avidmx library])],
    [with_avidmx=$enableval],
    [with_avidmx=yes])

#Check whether FLV Demuxer library have been requested
AC_ARG_ENABLE(
    [flvdmx],
    [AC_HELP_STRING(
        [--disable-flvdmx],
        [whether to disable the flvdmx library])],
    [with_flvdmx=$enableval],
    [with_flvdmx=yes])

#Check whether MKV Demuxer library have been requested
AC_ARG_ENABLE(
    [mkvdmx],
    [AC_HELP_STRING(
        [--disable-mkvdmx],
        [whether to disable the mkvdmx library])],
    [with_mkvdmx=$enableval],
    [with_mkvdmx=yes])

#Check whether MP4 Demuxer library have been requested
AC_ARG_ENABLE(
    [mp4dmx],
    [AC_HELP_STRING(
        [--disable-mp4dmx],
        [whether to enable the mp4dmx library])],
    [with_mp4dmx=$enableval],
    [with_mp4dmx=yes])

#Check whether MPG Demuxer library have been requested
AC_ARG_ENABLE(
    [mpgdmx],
    [AC_HELP_STRING(
        [--disable-mpgdmx],
        [whether to disable the mpgdmx library])],
    [with_mpgdmx=$enableval],
    [with_mpgdmx=yes])

#Check whether OGG Demuxer library have been requested
AC_ARG_ENABLE(
    [oggdmx],
    [AC_HELP_STRING(
        [--disable-oggdmx],
        [whether to disable the oggdmx library])],
    [with_oggdmx=$enableval],
    [with_oggdmx=yes])

#Check whether TS Demuxer library have been requested
AC_ARG_ENABLE(
    [tsdmx],
    [AC_HELP_STRING(
        [--disable-tsdmx],
        [whether to disable the tsdmx library])],
    [with_tsdmx=$enableval],
    [with_tsdmx=yes])

AC_CONFIG_FILES([Makefile
                 apedmx/Makefile
                 asfdmx/Makefile
                 avidmx/Makefile
                 flvdmx/Makefile
                 mkvdmx/Makefile
                 mp4dmx/Makefile
                 mpgdmx/Makefile
                 oggdmx/Makefile
                 tsdmx/Makefile])

AM_CONDITIONAL([WITH_APEDMX], [test x$with_apedmx = xyes])
AM_CONDITIONAL([WITH_ASFDMX], [test x$with_asfdmx = xyes])
AM_CONDITIONAL([WITH_AVIDMX], [test x$with_avidmx = xyes])
AM_CONDITIONAL([WITH_FLVDMX], [test x$with_flvdmx = xyes])
AM_CONDITIONAL([WITH_MKVDMX], [test x$with_mkvdmx = xyes])
AM_CONDITIONAL([WITH_MP4DMX], [test x$with_mp4dmx = xyes])
AM_CONDITIONAL([WITH_MPGDMX], [test x$with_mpgdmx = xyes])
AM_CONDITIONAL([WITH_OGGDMX], [test x$with_oggdmx = xyes])
AM_CONDITIONAL([WITH_TSDMX], [test x$with_tsdmx = xyes])

AS_IF([test "x$PROCESSOR" = x], [PROCESSOR="simulate"])
AM_CONDITIONAL(IS_TCC802X, test "x$PROCESSOR" = "xtcc802x")
AM_CONDITIONAL(IS_TCC803X, test "x$PROCESSOR" = "xtcc803x")
AM_CONDITIONAL(IS_TCC899X, test "x$PROCESSOR" = "xtcc899x")
AM_CONDITIONAL(IS_TCC898X, test "x$PROCESSOR" = "xtcc898x")
AM_CONDITIONAL(IS_TCC897X, test "x$PROCESSOR" = "xtcc897x")
AM_CONDITIONAL(IS_TCC896X, test "x$PROCESSOR" = "xtcc896x")
AM_CONDITIONAL(IS_TCC893X, test "x$PROCESSOR" = "xtcc893x")
AM_CONDITIONAL(IS_TCC8935S, test "x$PROCESSOR" = "xtcc8935s")
AM_CONDITIONAL(IS_TCC892X, test "x$PROCESSOR" = "xtcc892x")
AM_CONDITIONAL(IS_TCC8925S, test "x$PROCESSOR" = "xtcc8925s")
AM_CONDITIONAL(IS_SIMULATE, test "x$PROCESSOR" = "xsimulate")

AS_IF([test "x$TUNE_ARCH" = x], [TUNE_ARCH="null"])
AM_CONDITIONAL(IS_64BIT, test "x$TUNE_ARCH" = "x64bit")

AS_IF([test "x$FLOAT_ABI" = x], [FLOAT_ABI="null"])
AM_CONDITIONAL(USE_HARDFP, test "x$FLOAT_ABI" = "xhard")

AC_OUTPUT
