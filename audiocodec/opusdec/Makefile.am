lib_LTLIBRARIES = libtccopusdec.la

libtccopusdec_la_SOURCES = opusdec.c

if IS_64BIT
libtccopusdec_la_LDFLAGS = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_OPUSDEC_LINUX_V1.1.3
else
if USE_HARDFP
libtccopusdec_la_LDFLAGS = -L$(srcdir)/lib \
						-lTCC_ARMv7_OPUSDEC_LINUX_V1.1.3_HF
else
if IS_TCC803X
libtccopusdec_la_LDFLAGS = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_OPUSDEC_LINUX_V1.1.3
else
libtccopusdec_la_LDFLAGS = -L$(srcdir)/lib \
						-lTCC_ARMv7_OPUSDEC_LINUX_V1.1.3
endif				
endif
endif				
libtccopusdec_la_CFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = lib
