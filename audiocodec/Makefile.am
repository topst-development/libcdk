if WITH_AACDEC
    BUILD_AACDEC = aacdec
endif

if WITH_MP3DEC
    BUILD_MP3DEC = mp3dec
endif

if WITH_AC3DEC
    BUILD_AC3DEC = ac3dec
endif

if WITH_DDPDEC
    BUILD_DDPDEC = ddpdec
endif

if WITH_DTSDEC
    BUILD_DTSDEC = dtsdec
endif


if WITH_WMADEC
    BUILD_WMADEC = wmadec
endif


if WITH_FLACDEC
    BUILD_FLACDEC = flacdec
endif

if WITH_APEDEC
    BUILD_APEDEC = apedec
endif

if WITH_WAVDEC
    BUILD_WAVDEC = wavdec
endif

if WITH_AMRNBDEC
    BUILD_AMRNBDEC = amrnbdec
endif

if WITH_OPUSDEC
    BUILD_OPUSDEC = opusdec
endif

SUBDIRS = $(BUILD_AACDEC) $(BUILD_MP3DEC) $(BUILD_AC3DEC) $(BUILD_DDPDEC) $(BUILD_DTSDEC) $(BUILD_WMADEC) $(BUILD_FLACDEC) $(BUILD_APEDEC) $(BUILD_WAVDEC) $(BUILD_AMRNBDEC) $(BUILD_OPUSDEC)

EXTRA_DIST = include \
			tccaudiocodec_build

ACLOCAL_AMFLAGS = -I m4
