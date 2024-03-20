#########################################################
#
#	arm eabi toolchain 
#
#########################################################

#########################################################
#	Setting Command
#########################################################
RM := -/bin/rm -f
CP := -/bin/cp
MV := -/bin/mv
MKDIR := -/bin/mkdir -p
ECHO := echo
FIND := find
AWK := gawk
TAR := tar
GREP := grep
PERL := /usr/bin/perl
RMDIR := -/bin/rmdir -p
MAKEDEP := makedepend

TOOLCHAINPATH=C:/SysGCC/arm-elf
#TOOLCHAINPATH=/cygdrive/c/SysGCC/arm-elf

DEFSYM_ASM=defsym

#########################################################
#	Setting Compile
#########################################################

PRE		= $(TOOLCHAINPATH)/bin/arm-elf

CC			= $(PRE)-gcc
CXX			= $(PRE)-g++

ARMCC			= $(PRE)-gcc
ARMASM			= $(PRE)-as
AR				= $(PRE)-ar
ARMLINK			= $(PRE)-ld
ARMNM			= $(PRE)-nm
ARMSIZE			= $(PRE)-size
ARMOBJDUMP		= $(PRE)-objdump
ARMOBJCOPY		= $(PRE)-objcopy

#########################################################
#	Setting Location of Compiler
#########################################################
#INC_PATH_C=-I$(TOOLCHAINPATH)/lib/gcc/arm-elf/4.6.3/include
STDLIB_INC_PATH_C=-I$(TOOLCHAINPATH)/lib/gcc/arm-elf/4.6.3/include
LIBPATHS = $(TOOLCHAINPATH)/arm-elf/lib
LIBPATHS += $(TOOLCHAINPATH)/lib/gcc/arm-elf/4.6.3

#########################################################
#	Setting Debug Extention 
#########################################################
GDWARF	= -gdwarf-2
ifeq ($(TARGET_DEBUG),Y)
DBGSUBFIX=
else
DBGSUBFIX=
endif

#DBGLIBSUBFIX=$(DBGSUBFIX)


#CFLAGS			= -msoft-float -mcpu=$(TARGET_CPU) -Wall -fno-strict-aliasing -c $(INC_PATH_C) $(PRE_DEFINE_C) 
#CFLAGS			= -mcpu=$(TARGET_CPU) -marm -mno-thumb-interwork -msoft-float -Wall -fno-strict-aliasing -c $(INC_PATH_C) $(PRE_DEFINE_C) 
#CFLAGS			= -march=armv5 -mcpu=$(TARGET_CPU) -marm -mno-thumb-interwork -msoft-float -Wall -fno-strict-aliasing -c $(INC_PATH_C) $(PRE_DEFINE_C) 
#CFLAGS			= -march=armv5te -mcpu=arm9e -marm -mno-thumb-interwork -msoft-float -Wall -fno-strict-aliasing -c $(INC_PATH_C) $(PRE_DEFINE_C) 
CFLAGS			= -march=armv5te -mcpu=arm9e -marm -mno-thumb -mfpu=fpa -mfloat-abi=softfp -msoft-float -Wall -fno-strict-aliasing -c $(INC_PATH_C) $(PRE_DEFINE_C) -mlong-calls -w
#CFLAGS			= -mcpu=arm9 -marm -mno-thumb -mfpu=fpa -mfloat-abi=softfp -msoft-float -Wall -fno-strict-aliasing -c $(INC_PATH_C) $(PRE_DEFINE_C) -mlong-calls -w
CFLAGS			+=-mlong-calls

#AFLAGS 	    	= -mfpu=softfpa -mcpu=$(TARGET_CPU) -EL
AFLAGS 	    	= -march=armv5te -mcpu=arm9e -mfpu=softfpa -mfloat-abi=softfp  -EL
#AFLAGS 	    	= -mcpu=arm9 -mfpu=softfpa -mfloat-abi=softfp  -EL
ifeq ($(TARGET_DEBUG),Y)
AFLAGS 	    	+= -gdwarf-2
CFLAGS				+= -g -gdwarf-2
endif

CFLAGS	+= -O2

LFLAG_MAP		= -Map
LFLAG_SCATTER	= -T
LFLAGS			= -v --cref --no-strip-discarded -static

#$(OBJ_PATH)/%.o: %.c
#	@echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
##	@echo '[compile : $(@D)]'
##	@echo '[comple : \x1b[1;33m $^ \x1b[0m]'
#	@echo -e "[ compile : \x1b[1;33m $^ \x1b[0m]" >&2
#	$(Q)$(MKDIR) $(@D)
#	$(Q)$(ARMCC) $(CFLAGS) $(STDLIB_INC_PATH_C)
#
#$(OBJ_PATH)/%.o: %.s
#	@echo -e "[ assemble : \x1b[1;33m $(@D) \x1b[0m]" >&2
#	$(Q)$(MKDIR) $(@D)
#	$(Q)$(ARMASM) $(AFLAGS)


