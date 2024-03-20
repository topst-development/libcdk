#
#	Telechips  make define
#

#########################################################
#   1) Set Current Folder and Global Path
#
#########################################################
CURRENT_FOlDER	:=./

SRC_DIR		= ../..
AUDIO_HEADER_DIR = $(SRC_DIR)/../../include
AACDEC_HEADER_DIR	= $(SRC_DIR)/libAACdec/include
AACENC_HEADER_DIR	= $(SRC_DIR)/libAACenc/include 
PCM_HEADER_DIR	= $(SRC_DIR)/libPCMutils/include
FDK_HEADER_DIR	= $(SRC_DIR)/libFDK/include
SYS_HEADER_DIR	= $(SRC_DIR)/libSYS/include
TPD_HEADER_DIR	= $(SRC_DIR)/libMpegTPDec/include
TPE_HEADER_DIR	= $(SRC_DIR)/libMpegTPEnc/include
SBRDEC_HEADER_DIR	= $(SRC_DIR)/libSBRdec/include
SBRENC_HEADER_DIR	= $(SRC_DIR)/libSBRenc/include
CDK_HEADER_DIR	= $(SRC_DIR)

#########################################################
#	2) Setting CFLAGS depend on the top config
#	   CFLAGS += -D
#      Change this item : add preprocessor defininitions
#########################################################
#CFLAGS += -DASM_OPT

#########################################################
#	3) Setting Include
#	   INCLUDE += -I
#########################################################
INCLUDE += -I./
INCLUDE += -I$(AUDIO_HEADER_DIR)
INCLUDE += -I$(AACDEC_HEADER_DIR)
INCLUDE += -I$(AACENC_HEADER_DIR)
INCLUDE += -I$(PCM_HEADER_DIR)
INCLUDE += -I$(FDK_HEADER_DIR)
INCLUDE += -I$(SYS_HEADER_DIR)
INCLUDE += -I$(TPD_HEADER_DIR)
INCLUDE += -I$(TPE_HEADER_DIR)
INCLUDE += -I$(SBRDEC_HEADER_DIR)
INCLUDE += -I$(SBRENC_HEADER_DIR)

#########################################################
#	4) Setting Library PATH
#	   LDFLAGS += -L
#########################################################
LDPATH += -L./

#########################################################
#	5) Setting Library
#	   LIBS+=-l
#########################################################


#########################################################
#	6) Setting Source Files
#	   SOURCE_FILES =
##### Set All Files in the Path to Source Files #########
#SOURCE_FILES + = $(wildcard $(SOURCE_PATH) .c)
#
#   Change this item : add library source-codes
#########################################################

CDK_WRAPPER_SRC = \
	$(SRC_DIR)/TCAS_AAC_DEC.c \
	$(SRC_DIR)/TCAS_AAC_ENC.c

AACDEC_SRC = \
    $(SRC_DIR)/libAACdec/src/aacdec_drc.cpp \
    $(SRC_DIR)/libAACdec/src/aacdec_hcr.cpp \
    $(SRC_DIR)/libAACdec/src/aacdecoder.cpp \
    $(SRC_DIR)/libAACdec/src/aacdec_pns.cpp \
    $(SRC_DIR)/libAACdec/src/aac_ram.cpp \
    $(SRC_DIR)/libAACdec/src/block.cpp \
    $(SRC_DIR)/libAACdec/src/channelinfo.cpp \
    $(SRC_DIR)/libAACdec/src/ldfiltbank.cpp \
    $(SRC_DIR)/libAACdec/src/rvlcbit.cpp \
    $(SRC_DIR)/libAACdec/src/rvlc.cpp \
    $(SRC_DIR)/libAACdec/src/aacdec_hcr_bit.cpp \
    $(SRC_DIR)/libAACdec/src/aacdec_hcrs.cpp \
    $(SRC_DIR)/libAACdec/src/aacdecoder_lib.cpp \
    $(SRC_DIR)/libAACdec/src/aacdec_tns.cpp \
    $(SRC_DIR)/libAACdec/src/aac_rom.cpp \
    $(SRC_DIR)/libAACdec/src/channel.cpp \
    $(SRC_DIR)/libAACdec/src/conceal.cpp \
    $(SRC_DIR)/libAACdec/src/pulsedata.cpp \
    $(SRC_DIR)/libAACdec/src/rvlcconceal.cpp \
    $(SRC_DIR)/libAACdec/src/stereo.cpp

