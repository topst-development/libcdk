AC_PREREQ([2.65])
AC_INIT([Telechips videocodec], [1.0.0], [www.telechips.com])
AM_INIT_AUTOMAKE( [foreign -Wall -Werror] )

# Check if header files are present

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
AM_PROG_AR

# Checks for libraries.

# Check for libtool
AM_PROG_LIBTOOL

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([select])

# Set to 'm4' the directory where the extra autoconf macros are stored
AC_CONFIG_MACRO_DIR([m4])

#Check whether WMA Decoder library have been requested
AC_ARG_ENABLE(
    [wmv78dec],
    [AC_HELP_STRING(
        [--enable-wmv78dec],
        [whether to enable the wmadec library])],
    [with_wmv78dec=$enableval],
    [with_wmv78dec=no])

AC_CONFIG_FILES([Makefile
                 wmv78dec/Makefile])

AM_CONDITIONAL([WITH_WMV78DEC], [test x$with_wmv78dec = xyes])

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
