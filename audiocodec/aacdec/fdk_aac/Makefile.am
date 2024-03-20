#ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects
#FDK_AAC_VERSION=1:0:0

AM_CPPFLAGS = \
		-I$(top_srcdir)/include \
		-I$(srcdir)/libAACdec/include \
		-I$(srcdir)/libAACenc/include \
		-I$(srcdir)/libSBRdec/include \
		-I$(srcdir)/libSBRenc/include \
		-I$(srcdir)/libMpegTPDec/include \
		-I$(srcdir)/libMpegTPEnc/include \
		-I$(srcdir)/libSYS/include \
		-I$(srcdir)/libFDK/include \
		-I$(srcdir)/libPCMutils/include

AM_CXXFLAGS = -fno-exceptions -fno-rtti
#libtccaacdec_la_LINK = $(LINK) $(libtccaacdec_la_LDFLAGS)
# Mention a dummy pure C file to trigger generation of the $(LINK) variable
#nodist_EXTRA_libtccaacdec_la_SOURCES = dummy.c

fdk_aacincludedir = $(includedir)/fdk-aac
fdk_aacinclude_HEADERS = \
		$(srcdir)/libSYS/include/machine_type.h \
		$(srcdir)/libSYS/include/genericStds.h \
		$(srcdir)/libSYS/include/FDK_audio.h \
		$(srcdir)/libAACenc/include/aacenc_lib.h \
		$(srcdir)/libAACdec/include/aacdecoder_lib.h

lib_LTLIBRARIES = libtccaacdec.la

#libtccaacdec_la_LDFLAGS = -version-info @FDK_AAC_VERSION@ -no-undefined

AACDEC_SRC = \
		libAACdec/src/aacdec_drc.cpp \
		libAACdec/src/aacdec_hcr.cpp \
		libAACdec/src/aacdecoder.cpp \
		libAACdec/src/aacdec_pns.cpp \
		libAACdec/src/aac_ram.cpp \
		libAACdec/src/block.cpp \
		libAACdec/src/channelinfo.cpp \
		libAACdec/src/ldfiltbank.cpp \
		libAACdec/src/rvlcbit.cpp \
		libAACdec/src/rvlc.cpp \
		libAACdec/src/aacdec_hcr_bit.cpp \
		libAACdec/src/aacdec_hcrs.cpp \
		libAACdec/src/aacdecoder_lib.cpp \
		libAACdec/src/aacdec_tns.cpp \
		libAACdec/src/aac_rom.cpp \
		libAACdec/src/channel.cpp \
		libAACdec/src/conceal.cpp \
		libAACdec/src/pulsedata.cpp \
		libAACdec/src/rvlcconceal.cpp \
		libAACdec/src/stereo.cpp

AACENC_SRC = \
		libAACenc/src/aacenc.cpp \
		libAACenc/src/aacEnc_ram.cpp \
		libAACenc/src/band_nrg.cpp \
		libAACenc/src/block_switch.cpp \
		libAACenc/src/grp_data.cpp \
		libAACenc/src/metadata_main.cpp \
		libAACenc/src/pre_echo_control.cpp \
		libAACenc/src/quantize.cpp \
		libAACenc/src/tonality.cpp \
		libAACenc/src/aacEnc_rom.cpp \
		libAACenc/src/bandwidth.cpp \
		libAACenc/src/channel_map.cpp \
		libAACenc/src/intensity.cpp \
		libAACenc/src/ms_stereo.cpp \
		libAACenc/src/psy_configuration.cpp \
		libAACenc/src/sf_estim.cpp \
		libAACenc/src/transform.cpp \
		libAACenc/src/aacenc_lib.cpp \
		libAACenc/src/aacenc_tns.cpp \
		libAACenc/src/bit_cnt.cpp \
		libAACenc/src/chaosmeasure.cpp \
		libAACenc/src/line_pe.cpp \
		libAACenc/src/noisedet.cpp \
		libAACenc/src/psy_main.cpp \
		libAACenc/src/spreading.cpp \
		libAACenc/src/aacenc_pns.cpp \
		libAACenc/src/adj_thr.cpp \
		libAACenc/src/bitenc.cpp \
		libAACenc/src/dyn_bits.cpp \
		libAACenc/src/metadata_compressor.cpp \
		libAACenc/src/pnsparam.cpp \
		libAACenc/src/qc_main.cpp

FDK_SRC = \
		libFDK/src/autocorr2nd.cpp \
		libFDK/src/dct.cpp \
		libFDK/src/FDK_bitbuffer.cpp \
		libFDK/src/FDK_core.cpp \
		libFDK/src/FDK_crc.cpp \
		libFDK/src/FDK_hybrid.cpp \
		libFDK/src/FDK_tools_rom.cpp \
		libFDK/src/FDK_trigFcts.cpp \
		libFDK/src/fft.cpp \
		libFDK/src/fft_rad2.cpp \
		libFDK/src/fixpoint_math.cpp \
		libFDK/src/mdct.cpp \
		libFDK/src/qmf.cpp \
		libFDK/src/scale.cpp

MPEGTPDEC_SRC = \
		libMpegTPDec/src/tpdec_adif.cpp \
		libMpegTPDec/src/tpdec_adts.cpp \
		libMpegTPDec/src/tpdec_asc.cpp \
		libMpegTPDec/src/tpdec_drm.cpp \
		libMpegTPDec/src/tpdec_latm.cpp \
		libMpegTPDec/src/tpdec_lib.cpp