AACENC_SRC = \
		$(SRC_DIR)/libAACenc/src/aacenc.cpp \
		$(SRC_DIR)/libAACenc/src/aacEnc_ram.cpp \
		$(SRC_DIR)/libAACenc/src/band_nrg.cpp \
		$(SRC_DIR)/libAACenc/src/block_switch.cpp \
		$(SRC_DIR)/libAACenc/src/grp_data.cpp \
		$(SRC_DIR)/libAACenc/src/metadata_main.cpp \
		$(SRC_DIR)/libAACenc/src/pre_echo_control.cpp \
		$(SRC_DIR)/libAACenc/src/quantize.cpp \
		$(SRC_DIR)/libAACenc/src/tonality.cpp \
		$(SRC_DIR)/libAACenc/src/aacEnc_rom.cpp \
		$(SRC_DIR)/libAACenc/src/bandwidth.cpp \
		$(SRC_DIR)/libAACenc/src/channel_map.cpp \
		$(SRC_DIR)/libAACenc/src/intensity.cpp \
		$(SRC_DIR)/libAACenc/src/ms_stereo.cpp \
		$(SRC_DIR)/libAACenc/src/psy_configuration.cpp \
		$(SRC_DIR)/libAACenc/src/sf_estim.cpp \
		$(SRC_DIR)/libAACenc/src/transform.cpp \
		$(SRC_DIR)/libAACenc/src/aacenc_lib.cpp \
		$(SRC_DIR)/libAACenc/src/aacenc_tns.cpp \
		$(SRC_DIR)/libAACenc/src/bit_cnt.cpp \
		$(SRC_DIR)/libAACenc/src/chaosmeasure.cpp \
		$(SRC_DIR)/libAACenc/src/line_pe.cpp \
		$(SRC_DIR)/libAACenc/src/noisedet.cpp \
		$(SRC_DIR)/libAACenc/src/psy_main.cpp \
		$(SRC_DIR)/libAACenc/src/spreading.cpp \
		$(SRC_DIR)/libAACenc/src/aacenc_pns.cpp \
		$(SRC_DIR)/libAACenc/src/adj_thr.cpp \
		$(SRC_DIR)/libAACenc/src/bitenc.cpp \
		$(SRC_DIR)/libAACenc/src/dyn_bits.cpp \
		$(SRC_DIR)/libAACenc/src/metadata_compressor.cpp \
		$(SRC_DIR)/libAACenc/src/pnsparam.cpp \
		$(SRC_DIR)/libAACenc/src/qc_main.cpp

FDK_SRC = \
    $(SRC_DIR)/libFDK/src/autocorr2nd.cpp \
    $(SRC_DIR)/libFDK/src/dct.cpp \
    $(SRC_DIR)/libFDK/src/FDK_bitbuffer.cpp \
    $(SRC_DIR)/libFDK/src/FDK_core.cpp \
    $(SRC_DIR)/libFDK/src/FDK_crc.cpp \
    $(SRC_DIR)/libFDK/src/FDK_hybrid.cpp \
    $(SRC_DIR)/libFDK/src/FDK_tools_rom.cpp \
    $(SRC_DIR)/libFDK/src/FDK_trigFcts.cpp \
    $(SRC_DIR)/libFDK/src/fft.cpp \
    $(SRC_DIR)/libFDK/src/fft_rad2.cpp \
    $(SRC_DIR)/libFDK/src/fixpoint_math.cpp \
    $(SRC_DIR)/libFDK/src/mdct.cpp \
    $(SRC_DIR)/libFDK/src/qmf.cpp \
    $(SRC_DIR)/libFDK/src/scale.cpp

MPEGTPDEC_SRC = \
    $(SRC_DIR)/libMpegTPDec/src/tpdec_adif.cpp \
    $(SRC_DIR)/libMpegTPDec/src/tpdec_adts.cpp \
    $(SRC_DIR)/libMpegTPDec/src/tpdec_asc.cpp \
    $(SRC_DIR)/libMpegTPDec/src/tpdec_drm.cpp \
    $(SRC_DIR)/libMpegTPDec/src/tpdec_latm.cpp \
    $(SRC_DIR)/libMpegTPDec/src/tpdec_lib.cpp

MPEGTPENC_SRC = \
		$(SRC_DIR)/libMpegTPEnc/src/tpenc_adif.cpp \
		$(SRC_DIR)/libMpegTPEnc/src/tpenc_adts.cpp \
		$(SRC_DIR)/libMpegTPEnc/src/tpenc_asc.cpp \
		$(SRC_DIR)/libMpegTPEnc/src/tpenc_latm.cpp \
		$(SRC_DIR)/libMpegTPEnc/src/tpenc_lib.cpp

