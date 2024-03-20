lib_LTLIBRARIES = libtccapedec.la

libtccapedec_la_SOURCES = apedec.c

if IS_64BIT
libtccapedec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_APEDEC_LINUX_V1.1.9
else
if USE_HARDFP
libtccapedec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_APEDEC_LINUX_V1.1.9_hf
else
if IS_TCC803X
libtccapedec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_APEDEC_LINUX_V1.1.9
else
libtccapedec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_APEDEC_LINUX_V1.1.9
endif
endif
endif
libtccapedec_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
