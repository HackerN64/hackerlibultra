# One of:
# libgultra_rom, libgultra_d, libgultra
# libultra_rom, libultra_d, libultra
TARGET ?= libgultra_rom
VERSION ?= L
CROSS ?= mips64-elf-
VERBOSE ?= 0

ifeq ($(VERBOSE), 0)
V=@
else
V=
endif

BUILD_ROOT := build
BUILD_DIR := $(BUILD_ROOT)/$(VERSION)/$(TARGET)
BUILD_AR := $(BUILD_DIR)/$(TARGET).a

WORKING_DIR := $(shell pwd)

CPP := cpp -P
AR := $(CROSS)ar

VERSION_DEFINE := -DBUILD_VERSION=VERSION_$(VERSION) -DBUILD_VERSION_STRING=\"2.0$(VERSION)\"

ifeq ($(findstring _d,$(TARGET)),_d)
DEBUGFLAG := -D_DEBUG
else
DEBUGFLAG := -DNDEBUG
endif

-include makefiles/modern_gcc.mk

ifeq ($(findstring _rom,$(TARGET)),_rom)
CPPFLAGS += -D_FINALROM
endif

SRC_DIRS := $(shell find src -type d)
C_FILES  := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES  := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

# Versions J and below used the C matrix math implementations
MGU_MATRIX_FILES := mtxcatf normalize scale translate
ifneq ($(filter $(VERSION),D E F G H I J),)
S_FILES := $(filter-out $(addprefix src/mgu/,$(MGU_MATRIX_FILES:=.s)),$(S_FILES))
else
C_FILES := $(filter-out $(addprefix src/gu/,$(MGU_MATRIX_FILES:=.c)),$(C_FILES))
endif

C_O_FILES := $(foreach f,$(C_FILES:.c=.o),$(BUILD_DIR)/$f)
S_O_FILES := $(foreach f,$(S_FILES:.s=.o),$(BUILD_DIR)/$f)
O_FILES   := $(S_O_FILES) $(C_O_FILES)

AR_OBJECTS := $(shell cat base/$(VERSION)/$(TARGET).txt)
# If the version and target doesn't have a text file yet, resort back to using the base archive to get objects

# Try to find a file corresponding to an archive file in src/ or the base directory, prioritizing src then the original file
AR_ORDER = $(foreach f,$(AR_OBJECTS),$(shell find $(BUILD_DIR)/src -iname $f -type f -print -quit))

$(shell mkdir -p src $(foreach dir,$(SRC_DIRS),$(BUILD_DIR)/$(dir)))

.PHONY: all clean distclean setup
all: $(BUILD_AR)

$(BUILD_AR): $(O_FILES)
	@printf "    [AR] $@\n"
	$(V)$(AR) rcs $@ $(AR_ORDER)

clean:
	$(RM) -rf $(BUILD_DIR)

distclean:
	$(RM) -rf extracted/ $(BUILD_ROOT)

GBIDEFINE := -DF3DEX_GBI

$(BUILD_DIR)/src/gu/parse_gbi.o: GBIDEFINE := -DF3D_GBI
$(BUILD_DIR)/src/gu/us2dex_emu.o: GBIDEFINE :=
$(BUILD_DIR)/src/gu/us2dex2_emu.o: GBIDEFINE :=
$(BUILD_DIR)/src/sp/sprite.o: GBIDEFINE := -DF3D_GBI
$(BUILD_DIR)/src/sp/spriteex.o: GBIDEFINE :=
$(BUILD_DIR)/src/sp/spriteex2.o: GBIDEFINE :=
$(BUILD_DIR)/src/voice/%.o: OPTFLAGS += -DLANG_JAPANESE -I$(WORKING_DIR)/src -I$(WORKING_DIR)/src/voice
$(BUILD_DIR)/src/voice/%.o: CC := $(WORKING_DIR)/tools/compile_sjis.py -D__CC=$(CC) -D__BUILD_DIR=$(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	@printf "    [CC] $<\n"
	$(V)$(CC) $(CFLAGS) $(MIPS_VERSION) $(CPPFLAGS) $(OPTFLAGS) $< $(IINC) -o $@
	$(V)tools/set_o32abi_bit.py $@
	$(V)$(CROSS)strip $@ -N asdasdasdasd
	$(V)$(CROSS)objcopy --remove-section .mdebug $@

$(BUILD_DIR)/%.o: %.s
	@printf "    [AS] $<\n"
	$(V)$(AS) $(ASFLAGS) $(MIPS_VERSION) $(CPPFLAGS) $(ASOPTFLAGS) $< $(IINC) -o $@
	$(V)tools/set_o32abi_bit.py $@
	$(V)$(CROSS)strip $@ -N asdasdasdasd
	$(V)$(CROSS)objcopy --remove-section .mdebug $@

# Disable built-in rules
.SUFFIXES:
print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