PCMUTILS_SRC = \
    $(SRC_DIR)/libPCMutils/src/limiter.cpp \
    $(SRC_DIR)/libPCMutils/src/pcmutils_lib.cpp

SBRDEC_SRC = \
    $(SRC_DIR)/libSBRdec/src/env_calc.cpp \
    $(SRC_DIR)/libSBRdec/src/env_dec.cpp \
    $(SRC_DIR)/libSBRdec/src/env_extr.cpp \
    $(SRC_DIR)/libSBRdec/src/huff_dec.cpp \
    $(SRC_DIR)/libSBRdec/src/lpp_tran.cpp \
    $(SRC_DIR)/libSBRdec/src/psbitdec.cpp \
    $(SRC_DIR)/libSBRdec/src/psdec.cpp \
    $(SRC_DIR)/libSBRdec/src/psdec_hybrid.cpp \
    $(SRC_DIR)/libSBRdec/src/sbr_crc.cpp \
    $(SRC_DIR)/libSBRdec/src/sbr_deb.cpp \
    $(SRC_DIR)/libSBRdec/src/sbr_dec.cpp \
    $(SRC_DIR)/libSBRdec/src/sbrdec_drc.cpp \
    $(SRC_DIR)/libSBRdec/src/sbrdec_freq_sca.cpp \
    $(SRC_DIR)/libSBRdec/src/sbrdecoder.cpp \
    $(SRC_DIR)/libSBRdec/src/sbr_ram.cpp \
    $(SRC_DIR)/libSBRdec/src/sbr_rom.cpp

SBRENC_SRC = \
		$(SRC_DIR)/libSBRenc/src/bit_sbr.cpp \
		$(SRC_DIR)/libSBRenc/src/env_bit.cpp \
		$(SRC_DIR)/libSBRenc/src/fram_gen.cpp \
		$(SRC_DIR)/libSBRenc/src/mh_det.cpp \
		$(SRC_DIR)/libSBRenc/src/ps_bitenc.cpp \
		$(SRC_DIR)/libSBRenc/src/ps_encode.cpp \
		$(SRC_DIR)/libSBRenc/src/resampler.cpp \
		$(SRC_DIR)/libSBRenc/src/sbr_encoder.cpp \
		$(SRC_DIR)/libSBRenc/src/sbr_ram.cpp \
		$(SRC_DIR)/libSBRenc/src/ton_corr.cpp \
		$(SRC_DIR)/libSBRenc/src/code_env.cpp \
		$(SRC_DIR)/libSBRenc/src/env_est.cpp \
		$(SRC_DIR)/libSBRenc/src/invf_est.cpp \
		$(SRC_DIR)/libSBRenc/src/nf_est.cpp \
		$(SRC_DIR)/libSBRenc/src/ps_main.cpp \
		$(SRC_DIR)/libSBRenc/src/sbrenc_freq_sca.cpp \
		$(SRC_DIR)/libSBRenc/src/sbr_misc.cpp \
		$(SRC_DIR)/libSBRenc/src/sbr_rom.cpp \
		$(SRC_DIR)/libSBRenc/src/tran_det.cpp

SYS_SRC = \
	$(SRC_DIR)/libSYS/src/genericStds.cpp
#    $(SRC_DIR)/libSYS/src/cmdl_parser.cpp \
#    $(SRC_DIR)/libSYS/src/conv_string.cpp \
#    $(SRC_DIR)/libSYS/src/genericStds.cpp \
#    $(SRC_DIR)/libSYS/src/wav_file.cpp

#########################################################
#	7) Setting OBJECTS Files
##OBJECTS =
##### Covert All Source Files to Objects files ##########
#OBJECTS += $(SOURCE_FILES:.c=.o)
#########################################################
OBJECTS += $(CDK_WRAPPER_SRC:.c=.o)
OBJECTS += $(AACDEC_SRC:.cpp=.o)
OBJECTS += $(AACENC_SRC:.cpp=.o)
OBJECTS += $(FDK_SRC:.cpp=.o)
OBJECTS += $(MPEGTPDEC_SRC:.cpp=.o)
OBJECTS += $(MPEGTPENC_SRC:.cpp=.o)
OBJECTS += $(PCMUTILS_SRC:.cpp=.o)
OBJECTS += $(SBRDEC_SRC:.cpp=.o)
OBJECTS += $(SBRENC_SRC:.cpp=.o)
OBJECTS += $(SYS_SRC:.cpp=.o)
