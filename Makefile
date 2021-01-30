SHELL				:= cmd.exe
export SHELL

TARGET_NAME			:= TeensyJBOD

BOARD_ID			:= TEENSY41
MCU					:= imxrt1062
CPU_SPEED			:= 600000000
USB_MODE			:= SERIAL

ARD_LIBS_BASE		:= arduino_lib
LIBS_BASE			:= lib

ARD_LIBS		    := SPI NativeEthernet FNET TeensyThreads Wire TeensyID

CORE_BASE			:= ./teensy4
GCC_BASE      		:= C:\PROGRA~2\Arduino\hardware\tools\arm\bin
UPL_PJRC_B    		:= C:\PROGRA~2\Arduino\hardware\tools

#******************************************************************************
# Flags and Defines
#******************************************************************************
FLAGS_CPU   := -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16
FLAGS_OPT   := -O2
FLAGS_COM   := -g -Wall -ffunction-sections -fdata-sections -nostdlib -MMD
FLAGS_LSP   := 

FLAGS_CPP   := -std=gnu++14 -fno-exceptions -fpermissive -fno-rtti -fno-threadsafe-statics -felide-constructors -Wno-error=narrowing
FLAGS_C     := 
FLAGS_S     := -x assembler-with-cpp
FLAGS_LD    := -Wl,--print-memory-usage,--gc-sections,--relax,--defsym=__rtc_localtime=$(shell powershell [int](Get-Date -UFormat +%s)[0]) -T$(CORE_BASE)/imxrt1062_t41.ld

LIBS        := -larm_cortexM7lfsp_math -lm -lstdc++

DEFINES     := -D__IMXRT1062__ -DTEENSYDUINO=153 -DARDUINO_TEENSY41 -DARDUINO=10813
DEFINES     += -DF_CPU=$(CPU_SPEED) -DUSB_$(USB_MODE) -DLAYOUT_US_ENGLISH

CPP_FLAGS   := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_CPP)
C_FLAGS     := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_C)
S_FLAGS     := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_COM) $(DEFINES) $(FLAGS_S)
LD_FLAGS    := $(FLAGS_CPU) $(FLAGS_OPT) $(FLAGS_LSP) $(FLAGS_LD)
AR_FLAGS    := rcs
NM_FLAGS    := --numeric-sort --defined-only --demangle --print-size

