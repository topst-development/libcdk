lib_LTLIBRARIES = libtccamrnbdec.la

libtccamrnbdec_la_SOURCES = amrnbdec.c

if IS_64BIT
libtccamrnbdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_AMRNBDEC_LINUX_V3.1.1
else
if USE_HARDFP
libtccamrnbdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_AMRNBDEC_LINUX_V3.1.1_hf
else
if IS_TCC803X
libtccamrnbdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_AMRNBDEC_LINUX_V3.1.1
else
libtccamrnbdec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_AMRNBDEC_LINUX_V3.1.1
endif
endif
endif
libtccamrnbdec_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
