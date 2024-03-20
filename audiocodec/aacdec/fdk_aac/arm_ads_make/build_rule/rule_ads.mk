#
#	Telechips ADS 1.2 or RVCT (2.2/3.1/4.0) make rules
#

#########################################################
#	Setting Compile
#########################################################
CROSS_COMPILE	=
LD			= $(CROSS_COMPILE)armlink
CC			= $(CROSS_COMPILE)armcc
CPP			= $(CROSS_COMPILE)armcpp
AR			= $(CROSS_COMPILE)armar -cru
ARMASM		= $(CROSS_COMPILE)armasm
STRIP		= $(CROSS_COMPILE)fromelf
COPY		= copy
REMOVE		= rm -rf

#########################################################
#	Setting CFLAGS/AFLAGS
#########################################################
CFLAGS			+= -cpu $(CPU_TYPE)
AFLAGS			+= -cpu $(CPU_TYPE)

ifeq ($(USE_VFP),Y)
CFLAGS			+= -fpu $(FPU_TYPE)
AFLAGS			+= -fpu $(FPU_TYPE)
endif

#########################################################
#	Error & Warning & Optimization
#########################################################
CFLAGS += -Ec

ifeq ($(DEBUG),Y)
CFLAGS += -g -O0
else
CFLAGS += -O2 -Otime --no_inlinemax
endif

#########################################################
#	Setting Include
#########################################################
#INCLUDE = -I./ -I/F:/data/ADS12/Include
#INCLUDE += -I./ -I/usr/include

#########################################################
#	Setting Library
#########################################################
#LDFLAGS += -L/usr/lib


#########################################################
#	Set Install Folder Name
#########################################################
#TCC_OUTPUT 	:=	TCC_Output