#******************************************************************************
# Colors
#******************************************************************************
COL_CORE    := [38;2;187;206;251m
COL_LIB     := [38;2;206;244;253m
COL_SRC     := [38;2;100;149;237m
COL_LINK    := [38;2;255;255;202m
COL_ERR     := [38;2;255;159;159m
COL_OK      := [38;2;179;255;179m
COL_RESET   := [0m

#******************************************************************************
# Folders and Files
#******************************************************************************
USR_SRC         := src
LIB_SRC         := lib
CORE_SRC        := $(CORE_BASE)

BIN             := ./build
USR_BIN         := $(BIN)/src
CORE_BIN        := $(BIN)/core
LIB_BIN         := $(BIN)/lib
ARD_LIB_BIN     := $(BIN)/arduino_lib
CORE_LIB        := $(BIN)/core.a
TARGET_HEX      := $(BIN)/$(TARGET_NAME).hex
TARGET_ELF      := $(BIN)/$(TARGET_NAME).elf
TARGET_LST      := $(BIN)/$(TARGET_NAME).lst
TARGET_SYM      := $(BIN)/$(TARGET_NAME).sym

#******************************************************************************
# BINARIES
#******************************************************************************
CC              := $(GCC_BASE)/arm-none-eabi-gcc
CXX             := $(GCC_BASE)/arm-none-eabi-g++
AR              := $(GCC_BASE)/arm-none-eabi-gcc-ar
NM              := $(GCC_BASE)/arm-none-eabi-gcc-nm
SIZE            := $(GCC_BASE)/arm-none-eabi-size
OBJDUMP         := $(GCC_BASE)/arm-none-eabi-objdump
OBJCOPY         := $(GCC_BASE)/arm-none-eabi-objcopy
UPL_PJRC        := "$(UPL_PJRC_B)/teensy_post_compile" -test -file=$(TARGET_NAME) -path=$(BIN) -tools="$(UPL_PJRC_B)" -board=$(BOARD_ID) -reboot

#******************************************************************************
# Source and Include Files
#******************************************************************************
# Recursively create list of source and object files in USR_SRC and CORE_SRC
# and corresponding subdirectories.
# The function rwildcard is taken from http://stackoverflow.com/a/12959694)

rwildcard =$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

#User Sources -----------------------------------------------------------------
USR_C_FILES     := $(call rwildcard,$(USR_SRC)/,*.c)
USR_CPP_FILES   := $(call rwildcard,$(USR_SRC)/,*.cpp)
USR_S_FILES     := $(call rwildcard,$(USR_SRC)/,*.S)
USR_OBJ         := $(USR_S_FILES:$(USR_SRC)/%.S=$(USR_BIN)/%.o) $(USR_C_FILES:$(USR_SRC)/%.c=$(USR_BIN)/%.o) $(USR_CPP_FILES:$(USR_SRC)/%.cpp=$(USR_BIN)/%.o)

# Core library sources --------------------------------------------------------
CORE_CPP_FILES  := $(call rwildcard,$(CORE_SRC)/,*.cpp)
CORE_C_FILES    := $(call rwildcard,$(CORE_SRC)/,*.c)
CORE_S_FILES    := $(call rwildcard,$(CORE_SRC)/,*.S)
CORE_OBJ        := $(CORE_S_FILES:$(CORE_SRC)/%.S=$(CORE_BIN)/%.o) $(CORE_C_FILES:$(CORE_SRC)/%.c=$(CORE_BIN)/%.o) $(CORE_CPP_FILES:$(CORE_SRC)/%.cpp=$(CORE_BIN)/%.o)

# User library sources (see https://github.com/arduino/arduino/wiki/arduino-ide-1.5:-library-specification)
ARD_LIB_DIRS    := $(foreach d, $(ARD_LIBS), $(ARD_LIBS_BASE)/$d $(ARD_LIBS_BASE)/$d/utility ) # base and /utility
ARD_LIB_DIRS    += $(foreach d, $(ARD_LIBS), $(ARD_LIBS_BASE)/$d/src $(dir $(call rwildcard,$(ARD_LIBS_BASE)/$d/src/,*/.))) # src and all subdirs of base

ARD_LIB_CPP     := $(foreach d, $(ARD_LIB_DIRS),$(call wildcard,$d/*.cpp))
ARD_LIB_C       := $(foreach d, $(ARD_LIB_DIRS),$(call wildcard,$d/*.c))
ARD_LIB_S       := $(foreach d, $(ARD_LIB_DIRS),$(call wildcard,$d/*.S))

ARD_LIB_OBJ     := $(ARD_LIB_CPP:$(ARD_LIBS_BASE)/%.cpp=$(ARD_LIB_BIN)/%.o)
ARD_LIB_OBJ     += $(ARD_LIB_C:$(ARD_LIBS_BASE)/%.c=$(ARD_LIB_BIN)/%.o)
ARD_LIB_OBJ     += $(ARD_LIB_S:$(ARD_LIBS_BASE)/%.S=$(ARD_LIB_BIN)/%.o)

# User library sources
USR_C_FILES     := $(call rwildcard,$(USR_SRC)/,*.c)
USR_CPP_FILES   := $(call rwildcard,$(USR_SRC)/,*.cpp)
USR_S_FILES     := $(call rwildcard,$(USR_SRC)/,*.S)
USR_OBJ         := $(USR_S_FILES:$(USR_SRC)/%.S=$(USR_BIN)/%.o) $(USR_C_FILES:$(USR_SRC)/%.c=$(USR_BIN)/%.o) $(USR_CPP_FILES:$(USR_SRC)/%.cpp=$(USR_BIN)/%.o)

# Includes -------------------------------------------------------------
INCLUDE         := -I./$(USR_SRC) -I./$(LIB_SRC) -I$(CORE_SRC) 
INCLUDE         += $(foreach d, $(ARD_LIB_DIRS), -I$d)

# Generate directories --------------------------------------------------------
DIRECTORIES     := $(sort $(dir $(CORE_OBJ) $(USR_OBJ) $(LIB_OBJ) $(ARD_LIB_OBJ)))
generateDirs    := $(foreach d, $(DIRECTORIES), $(shell if not exist "$d" mkdir "$d"))

#$(info dirs: $(DIRECTORIES))

#******************************************************************************
# Rules:
#******************************************************************************

.PHONY: directories all rebuild upload uploadTy uploadCLI clean cleanUser cleanCore cleanLib cleanArdLib

all:  $(TARGET_LST) $(TARGET_SYM) $(TARGET_HEX)

rebuild: cleanUser all

clean: cleanUser cleanUserLib cleanCore cleanArdLib

upload: all
	@$(UPL_PJRC)

# Core library ----------------------------------------------------------------
$(CORE_BIN)/%.o: $(CORE_SRC)/%.S
	@echo $(COL_CORE)CORE [ASM] $(notdir $<) $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_BIN)/%.o: $(CORE_SRC)/%.c
	@echo $(COL_CORE)CORE [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_BIN)/%.o: $(CORE_SRC)/%.cpp
	@echo $(COL_CORE)CORE [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o $@ -c $<

$(CORE_LIB) : $(CORE_OBJ)
	@echo $(COL_LINK)CORE [AR] $@ $(COL_ERR)
	@$(AR) $(AR_FLAGS) $@ $^
	@echo $(COL_OK)Teensy core built successfully &&echo.

# Arduino Libraries -------------------------------------------------------------
$(ARD_LIB_BIN)/%.o: $(ARD_LIBS_BASE)/%.S
	@echo $(COL_LIB)ARD_LIB [ASM] $(notdir $<) $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o $@ -c $<

$(ARD_LIB_BIN)/%.o: $(ARD_LIBS_BASE)/%.cpp
	@echo $(COL_LIB)ARD_LIB [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o $@ -c $<

$(ARD_LIB_BIN)/%.o: $(ARD_LIBS_BASE)/%.c
	@echo $(COL_LIB)ARD_LIB [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o $@ -c $<

# Handle user sources ---------------------------------------------------------
$(USR_BIN)/%.o: $(USR_SRC)/%.S
	@echo $(COL_SRC)USER [ASM] $< $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o "$@" -c $<

$(USR_BIN)/%.o: $(USR_SRC)/%.c
	@echo $(COL_SRC)USER [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o "$@" -c $<

$(USR_BIN)/%.o: $(USR_SRC)/%.cpp
	@echo $(COL_SRC)USER [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<

# Handle user library sources ---------------------------------------------------------
$(LIB_BIN)/%.o: $(LIB_SRC)/%.S
	@echo $(COL_SRC)LIB [ASM] $< $(COL_ERR)
	@"$(CC)" $(S_FLAGS) $(INCLUDE) -o "$@" -c $<

$(LIB_BIN)/%.o: $(LIB_SRC)/%.c
	@echo $(COL_SRC)LIB [CC]  $(notdir $<) $(COL_ERR)
	@"$(CC)" $(C_FLAGS) $(INCLUDE) -o "$@" -c $<

$(LIB_BIN)/%.o: $(LIB_SRC)/%.cpp
	@echo $(COL_SRC)LIB [CPP] $(notdir $<) $(COL_ERR)
	@"$(CXX)" $(CPP_FLAGS) $(INCLUDE) -o "$@" -c $<

# Linking ---------------------------------------------------------------------
$(TARGET_ELF): $(CORE_LIB) $(ARD_LIB_OBJ) $(LIB_OBJ) $(USR_OBJ)
	@echo $(COL_LINK)
	@echo [LD]  $@ $(COL_ERR)
	@$(CC) $(LD_FLAGS) -o "$@" $(ARD_LIB_OBJ) $(USR_OBJ) $(LIB_OBJ)  $(CORE_LIB) $(LIBS)
	@echo $(COL_OK)User code built and linked to libraries &&echo.

%.lst: %.elf
	@echo [LST] $@
	@$(OBJDUMP) -d -S --demangle --no-show-raw-insn "$<" > "$@"
	@echo $(COL_OK)Sucessfully built project$(COL_RESET) &&echo.

%.sym: %.elf
	@echo [SYM] $@
	@$(NM) $(NM_FLAGS) "$<" > "$@"

%.hex: %.elf
	@echo $(COL_LINK)[HEX] $@
	@$(OBJCOPY) -O ihex -R.eeprom "$<" "$@"

# Cleaning --------------------------------------------------------------------
cleanUser:
	@echo $(COL_LINK)Cleaning user binaries...$(COL_RESET)
	@if exist $(USR_BIN) rd /s/q "$(USR_BIN)"
	@if exist "$(TARGET_LST)" del $(subst /,\,$(TARGET_LST))
	@echo $(COL_OK)User binaries cleaned$(COL_RESET)

cleanCore:
	@echo $(COL_LINK)Cleaning core binaries...$(COL_RESET)
	@if exist $(CORE_BIN) rd /s/q "$(CORE_BIN)"
	@if exist $(CORE_LIB) del  $(subst /,\,$(CORE_LIB))
	@echo $(COL_OK)Core binaries cleaned$(COL_RESET)

cleanArdLib:
	@echo $(COL_LINK)Cleaning arduino library binaries...$(COL_RESET)
	@if exist $(ARD_LIB_BIN) rd /s/q "$(ARD_LIB_BIN)"
	@echo $(COL_OK)Arduino library binaries cleaned$(COL_RESET)

cleanUserLib:
	@echo $(COL_LINK)Cleaning user library binaries...$(COL_RESET)
	@if exist $(LIB_BIN) rd /s/q "$(LIB_BIN)"
	@echo $(COL_OK)User library binaries cleaned$(COL_RESET)

# compiler generated dependency info ------------------------------------------
-include $(CORE_OBJ:.o=.d)
-include $(USR_OBJ:.o=.d)
-include $(ARD_LIB_OBJ:.o=.d)
-include $(LIB_OBJ:.o=.d)