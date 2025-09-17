# Makefile to build libultra

include util.mk

# Preprocessor definitions

WORKING_DIR := $(shell pwd)

DEFINES :=

SRC_DIRS :=

# Whether to hide commands or not
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
  V := @
endif

# Whether to colorize build messages
COLOR ?= 1

# TARGET 	   - selects the version of the library to build
#   libultra	 - standard library
#   libultra_d   - debug library
#   libultra_rom - final ROM library
TARGET ?= libultra_rom
$(eval $(call validate-option,TARGET,libultra libultra_d libultra_rom))

ifeq      ($(TARGET),libultra)
	OPT_FLAGS := -Os -ggdb3 -ffast-math -fno-unsafe-math-optimizations
	DEFINES += NDEBUG=1
else ifeq ($(TARGET),libultra_d)
	OPT_FLAGS := -Og -ggdb3 -ffast-math -fno-unsafe-math-optimizations
	DEFINES += _DEBUG=1
else ifeq ($(TARGET),libultra_rom)
	OPT_FLAGS := -Os -ggdb3 -ffast-math -fno-unsafe-math-optimizations
	DEFINES += NDEBUG=1
	DEFINES += _FINALROM=1
endif

# detect prefix for MIPS toolchain
ifneq ($(call find-command,mips64-elf-ld),)
  CROSS := mips64-elf-
else ifneq ($(call find-command,mips-n64-ld),)
  CROSS := mips-n64-
else ifneq ($(call find-command,mips64-ld),)
  CROSS := mips64-
else ifneq ($(call find-command,mips-linux-gnu-ld),)
  CROSS := mips-linux-gnu-
else ifneq ($(call find-command,mips64-linux-gnu-ld),)
  CROSS := mips64-linux-gnu-
else ifneq ($(call find-command,mips64-none-elf-ld),)
  CROSS := mips64-none-elf-
else ifneq ($(call find-command,mips-ld),)
  CROSS := mips-
else ifneq ($(call find-command,mips-suse-linux-ld ),)
  CROSS := mips-suse-linux-
else
  $(error Unable to detect a suitable MIPS toolchain installed)
endif

ifeq ($(filter clean,$(MAKECMDGOALS)),)
  $(info ==== Build Options ====)
  $(info Version:        $(TARGET))
  $(info =======================)
endif

#==============================================================================#
# Target Executable and Sources                                                #
#==============================================================================#
BUILD_DIR_BASE := build
# BUILD_DIR is the location where all build artifacts are placed
BUILD_DIR      := $(BUILD_DIR_BASE)/$(TARGET)
LIB            := $(BUILD_DIR)/$(TARGET).a

# Directories containing source files
SRC_DIRS += $(shell find src -type d)

C_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

# Object files
O_FILES := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
           $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o))

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d)

#==============================================================================#
# Compiler Options                                                             #
#==============================================================================#

AS        := $(CROSS)as
CC        := $(CROSS)gcc
CPP       := $(CROSS)cpp
LD        := $(CROSS)ld
AR        := $(CROSS)ar

# Do NOT depend on system-installed headers! If you need to make a header change,
# test it in your source first!
INCLUDE_DIRS += $(WORKING_DIR)/include $(WORKING_DIR)/include/PR $(WORKING_DIR)/include/libc $(BUILD_DIR) $(BUILD_DIR)/include $(WORKING_DIR)/src $(WORKING_DIR)

GBIDEFINE := -DF3DEX_GBI_2

C_DEFINES = $(foreach d,$(DEFINES),-D$(d))
DEF_INC_CFLAGS = $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(C_DEFINES)

WARNINGS := -Wall -Wextra -Wno-format-security -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-builtin-declaration-mismatch
WARNINGS += -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-implicit-function-declaration # TODO: Try adjusting code to remove these
CFLAGS := -G 0 -c -nostdinc -march=vr4300 -mfix4300 -mabi=32 -mips3 -mno-abicalls -mdivide-breaks -fno-PIC -fno-common -ffreestanding -fbuiltin -fno-builtin-sinf -fno-builtin-cosf -funsigned-char $(WARNINGS)
CFLAGS += -fno-strict-aliasing # TODO: Try adjusting code to remove this
CFLAGS += -D_MIPS_SZLONG=32 -D__USE_ISOC99 $(C_DEFINES) $(DEF_INC_CFLAGS)

