#ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = \
		-I$(top_srcdir)/include \
		-I$(srcdir)/include \
		-I$(srcdir)/src

#AM_CXXFLAGS = -fno-exceptions -fno-rtti

lib_LTLIBRARIES = libtccmp3dec.la

MP3DEC_SRC = \
		src/pvmp3_normalize.cpp \
		src/pvmp3_alias_reduction.cpp \
		src/pvmp3_crc.cpp \
		src/pvmp3_decode_header.cpp \
		src/pvmp3_decode_huff_cw.cpp \
		src/pvmp3_getbits.cpp \
		src/pvmp3_dequantize_sample.cpp \
		src/pvmp3_framedecoder.cpp \
		src/pvmp3_get_main_data_size.cpp \
		src/pvmp3_get_side_info.cpp \
		src/pvmp3_get_scale_factors.cpp \
		src/pvmp3_mpeg2_get_scale_data.cpp \
		src/pvmp3_mpeg2_get_scale_factors.cpp \
		src/pvmp3_mpeg2_stereo_proc.cpp \
		src/pvmp3_huffman_decoding.cpp \
		src/pvmp3_huffman_parsing.cpp \
		src/pvmp3_tables.cpp \
		src/pvmp3_imdct_synth.cpp \
		src/pvmp3_mdct_6.cpp \
		src/pvmp3_dct_6.cpp \
		src/pvmp3_poly_phase_synthesis.cpp \
		src/pvmp3_equalizer.cpp \
		src/pvmp3_seek_synch.cpp \
		src/pvmp3_stereo_proc.cpp \
		src/pvmp3_reorder.cpp \
		src/pvmp3_polyphase_filter_window.cpp \
		src/pvmp3_mdct_18.cpp \
		src/pvmp3_dct_9.cpp \
		src/pvmp3_dct_16.cpp

CDK_WRAPPER_SRC = \
		TCC_MP3_DEC.c

libtccmp3dec_la_SOURCES = \
		$(MP3DEC_SRC) \
		$(CDK_WRAPPER_SRC) \
		mp3dec.c

if IS_64BIT
libtccmp3dec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MP2DEC_LINUX_V4.07.08 \
                        -lTCC_ARMv8_MP1DEC_LINUX_V0.01.00
else
if USE_HARDFP
libtccmp3dec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MP2DEC_LINUX_V4.07.08HF  \
                        -lTCC_ARMv7_MP1DEC_LINUX_V0.01.00HF

else
if IS_TCC803X
libtccmp3dec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv8_MP2DEC_LINUX_V4.07.08 \
                        -lTCC_ARMv8_MP1DEC_LINUX_V0.01.00
else
libtccmp3dec_la_LIBADD = -L$(srcdir)/lib \
						-lTCC_ARMv7_MP2DEC_LINUX_V4.07.08 \
                        -lTCC_ARMv7_MP1DEC_LINUX_V0.01.00
endif
endif
endif

EXTRA_DIST = \
		$(srcdir)/NOTICE \
		$(srcdir)/patent_disclaimer.txt \
		$(srcdir)/src/*.h \
		$(srcdir)/src/asm/*.s \
		$(srcdir)/include/*.h
