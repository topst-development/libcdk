lib_LTLIBRARIES = libtccavidmx.la

libtccavidmx_la_SOURCES = avidmx.c

if IS_64BIT
libtccavidmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_AVIDMX_LINUX_V3.0.1.6
else
if USE_HARDFP
libtccavidmx_la_LIBADD = -L$(srcdir)/lib \
						 -lTCC_ARMv7_AVIDMX_LINUX_V3.0.1.6_hf
else
if IS_TCC803X
libtccavidmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_AVIDMX_LINUX_V3.0.1.6
else
libtccavidmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_AVIDMX_LINUX_V3.0.1.6
endif
endif
endif

libtccavidmx_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
