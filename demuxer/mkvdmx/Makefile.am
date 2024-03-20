lib_LTLIBRARIES = libtccmkvdmx.la

libtccmkvdmx_la_SOURCES = mkvdmx.c

if IS_64BIT
libtccmkvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MKVDMX_LINUX_V1.3.0.10
else
if USE_HARDFP
libtccmkvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MKVDMX_LINUX_V1.3.0.10_hf
else
if IS_TCC803X
libtccmkvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MKVDMX_LINUX_V1.3.0.10
else
libtccmkvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MKVDMX_LINUX_V1.3.0.10
endif
endif
endif

libtccmkvdmx_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
