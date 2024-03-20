#
#	Telechips RVCT3.1 Application(Launcher)  make define
#

#########################################################
#	1) Set Current Folder and Global Path 
#	
#########################################################
CURRENT_FOlDER		:=./
SRC_DIR		= $(CURRENT_FOlDER)/sim_source
ASM_DIR 	= ../../
AUDIO_HEADER_DIR = ../../../../include
HEADER_DIR	= $(CURRENT_FOlDER)/sim_header
HEADER_DIR1	= ../../libSYS/include

#########################################################
#	2) Set Current Lib Folder and PROG
#
#########################################################
CODEC_LIB_FOLDER := ../make_lib

#########################################################
#	3) Setting CFLAGS depend on the top config
#	   CFLAGS += -D
#########################################################
#CFLAGS += -DALSA_DETECTED


#########################################################
#	4) Setting Include
#	   INCLUDE += -I
#########################################################
INCLUDE += -I./
INCLUDE += -I$(HEADER_DIR)
#INCLUDE += -I$(HEADER_DIR1)
INCLUDE += -I$(AUDIO_HEADER_DIR)

#########################################################
#	5) Setting Library PATH 
#	   LDFLAGS += -L
#########################################################
LDPATH += -L./

#########################################################
#	6) Setting Library 
#	   LIBS+=-l
#
#   Change this item : select library type (.so or .a)
#########################################################
LIBS += $(CODEC_LIB_FOLDER)/$(LIB_NAME).a


#########################################################
#	7) Setting Source Files 
#	SOURCE_FILES =
##### Set All Files in the Path to Source Files #########
#SOURCE_FILES + = $(wildcard $(SOURCE_PATH) .c)
#
#   Change this item : add api source-codes
#########################################################
# C src
API_SOURCE_FILES += $(SRC_DIR)/aacenc_sim.c

