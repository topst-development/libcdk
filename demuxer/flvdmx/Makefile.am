lib_LTLIBRARIES = libtccflvdmx.la

libtccflvdmx_la_SOURCES = flvdmx.c

if IS_64BIT
libtccflvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_FLVDMX_LINUX_V0.3.34
else
if USE_HARDFP
libtccflvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_HF_FLVDMX_LINUX_V0.3.34
else
if IS_TCC803X
libtccflvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_FLVDMX_LINUX_V0.3.34
else
libtccflvdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_FLVDMX_LINUX_V0.3.34
endif
endif
endif

libtccflvdmx_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
