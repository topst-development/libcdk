lib_LTLIBRARIES = libtccpcmdec.la

libtccpcmdec_la_SOURCES = wavdec.c

if IS_64BIT
libtccpcmdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_WAVDEC_LINUX_V4.20.03
else
if USE_HARDFP
libtccpcmdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_WAVDEC_LINUX_V4.20.03_hf
else
if IS_TCC803X
libtccpcmdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_WAVDEC_LINUX_V4.20.03
else
libtccpcmdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_WAVDEC_LINUX_V4.20.03
endif
endif
endif
libtccpcmdec_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
