lib_LTLIBRARIES = libtccmpgdmx.la

libtccmpgdmx_la_SOURCES = mpgdmx.c

if IS_64BIT
libtccmpgdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MPGDMX_LINUX_V2.6.1
else
if USE_HARDFP
libtccmpgdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MPGDMX_LINUX_V2.6.1_hf
else
if IS_TCC803X
libtccmpgdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MPGDMX_LINUX_V2.6.1
else
libtccmpgdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MPGDMX_LINUX_V2.6.1
endif
endif
endif

libtccmpgdmx_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
