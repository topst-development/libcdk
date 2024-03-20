AC_PREREQ([2.65])
AC_INIT([Telechips audiocodec], [1.0.0], [www.telechips.com])
AM_INIT_AUTOMAKE( [foreign -Wall -Werror] )

# Check if header files are present
AC_CONFIG_SRCDIR([include/tcc_audio_interface.h])

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

#Check whether AAC Decoder library have been requested
AC_ARG_ENABLE(
    [aacdec],
    [AC_HELP_STRING(
        [--disable-aacdec],
        [whether to disable the aacdec library])],
    [with_aacdec=$enableval],
    [with_aacdec=yes])

#Check whether MP3 Decoder library have been requested
AC_ARG_ENABLE(
    [mp3dec],
    [AC_HELP_STRING(
        [--disable-mp3dec],
        [whether to disable the mp3dec library])],
    [with_mp3dec=$enableval],
    [with_mp3dec=yes])

#Check whether WAV Decoder have been requested
AC_ARG_ENABLE(
    [wavdec],
    [AC_HELP_STRING(
        [--disable-wavdec],
        [whether to disable the wavdec library])],
    [with_wavdec=$enableval],
    [with_wavdec=yes])

#Check whether AMRNB Decoder have been requested
AC_ARG_ENABLE(
    [amrnbdec],
    [AC_HELP_STRING(
        [--disable-amrnbdec],
        [whether to disable the amrnbdec library])],
    [with_amrnbdec=$enableval],
    [with_amrnbdec=yes])

#Check whether FLAC Decoder library have been requested
AC_ARG_ENABLE(
    [flacdec],
    [AC_HELP_STRING(
        [--disable-flacdec],
        [whether to disable the flacdec library])],
    [with_flacdec=$enableval],
    [with_flacdec=yes])

#Check whether APE Decoder library have been requested
AC_ARG_ENABLE(
    [apedec],
    [AC_HELP_STRING(
        [--disable-apedec],
        [whether to disable the apedec library])],
    [with_apedec=$enableval],
    [with_apedec=yes])

#Check whether AC3 Decoder library have been requested
AC_ARG_ENABLE(
    [ac3dec],
    [AC_HELP_STRING(
        [--enable-ac3dec],
        [whether to enable the ac3dec library, ac3 and ddp decoder can not be applied simultaneously])],
    [with_ac3dec=$enableval],
    [with_ac3dec=no])

#Check whether DDP Decoder library have been requested
AC_ARG_ENABLE(
    [ddpdec],
    [AC_HELP_STRING(
        [--enable-ddpdec],
        [whether to enable the ddpdec library, ac3 and ddp decoder can not be applied simultaneously])],
    [with_ddpdec=$enableval],
    [with_ddpdec=no])

#Check whether DTS Decoder library have been requested
AC_ARG_ENABLE(
    [dtsdec],
    [AC_HELP_STRING(
        [--enable-dtsdec],
        [whether to enable the dtsdec library])],
    [with_dtsdec=$enableval],
    [with_dtsdec=no])

#Check whether WMA Decoder library have been requested
AC_ARG_ENABLE(
    [wmadec],
    [AC_HELP_STRING(
        [--enable-wmadec],
        [whether to enable the wmadec library])],
    [with_wmadec=$enableval],
    [with_wmadec=no])

#Check whether OPUS Decoder library have been requested
AC_ARG_ENABLE(
    [opusdec],
    [AC_HELP_STRING(
        [--disable-opusdec],
        [whether to enable the opusdec library])],
    [with_opusdec=$enableval],
    [with_opusdec=yes])

if test "x$with_ac3dec" = "xyes" && test "x$with_ddpdec" = "xyes"; then
	with_ac3dec=no
	with_ddpdec=no
	AC_MSG_WARN([AC3 and DDP decoder can not be applied simultaneously!])
	AC_MSG_WARN([both library are all disabled!])
fi

AC_CONFIG_FILES([Makefile
                 aacdec/Makefile
                 aacdec/fdk_aac/Makefile
                 ac3dec/Makefile
                 apedec/Makefile
                 ddpdec/Makefile
                 dtsdec/Makefile
                 flacdec/Makefile
                 mp3dec/Makefile
                 wavdec/Makefile
                 amrnbdec/Makefile
                 wmadec/Makefile
                 opusdec/Makefile])

AM_CONDITIONAL([WITH_AACDEC], [test x$with_aacdec = xyes])
AM_CONDITIONAL([WITH_AC3DEC], [test x$with_ac3dec = xyes])
AM_CONDITIONAL([WITH_DDPDEC], [test x$with_ddpdec = xyes])
AM_CONDITIONAL([WITH_DTSDEC], [test x$with_dtsdec = xyes])
AM_CONDITIONAL([WITH_MP3DEC], [test x$with_mp3dec = xyes])
AM_CONDITIONAL([WITH_FLACDEC], [test x$with_flacdec = xyes])
AM_CONDITIONAL([WITH_WMADEC], [test x$with_wmadec = xyes])
AM_CONDITIONAL([WITH_APEDEC], [test x$with_apedec = xyes])
AM_CONDITIONAL([WITH_WAVDEC], [test x$with_wavdec = xyes])
AM_CONDITIONAL([WITH_AMRNBDEC], [test x$with_amrnbdec = xyes])
AM_CONDITIONAL([WITH_OPUSDEC], [test x$with_opusdec = xyes])

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
