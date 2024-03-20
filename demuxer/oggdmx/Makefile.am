lib_LTLIBRARIES = libtccoggdmx.la

libtccoggdmx_la_SOURCES = oggdmx.c

if IS_64BIT
libtccoggdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_OGGDMX_LINUX_V0.00.62
else
if USE_HARDFP
libtccoggdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_OGGDMX_LINUX_V0.00.62_hf
else
if IS_TCC803X
libtccoggdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_ARM64_OGGDMX_LINUX_V0.00.62
else
libtccoggdmx_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_OGGDMX_LINUX_V0.00.62
endif
endif
endif

libtccoggdmx_la_CFLAGS = -I$(top_srcdir)/include
