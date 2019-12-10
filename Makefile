VPATH := $(shell find . -name "src") ./embedded/app/main/ ./embedded/examples/ ./middleware/examples/ ./embedded/utils/ ./embedded/data/

# Code and Includes (I know, shell commands everywhere! Works though)
ALL_SRC	:= $(shell find . -name "src" -exec ls {} \;)
ALL_EX	:= $(shell find . -name "examples" -exec ls {} \; | grep ".c")
ALL_UTL	:= $(shell find . -name "utils" -exec ls {} \; | grep ".c")

# Should find all our include directories
INCLUDE_DIRS := $(shell find . -name "include") ./middleware/libs/
ALL_H   := $(shell find . -name "include" -exec ls {} \; | grep ".h") ./middleware/libs/PracticalSocket/PracticalSocket.h ./middleware/libs/CRCpp/CRC.h
# Add specific flags that allow our examples to be run with hardware-out-of-the-loop
ifdef VIRTUAL
USE_VCAN := USE_VCAN
endif

# Add a define for debugging prints in various places. Can be customized for
# whichever components are wanted
ifdef DEBUG
DEBUG_MODE := DEBUG_RETRO DEBUG_RMS DEBUG_BMS DEBUG_PRES
endif

ifdef NF
NO_FAULT := NO_FAULT
endif
ifdef LOCAL
NOI2C := NOI2C
endif

HV_USER	  := "debian"
HV_IP  := "192.168.0.7"

ifdef BB
BEAGLE := ${BBCC}
endif

# Compiler options
GCC	   	:= $(BEAGLE)gcc
GPP	   	:= $(BEAGLE)g++
IFLAGS 	:= $(addprefix -I,$(INCLUDE_DIRS))
WFLAGS	:= -Wall -Wno-deprecated -Wextra -Wno-type-limits -fdiagnostics-color
CFLAGS 	:= -std=gnu11 $(addprefix -D,$(USE_VCAN)) $(addprefix -D, $(DEBUG_MODE)) $(addprefix -D, $(NOI2C)) $(addprefix -D, $(NF))
CPFLAGS := -std=c++11
LDFLAGS := -Llib
LDLIBS 	:= -lm -lpthread

# Output Control
OUTPUT_DIR 	:= out
OBJ_DIR	   	:= $(OUTPUT_DIR)/obj
EX_OUT_DIR	:= $(OUTPUT_DIR)/tests
UTL_OUT_DIR := $(OUTPUT_DIR)/utils

HV_MAIN		:= badgerloop_HV
LV_MAIN		:= badgerloop_LV

TARGETS		:= $(addprefix $(OUTPUT_DIR)/,$(HV_MAIN))
EXAMPLES	:= $(addprefix $(EX_OUT_DIR)/,$(basename $(ALL_EX)))
UTILS		:= $(addprefix $(UTL_OUT_DIR)/,$(basename $(ALL_UTL)))

# Each Example obj should have a main(); so it has to be linked into its own executable
GEN_OBJ		:= $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(basename $(ALL_SRC))))
EX_OBJ		:= $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(basename $(ALL_EX))))
UTL_OBJ		:= $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(basename $(ALL_UTL))))

.PHONY: all examples utils clean
# .SEC keeps intermediates, so make doesnt automatically clean .o files
.SECONDARY:


all: codegen $(TARGETS)
	
examples: codegen $(EXAMPLES)

utils: $(UTILS)

codegen:
	python ./autocoding/autocode.py

copy:
	-scp -q -o ConnectTimeout=2 -r $(OUTPUT_DIR) $(HV_USER)@$(HV_IP):~/bin &

$(OUTPUT_DIR)/%: $(GEN_OBJ) $(OBJ_DIR)/%.o
	$(GPP) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EX_OUT_DIR)/%: $(GEN_OBJ) $(OBJ_DIR)/%.o | $(EX_OUT_DIR)
	$(GPP) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(UTL_OUT_DIR)/%: $(GEN_OBJ) $(OBJ_DIR)/%.o | $(UTL_OUT_DIR)
	$(GPP) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(GCC) -c $(CFLAGS) $(IFLAGS) $(WFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(GPP) -c $(CPFLAGS) $(IFLAGS) $(WFLAGS) $< -o $@

$(OBJ_DIR): | $(OUTPUT_DIR)
	mkdir $(OBJ_DIR)

$(EX_OUT_DIR): | $(OUTPUT_DIR)
	mkdir $(EX_OUT_DIR)

$(UTL_OUT_DIR): | $(OUTPUT_DIR)
	mkdir $(UTL_OUT_DIR)

$(OUTPUT_DIR):
	mkdir $(OUTPUT_DIR)

clean:
	-rm -rf $(OUTPUT_DIR)

