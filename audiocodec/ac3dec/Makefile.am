lib_LTLIBRARIES = libtccac3dec.la

libtccac3dec_la_SOURCES = ac3dec.c

libtccac3dec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC893x_AC3DEC_LINUX_V2.12.13 \
						-lTCC_NEON_OMXFFT_LINUX_V1.00.00

libtccac3dec_la_CFLAGS = -I$(top_srcdir)/include