# C preprocessor flags
CPPFLAGS := -P -Wno-trigraphs $(DEF_INC_CFLAGS)

# tools
PRINT = printf

ifeq ($(COLOR),1)
NO_COL  := \033[0m
RED     := \033[0;31m
GREEN   := \033[0;32m
BLUE    := \033[0;34m
YELLOW  := \033[0;33m
BLINK   := \033[33;5m
endif

# Common build print status function
define print
  @$(PRINT) "$(GREEN)$(1) $(YELLOW)$(2)$(GREEN) -> $(BLUE)$(3)$(NO_COL)\n"
endef

#==============================================================================#
# Main Targets                                                                 #
#==============================================================================#

# Default target
default: $(LIB)

clean:
	$(RM) -r $(BUILD_DIR_BASE)

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

# Make sure build directory exists before compiling anything
$(shell mkdir -p $(ALL_DIRS))

$(BUILD_DIR)/src/voice/%.o: CFLAGS += -I$(WORKING_DIR)/src/voice
$(BUILD_DIR)/src/voice/%.o: DEFINES += LANG_JAPANESE=1
$(BUILD_DIR)/src/gu/parse_gbi.o: GBIDEFINE := -DF3D_GBI
$(BUILD_DIR)/src/gu/us2dex_emu.o: GBIDEFINE :=
$(BUILD_DIR)/src/gu/us2dex2_emu.o: GBIDEFINE :=
$(BUILD_DIR)/src/sp/sprite.o: GBIDEFINE := -DF3D_GBI
$(BUILD_DIR)/src/sp/spriteex.o: GBIDEFINE :=
$(BUILD_DIR)/src/sp/spriteex2.o: GBIDEFINE :=

#==============================================================================#
# Compilation Recipes                                                          #
#==============================================================================#

# Compile C code
$(BUILD_DIR)/src/voice/%.o: src/voice/%.c
	$(call print,Compiling:,$<,$@)
	$(V)tools/compile_sjis.py -D__CC=$(CC) -D__BUILD_DIR=$(BUILD_DIR) -c $(CFLAGS) -MMD -MP -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(GBIDEFINE) -MMD -MP -o $@ $<
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(GBIDEFINE) -MMD -MP -o $@ $<

# Assemble assembly code
$(BUILD_DIR)/%.o: %.s
	$(call print,Assembling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(foreach i,$(INCLUDE_DIRS),-Wa,-I$(i)) -x assembler-with-cpp  -DMIPSEB -D_LANGUAGE_ASSEMBLY -D_ULTRA64 -MMD -MP -o $@ $<

# Creating final library file
$(LIB): $(O_FILES)
	@$(PRINT) "$(GREEN)Creating $(TARGET):  $(BLUE)$@ $(NO_COL)\n"
	$(V)$(AR) rcs -o $@ $(O_FILES)

all: $(BUILD_DIR_BASE)/libultra.a $(BUILD_DIR_BASE)/libultra_d.a $(BUILD_DIR_BASE)/libultra_rom.a

$(BUILD_DIR_BASE)/libultra.a: 
	$(V)$(MAKE) TARGET=libultra
	$(V)cp $(BUILD_DIR_BASE)/libultra/libultra.a $(BUILD_DIR_BASE)

$(BUILD_DIR_BASE)/libultra_d.a:
	$(V)$(MAKE) TARGET=libultra_d
	$(V)cp $(BUILD_DIR_BASE)/libultra_d/libultra_d.a $(BUILD_DIR_BASE)

$(BUILD_DIR_BASE)/libultra_rom.a:
	$(V)$(MAKE) TARGET=libultra_rom
	$(V)cp $(BUILD_DIR_BASE)/libultra_rom/libultra_rom.a $(BUILD_DIR_BASE)

.PHONY: clean default all install pkginstall
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
