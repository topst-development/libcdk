lib_LTLIBRARIES = libtccmp4dmx.la

libtccmp4dmx_la_SOURCES = mp4dmx.c

if IS_64BIT
libtccmp4dmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MP4DMX_LINUX_V3.10.00
else
if USE_HARDFP
libtccmp4dmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MP4DMX_LINUX_V3.10.00_hf
else
if IS_TCC803X
libtccmp4dmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MP4DMX_LINUX_V3.10.00
else
libtccmp4dmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MP4DMX_LINUX_V3.10.00
endif
endif
endif

libtccmp4dmx_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