MPEGTPENC_SRC = \
		libMpegTPEnc/src/tpenc_adif.cpp \
		libMpegTPEnc/src/tpenc_adts.cpp \
		libMpegTPEnc/src/tpenc_asc.cpp \
		libMpegTPEnc/src/tpenc_latm.cpp \
		libMpegTPEnc/src/tpenc_lib.cpp

PCMUTILS_SRC = \
		libPCMutils/src/limiter.cpp \
		libPCMutils/src/pcmutils_lib.cpp

SBRDEC_SRC = \
		libSBRdec/src/env_calc.cpp \
		libSBRdec/src/env_dec.cpp \
		libSBRdec/src/env_extr.cpp \
		libSBRdec/src/huff_dec.cpp \
		libSBRdec/src/lpp_tran.cpp \
		libSBRdec/src/psbitdec.cpp \
		libSBRdec/src/psdec.cpp \
		libSBRdec/src/psdec_hybrid.cpp \
		libSBRdec/src/sbr_crc.cpp \
		libSBRdec/src/sbr_deb.cpp \
		libSBRdec/src/sbr_dec.cpp \
		libSBRdec/src/sbrdec_drc.cpp \
		libSBRdec/src/sbrdec_freq_sca.cpp \
		libSBRdec/src/sbrdecoder.cpp \
		libSBRdec/src/sbr_ram.cpp \
		libSBRdec/src/sbr_rom.cpp

SBRENC_SRC = \
		libSBRenc/src/bit_sbr.cpp \
		libSBRenc/src/env_bit.cpp \
		libSBRenc/src/fram_gen.cpp \
		libSBRenc/src/mh_det.cpp \
		libSBRenc/src/ps_bitenc.cpp \
		libSBRenc/src/ps_encode.cpp \
		libSBRenc/src/resampler.cpp \
		libSBRenc/src/sbr_encoder.cpp \
		libSBRenc/src/sbr_ram.cpp \
		libSBRenc/src/ton_corr.cpp \
		libSBRenc/src/code_env.cpp \
		libSBRenc/src/env_est.cpp \
		libSBRenc/src/invf_est.cpp \
		libSBRenc/src/nf_est.cpp \
		libSBRenc/src/ps_main.cpp \
		libSBRenc/src/sbrenc_freq_sca.cpp \
		libSBRenc/src/sbr_misc.cpp \
		libSBRenc/src/sbr_rom.cpp \
		libSBRenc/src/tran_det.cpp

SYS_SRC = \
		libSYS/src/cmdl_parser.cpp \
		libSYS/src/conv_string.cpp \
		libSYS/src/genericStds.cpp \
		libSYS/src/wav_file.cpp

CDK_WRAPPER_SRC = \
		TCAS_AAC_DEC.c

#libtccaacdec_la_SOURCES = \
#    $(AACDEC_SRC) $(AACENC_SRC) \
#    $(MPEGTPDEC_SRC) $(MPEGTPENC_SRC) \
#    $(SBRDEC_SRC) $(SBRENC_SRC) \
#    $(PCMUTILS_SRC) $(FDK_SRC) $(SYS_SRC)

# exclude encoder source
libtccaacdec_la_SOURCES = \
		$(AACDEC_SRC) \
		$(MPEGTPDEC_SRC) \
		$(SBRDEC_SRC) \
		$(PCMUTILS_SRC) $(FDK_SRC) $(SYS_SRC) \
		$(CDK_WRAPPER_SRC) \
		aacdec.c

EXTRA_DIST = \
		$(srcdir)/autogen.sh \
		$(srcdir)/MODULE_LICENSE_FRAUNHOFER \
		$(srcdir)/THIRD_PARTY_MODIFIED_VERSION_OF_THE_FRAUNHOFER_FDK_AAC \
		$(srcdir)/NOTICE \
		$(srcdir)/ChangeLog \
		$(srcdir)/ChangeLogByTelechips \
		$(srcdir)/Android.mk \
		$(srcdir)/fdk-aac.sym \
		$(srcdir)/Makefile.vc \
		$(srcdir)/documentation/*.pdf \
		$(srcdir)/libAACdec/src/*.h \
		$(srcdir)/libAACdec/src/arm/*.cpp \
		$(srcdir)/libAACenc/src/*.h \
		$(srcdir)/libSBRenc/src/*.h \
		$(srcdir)/libSBRenc/include/*.h \
		$(srcdir)/libSBRdec/src/*.h \
		$(srcdir)/libSBRdec/src/arm/*.cpp \
		$(srcdir)/libSBRdec/include/*.h \
		$(srcdir)/libSYS/include/*.h \
		$(srcdir)/libSYS/src/linux/*.cpp \
		$(srcdir)/libSYS/src/mips/*.cpp \
		$(srcdir)/libPCMutils/include/*.h \
		$(srcdir)/libMpegTPEnc/include/*.h \
		$(srcdir)/libMpegTPEnc/src/*.h \
		$(srcdir)/libMpegTPEnc/src/version \
		$(srcdir)/libMpegTPDec/include/*.h \
		$(srcdir)/libMpegTPDec/src/*.h \
		$(srcdir)/libMpegTPDec/src/version \
		$(srcdir)/libFDK/include/*.h \
		$(srcdir)/libFDK/include/aarch64/*.h \
		$(srcdir)/libFDK/include/arm/*.h \
		$(srcdir)/libFDK/include/mips/*.h \
		$(srcdir)/libFDK/include/ppc/*.h \
		$(srcdir)/libFDK/include/x86/*.h \
		$(srcdir)/libFDK/src/arm/*.cpp \
		$(srcdir)/libFDK/src/mips/*.cpp \
		$(srcdir)/win32/*.h
