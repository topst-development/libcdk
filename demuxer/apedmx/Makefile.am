lib_LTLIBRARIES = libtccapedmx.la

libtccapedmx_la_SOURCES = apedmx.c

if IS_64BIT
libtccapedmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_APEDMX_LINUX_V2.01.01
else
if USE_HARDFP
libtccapedmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_APEDMX_LINUX_V2.01.01_hf
else
if IS_TCC803X
libtccapedmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_APEDMX_LINUX_V2.01.01
else
libtccapedmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_APEDMX_LINUX_V2.01.01
endif
endif
endif

libtccapedmx_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
