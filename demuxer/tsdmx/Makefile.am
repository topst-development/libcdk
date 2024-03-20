lib_LTLIBRARIES = libtcctsdmx.la

libtcctsdmx_la_SOURCES = tsdmx.c

if IS_64BIT
libtcctsdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_TSDMX_LINUX_V1.7.41
else
if USE_HARDFP
libtcctsdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_HF_TSDMX_LINUX_V1.7.41
else
if IS_TCC803X
libtcctsdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_TSDMX_LINUX_V1.7.41
else
libtcctsdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_TSDMX_LINUX_V1.7.41
endif
endif
endif

libtcctsdmx_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
