#ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = \
		-I$(top_srcdir)/include \
		-I$(srcdir)/include \
		-I$(srcdir)/src

lib_LTLIBRARIES = libtccflacdec.la

FLACDEC_SRC = \
		src/TCC_FLAC_DEC.c \
		src/TCC_FLACDEC_bitreader.c \
		src/TCC_FLACDEC_decode.c \
		src/TCC_FLACDEC_fixed.c \
		src/TCC_FLACDEC_lpc.c \
		src/TCC_FLACDEC_stream_decoder.c

libtccflacdec_la_SOURCES = \
		$(FLACDEC_SRC) \
		flacdec.c

EXTRA_DIST = \
		$(srcdir)/include/*.h
