# /*
#  *****************************************************************************
#  * Copyright 2016-2018 Impinj, Inc.                                          *
#  *                                                                           *
#  * Licensed under the Apache License, Version 2.0 (the "License");           *
#  * you may not use this file except in compliance with the License.          *
#  * You may obtain a copy of the License at                                   *
#  *                                                                           *
#  * http://www.apache.org/licenses/LICENSE-2.0                                *
#  *                                                                           *
#  * Unless required by applicable law or agreed to in writing, software       *
#  * distributed under the License is distributed on an "AS IS" BASIS,         *
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
#  * See the License for the specific language governing permissions and       *
#  * limitations under the License.                                            *
#  *****************************************************************************/

# Where do the target files go
BUILD_DIR   = build
TARGET_DIR  = output

# Put something else here for cross compiling
# e.g. TOOL_PREFIX = arm_v5t_le-
# Leave it empty to use the default tool chain
TOOL_PREFIX =

CC          = $(TOOL_PREFIX)gcc
LD          = $(TOOL_PREFIX)ld
AR          = $(TOOL_PREFIX)ar
STRIP       = $(TOOL_PREFIX)strip
SIZE        = $(TOOL_PREFIX)size

#Possible levels are 0, 1, 2, 3, or s (optimize for size)
OPTIMIZATION = s

#Comment this out if you want to leave debugging symbols in the
#final binary
STRIP_BINARY = 1

# Test for host OS.
ifeq ($(OS), Windows_NT)
	EXE_SUFFIX = .exe
endif

ifeq ($(STRIP_BINARY), 1)
	STRIP_CMD = $(STRIP) $@$(EXE_SUFFIX)
else
	STRIP_CMD =
endif

# Test for host OS.
ifeq ($(OS), Windows_NT)
	PLATFORM_LINK_FLAGS = -lwinmm
	PLAT = win32
endif

ifeq ($(shell uname), Linux)
	PLATFORM_LINK_FLAGS = -lc
	PLAT = linux
endif

ifeq ($(shell uname), Darwin)
	PLATFORM_LINK_FLAGS = -lc
	PLAT = osx
endif

TOP_DIR = ..

DIRS = \
	$(TOP_DIR)/Library \
	$(TOP_DIR)/Library/Nanopb \
	$(TOP_DIR)/Library/PbMessages

VPATH = $(DIRS)

# IRI Library
IRI_LIB_SOURCES = \
		iri.c \
		$(notdir $(wildcard $(TOP_DIR)/Library/Nanopb/*.c)) \
		$(notdir $(wildcard $(TOP_DIR)/Library/PbMessages/*.c))
IRI_LIB_OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(IRI_LIB_SOURCES)))
IRI_LIB = $(BUILD_DIR)/IRI.a

# ipj_util library
IPJ_UTIL_SOURCES  = ipj_util.c
IPJ_UTIL_OBJECTS  = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(IPJ_UTIL_SOURCES)))
IPJ_UTIL_LIB = $(BUILD_DIR)/ipj_util.a

# Example apps
# All the example apps should begin with 'IRI_'
APP_PREFIX       = IRI_
EXAMPLE_SOURCES  = $(notdir $(wildcard $(APP_PREFIX)*.c))
EXAMPLE_OBJECTS  = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(EXAMPLE_SOURCES))) platform_$(PLAT).o
EXAMPLE_TARGETS  = $(patsubst %.c, $(TARGET_DIR)/%, $(EXAMPLE_SOURCES))

# No missing field initializers is necessary to prevent GCC from freaking out at the protobufs
CFLAGS += $(addprefix -I, $(DIRS)) -O$(OPTIMIZATION) -DPB_FIELD_16BIT -Wall -Wno-missing-field-initializers
LFLAGS += $(PLATFORM_LINK_FLAGS)

# Make the output directory
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(TARGET_DIR))

# Default target
all: $(EXAMPLE_TARGETS)

#Target to just build the library and produce a size report
library: $(IRI_LIB)
	$(SIZE) -t $(IRI_LIB) > library_size.txt


# Don't delete intermediate files. Without this the rule below
# will rm the $(BUILD_DIR)/*.o files the first time make is run
.SECONDARY:

# Build each app as its own executable (matches $(EXAMPLE_TARGETS))
$(TARGET_DIR)/%: $(BUILD_DIR)/%.o $(IPJ_UTIL_LIB) $(IRI_LIB) $(BUILD_DIR)/platform_$(PLAT).o
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS)
	$(STRIP_CMD)

$(IRI_LIB): $(IRI_LIB_OBJECTS)
	$(AR) r $@ $^

$(IPJ_UTIL_LIB): $(IPJ_UTIL_OBJECTS)
	$(AR) r $@ $^

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(TARGET_DIR)

# Rule for compiling c files and generating autodeps at the same time and
# placing all generated files in $(BUILD_DIR)
DEP_FILE = $(BUILD_DIR)/$(*F)
$(BUILD_DIR)/%.o : %.c
	$(CC) -MMD -c $(CFLAGS) $< -o $@
	@cp $(DEP_FILE).d $(DEP_FILE).P; \
      sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
          -e '/^$$/ d' -e 's/$$/ :/' < $(DEP_FILE).d >> $(DEP_FILE).P; \
      mv $(DEP_FILE).P $(DEP_FILE).d

# Include dependency targets
-include $(IRI_LIB_OBJECTS:%.o=%.d)
-include $(IPJ_UTIL_OBJECTS:%.o=%.d)
-include $(EXAMPLE_OBJECTS:%.o=%.d)
