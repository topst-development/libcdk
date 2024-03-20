if WITH_ASFDMX
    BUILD_ASFDMX = asfdmx
endif

if WITH_APEDMX
    BUILD_APEDMX = apedmx
endif

if WITH_AVIDMX
    BUILD_AVIDMX = avidmx
endif

if WITH_FLVDMX
    BUILD_FLVDMX = flvdmx
endif

if WITH_MKVDMX
    BUILD_MKVDMX = mkvdmx
endif

if WITH_MP4DMX
    BUILD_MP4DMX = mp4dmx
endif

if WITH_MPGDMX
    BUILD_MPGDMX = mpgdmx
endif

if WITH_OGGDMX
    BUILD_OGGDMX = oggdmx
endif

if WITH_TSDMX
    BUILD_TSDMX = tsdmx
endif

SUBDIRS = $(BUILD_APEDMX) $(BUILD_FLVDMX) $(BUILD_MPGDMX) $(BUILD_OGGDMX) $(BUILD_TSDMX)
SUBDIRS += $(BUILD_ASFDMX)
SUBDIRS += $(BUILD_AVIDMX) $(BUILD_MP4DMX) $(BUILD_MKVDMX)

EXTRA_DIST = include \
			tccdemuxer_build

ACLOCAL_AMFLAGS = -I m4
