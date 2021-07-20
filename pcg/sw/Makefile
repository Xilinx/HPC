#
# Copyright 2020-2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifndef XILINX_XRT
    XILINX_XRT = /opt/xilinx/xrt
    export XILINX_XRT
endif

ifndef XILINX_XRM
    XILINX_XRM=/opt/xilinx/xrm
    export XILINX_XRM
endif

ifndef XILINX_PLATFORMS
    XILINX_PLATFORMS = /opt/xilinx/platforms
    export XILINX_PLATFORMS
endif

# Which Alveo device to use for running tests
DEVICE_ID ?= 1

.DELETE_ON_ERROR:
.PHONY: all

#all: javaApi cppBuild javaTest stage
all: cppTest

#
# Global Definitions
#

## Location of HPC root directory
HPC_DIR = ../..

# General output directory for intermediate build artifacts
BUILD_DIR = build

#######################################################################################################################
#
# Java API
#

JAVA_LIB_DIR = lib
JAVA_BUILD_DIR = $(BUILD_DIR)
JAVA_SRC_DIR = java/src
JAVA_API_JAR = $(JAVA_LIB_DIR)/xilinxPcgApi-0.1.jar
JAVA_API_DOC_JAR = $(JAVA_LIB_DIR)/xilinxPcgApi-0.1-javadoc.jar
JAVA_JNI_HEADER = $(JAVA_BUILD_DIR)/api/c-headers/com_xilinx_apps_pcg_DoublePcg.h

.PHONY: javaApi javaApiJar
	
javaApi: javaApiJar $(JAVA_API_DOC_JAR)
	
javaApiJar:
	cd java && ant jar
	
$(JAVA_API_DOC_JAR): $(JAVA_API_JAR)
	cd java && ant docs


#######################################################################################################################
#
# C++ API Library
#

CCC = g++
CXX = g++

VPP_FLAGS += -DCG_numTasks=1 -DCG_dataType=double -DCG_instrBytes=64 -DCG_tkStrWidth=8 -DCG_parEntries=4 -DCG_numChannels=16 -DCG_vecParEntries=4 -DSPARSE_dataType=double -DSPARSE_dataBits=64 -DSPARSE_parEntries=4 -DSPARSE_indexType=uint16_t -DSPARSE_indexBits=16 -DSPARSE_maxRows=4096 -DSPARSE_maxCols=4096 -DSPARSE_accLatency=8 -DSPARSE_hbmMemBits=256 -DSPARSE_hbmChannels=16 

CXXFLAGS = -std=c++11 -fPIC -Wall -Wno-unknown-pragmas -Wno-unused-label \
           -Wno-narrowing -fmessage-length=0 $(VPP_FLAGS)

# Define the target directories.

ifdef DEBUG
CPP_BUILD_DIR = Debug
CXXFLAGS += -O0 -g
else
CPP_BUILD_DIR = Release
CXXFLAGS += -O3 -DNDEBUG
endif


## Target: libXilinxPcg.so
INCLUDES_libPcg = \
    -Iinclude \
    -isystem $(HPC_DIR)/L1/hpc/include \
    -I$(HPC_DIR)/pcg/hw/include \
    -I$(HPC_DIR)/L2/common/include/sw \
    -I$(HPC_DIR)/L2/sparse/include/sw/fp64 \
    -I$(HPC_DIR)/utils/include/sw \
    -I$(XILINX_XRT)/include
#	-I${JAVA_HOME}/include \
#	-I${JAVA_HOME}/include/linux

LDLIBS_libPcg = \
	
LIB_SHORT_NAME = XilinxPcg
LIB_NAME = lib$(LIB_SHORT_NAME).so
LIB_STATIC_SHORT_NAME = XilinxPcgStatic
LIB_STATIC_NAME = lib$(LIB_STATIC_SHORT_NAME).so

LOADER_SHORT_NAME = XilinxPcg_loader
LOADER_NAME = lib$(LOADER_SHORT_NAME).a

SRCS_HPC_NAMES = \
    L2/sparse/src/sw/c++/gen_signature.cpp \
    L2/common/src/sw/xFPGA.cpp \
    L2/common/src/sw/xcl2.cpp
    
SRCS_HPC = $(addprefix $(HPC_DIR)/,$(SRCS_HPC_NAMES))
OBJS_HPC = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_HPC:.cpp=.o)))

SRCS_loader = src/pcg_loader.c
OBJS_loader = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_loader:.c=.o)))
DEPS_loader = $(OBJS_loader:.o=.d)

SRCS_top = $(filter-out $(SRCS_loader),$(wildcard src/*.cpp))
OBJS_top = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_top:.cpp=.o)))

SRCS_all = $(SRCS_HPC) $(SRCS_top) $(SRCS_loader)
OBJS_all = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_all:.cpp=.o) $(SRCS_all:.c=.o)))
DEPS_all = $(OBJS_all:.o=.d)

OBJS_libPcg = $(OBJS_HPC) $(OBJS_top) 

CPP_DOXY_TARGET_DIR = $(BUILD_DIR)/cppdocs
CPP_DOXY_TRIGGER_FILE = $(CPP_DOXY_TARGET_DIR)/html/index.html

.PHONY: cppBuild cppBuild2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppBuild: $(CPP_BUILD_DIR)
	@make cppBuild2

cppBuild2: $(CPP_BUILD_DIR)/$(LIB_NAME) $(CPP_BUILD_DIR)/$(LOADER_NAME)


# Create the target directory (if needed)
$(CPP_BUILD_DIR):
	mkdir -p $(CPP_BUILD_DIR)

# .so

LIB_DEPS = -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -DVERBOSE

ifeq ($(MEMALLOC),tcmalloc)
    LIB_DEPS += -ltcmalloc
endif

SHAREDLIB_FLAGS_libPcg = -fPIC -rdynamic -shared -w -Wl,--export-dynamic

$(CPP_BUILD_DIR)/$(LIB_NAME): $(OBJS_libPcg) $(DEPLIBS_libPcg)
	$(LINK.cc) -o $@ $(OBJS_libPcg) $(SHAREDLIB_FLAGS_libPcg) $(LDLIBS_libPcg) $(LIB_DEPS)

$(CPP_BUILD_DIR)/$(LIB_STATIC_NAME): $(OBJS_libPcg) $(DEPLIBS_libPcg)
	$(LINK.cc) -o $@ $(OBJS_libPcg) -static-libgcc -static-libstdc++ \
	    $(SHAREDLIB_FLAGS_libPcg) $(LDLIBS_libPcg) $(LIB_DEPS)

# loader .a

$(CPP_BUILD_DIR)/$(LOADER_NAME): $(OBJS_loader) $(DEPLIBS_loader)
	ar ru $@ $(OBJS_loader)
	ranlib $@

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_libPcg) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_libPcg) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_all),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppBuild2)
-include $(DEPS_all)
endif

#======================================================================================================================

# Doxygen output for proofreading documentation (as opposed to the process to publish on xilinx.github.io)

.PHONY: doxy-cpp

doxy-cpp: $(CPP_DOXY_TRIGGER_FILE)

CPP_DOXY_CONFIG_FILE = docs/cpp/DoxyFile

$(CPP_DOXY_TRIGGER_FILE): include/pcg.h $(CPP_DOXY_CONFIG_FILE)
	@set -e; \
	if ! [ -x "$$(command -v doxygen)" ]; then \
	    echo "ERROR: doxygen appears not to be installed.  Please install at least version 1.8.5"; \
	    false; \
	fi; \
	mkdir -p $(CPP_DOXY_TARGET_DIR); \
	sed 's,COSINESIM_VERSION,$(strip $(shell cat VERSION)),g' < $(CPP_DOXY_CONFIG_FILE) > $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp; \
	doxygen $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp; \
	rm -f $(CPP_DOXY_TARGET_DIR)/doxyfile.tmp


#######################################################################################################################
#
# python API wrapper
#

PYTHON = python3.6
PYTHON_DIR = /usr/include/$(PYTHON)
PYTHONENV_NAME = pyvenv
PYTHONENV = ./$(PYTHONENV_NAME)/bin/$(PYTHON)
PYTHON_API_DIR = wrappers/python
PYTHON_LIB_NAME := xilPcg.so #$(shell python-config --extension-suffix)

SRC_FILE_NAMES_python = pythonapi.cpp

INCLUDES_python = \
    -I$(PYTHON_DIR) \
    -Iinclude

SRCS_python = $(addprefix $(PYTHON_API_DIR)/,$(SRC_FILE_NAMES_python))
OBJS_python = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_python:.cpp=.o)))

LIB_PATH = $(CPP_BUILD_DIR)

LDFLAGS_python = -L$(LIB_PATH) -l$(LIB_SHORT_NAME)

# Add pybind11 includes and set library name after setting virtual env
ifeq ($(MAKECMDGOALS),pythonApi2)
INCLUDES_python += $(shell $(PYTHONENV) -m pybind11 --includes)
endif

SHAREDLIB_FLAGS_python = -shared

.PHONY: pythonApi pythonApi2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
pythonApi: cppBuild $(PYTHONENV_NAME)
	@make pythonApi2

$(PYTHONENV_NAME):
	$(PYTHON) -m venv $(PYTHONENV_NAME);\
	$(PYTHONENV) -m pip install pybind11
#	$(PYTHONENV) -m pip install python-config

pythonApi2: $(CPP_BUILD_DIR) $(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME)

$(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME): $(OBJS_python)
	$(LINK.cc) $^ -o $@ $(SHAREDLIB_FLAGS_python) $(LDFLAGS_python)

# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	$(COMPILE.cc) $(INCLUDES_python) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	$(COMPILE.cc) -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_python) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_python),$(eval $(call BUILD_CPP_RULE,$(src))))


#######################################################################################################################
#
# XCLBIN
#

# Set this variable to the parent directory of the Vitis build directory (which is build_dir.hw...)
L2_TOP_DIR = $(HPC_DIR)/pcg/tests/pcg_spmv_c++

L2_XDEVICE = xilinx_u280_xdma_201920_3
L2_BUILD_DIR = $(L2_TOP_DIR)/build_dir.hw.$(L2_XDEVICE)
XCLBIN_NAME = cgSolver_ddrParParam_hbmRbParam.xclbin
XCLBIN_RELEASE_NAME = pcg_$(L2_XDEVICE).xclbin
XCLBIN_FILE = $(L2_BUILD_DIR)/$(XCLBIN_NAME)
XCLBIN_INSTALLED = /opt/xilinx/apps/hpc/pcg/xclbin/$(XCLBIN_RELEASE_NAME)

.PHONY: xclbin

xclbin:
	cd $(L2_TOP_DIR); \
	source $(XILINX_XRT)/setup.sh; \
	export PLATFORM_REPO_PATHS=$(XILINX_PLATFORMS); \
	make build TARGET=hw
	
$(XCLBIN_FILE):
	@if [ -r $(XCLBIN_INSTALLED) ]; then \
	    echo "INFO: The XCLBIN file $(XCLBIN_FILE) is missing.  Copying from installed area."; \
	    cp -r $(XCLBIN_INSTALLED) $@; \
	else \
	    echo "ERROR: The XCLBIN file $(XCLBIN_FILE) is missing."; \
	    echo "Because building an XCLBIN file can be an all-day proposition, you have two choices for completing this build:"; \
	    echo "1. Copy a pre-built XCLBIN to the target location above.  (Recommended for developers)"; \
	    echo "2. Do 'make xclbin' and go for a very long coffee break.  (Recommended for build automation)"; \
	    false; \
	fi

#######################################################################################################################
#
# Staging
#

STAGE_DIR = staging

# Files to be direct-copied from source tree to staging area

STAGE_COPY_FILES = \
    VERSION \
    include/pcg.h \
    \
    src/pcg_loader.c

#    impl/cgSolverGemv.hpp \
#    impl/cgSolverKernel.hpp \


define STAGE_COPY_RULE
$$(STAGE_DIR)/$(1): $(1)
	cp -f $(1) $$(STAGE_DIR)/$(1)
endef
$(foreach f,$(STAGE_COPY_FILES),$(eval $(call STAGE_COPY_RULE,$(f))))

# Template-based implementation headers from outside the PCG tree

STAGE_COPY_EXT_IMPL_FILES = \


#    pcg/hw/include/impl/cgInstr.hpp

define STAGE_COPY_EXT_IMPL_RULE
$(2): $(1)
	cp -f $(1) $(2)
endef
$(foreach f,$(STAGE_COPY_EXT_IMPL_FILES),$(eval $(call STAGE_COPY_EXT_IMPL_RULE,$(HPC_DIR)/$(f),$(STAGE_DIR)/include/impl/$(notdir $(f)))))

# Header dependencies from outside the PCG tree.  These are moved to the top-level include directory for now,
# as we need to determine whether it makes sense to convert all of them to impl headers
# TODO: work out what to do with these headers

STAGE_COPY_EXT_INCLUDE_FILES = \


#    pcg/hw/include/control.hpp \
#    L1/hpc/include/memInstr.hpp \
#    L2/common/include/sw/xFpga.hpp \
#    L2/common/include/sw/xcl2.hpp \
#    L2/sparse/include/sw/fp64/gen_signature.hpp \
#    L2/sparse/include/sw/fp64/matrix_params.hpp \
#    L2/sparse/include/sw/fp64/signature.hpp \
#    utils/include/sw/utils.hpp \
#    utils/include/sw/binFiles.hpp

define STAGE_COPY_EXT_INCLUDE_RULE
$(2): $(1)
	cp -f $(1) $(2)
endef
$(foreach f,$(STAGE_COPY_EXT_INCLUDE_FILES),$(eval $(call STAGE_COPY_EXT_INCLUDE_RULE,$(HPC_DIR)/$(f),$(STAGE_DIR)/include/$(notdir $(f)))))

# Binary (C++) files

STAGE_LIB_FILES = \
    $(LIB_NAME) \
    $(LIB_STATIC_NAME) \
    $(LOADER_NAME)

ifneq ($(wildcard $(CPP_BUILD_DIR)/$(PYTHON_LIB_NAME)),)
    STAGE_LIB_FILES += $(PYTHON_LIB_NAME)
endif

define STAGE_LIB_RULE
$$(STAGE_DIR)/lib/$(1): $$(CPP_BUILD_DIR)/$(1)
	cp -f $$^ $$@
endef
$(foreach f,$(STAGE_LIB_FILES),$(eval $(call STAGE_LIB_RULE,$(f))))

# Example files - need to be zipped up

STAGE_EXAMPLE_ZIP = examples.zip
STAGE_EXAMPLE_FILE_NAMES = \
    c/example.c c/Makefile

#    python/pythondemo.py python/run.sh \
#    python/README.md python/jupyter/jupyter_demo.ipynb

STAGE_EXAMPLE_FILES = $(addprefix examples/,$(STAGE_EXAMPLE_FILE_NAMES))
STAGE_STAGED_EXAMPLE_FILES = $(addprefix $(STAGE_DIR)/,$(STAGE_EXAMPLE_FILES))

.PHONY: stage fill-example-stage

$(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP): $(STAGE_EXAMPLE_FILES)
	mkdir -p $(sort $(dir $(STAGE_STAGED_EXAMPLE_FILES)))
	@make fill-example-stage
	@cd $(STAGE_DIR) ; \
	zip -r $(STAGE_EXAMPLE_ZIP) examples
	rm -rf $(STAGE_DIR)/examples

fill-example-stage: $(STAGE_STAGED_EXAMPLE_FILES)
	
$(STAGE_STAGED_EXAMPLE_FILES):
	cp -f $(patsubst $(STAGE_DIR)/%,%,$@) $@
	

# XCLBIN file

STAGE_XCLBIN_NAME = $(XCLBIN_RELEASE_NAME)
STAGE_XCLBIN_FILE = $(STAGE_DIR)/xclbin/$(STAGE_XCLBIN_NAME)

$(STAGE_XCLBIN_FILE): $(XCLBIN_FILE)
	cp -f $^ $@

# Top-level packaging rule

STAGE_SUBDIR_NAMES = include lib src xclbin
STAGE_SUBDIRS = $(addprefix $(STAGE_DIR)/,$(STAGE_SUBDIR_NAMES))
STAGE_ALL_FILES = \
    $(addprefix $(STAGE_DIR)/,$(STAGE_COPY_FILES)) \
    $(addprefix $(STAGE_DIR)/include/impl/,$(notdir $(STAGE_COPY_EXT_IMPL_FILES))) \
    $(addprefix $(STAGE_DIR)/include/,$(notdir $(STAGE_COPY_EXT_INCLUDE_FILES))) \
    $(addprefix $(STAGE_DIR)/lib/,$(STAGE_LIB_FILES)) \
    $(STAGE_XCLBIN_FILE) \
    $(STAGE_DIR)/$(STAGE_EXAMPLE_ZIP) \

stage: cppBuild $(STAGE_SUBDIRS) $(STAGE_ALL_FILES)

$(STAGE_SUBDIRS):
	mkdir -p $@

#######################################################################################################################
#
# C++ Tests
#

TEST_DIR = tests

#INCLUDES_test = -I$(STAGE_DIR)/include -I$(XILINX_XRT)/include
INCLUDES_test = -I$(STAGE_DIR)/include

LDLIBS_test = \
	

SRC_FILE_NAMES_test = \
    pcgtest.c

SRCS_test = $(addprefix $(TEST_DIR)/,$(SRC_FILE_NAMES_test))
OBJS_test = $(addprefix $(CPP_BUILD_DIR)/,$(notdir $(SRCS_test:.c=.o)))
DEPS_test = $(OBJS_test:.o=.d)

# List of all test executables to build
EXEC_FILE_NAMES_test = \
    pcgtest \
    pcgdyntest

EXECS_test = $(addprefix $(CPP_BUILD_DIR)/,$(EXEC_FILE_NAMES_test))

.PHONY: cppTest cppTest2

# Make in a child process so that we can isolate the run that includes automatic header dependencies
cppTest: stage
	@make cppTest2

cppTest2: $(EXECS_test)

# Test executables

LDFLAGS_test = -fPIC -w -L $(CPP_BUILD_DIR) -l$(LIB_SHORT_NAME) -lstdc++

$(CPP_BUILD_DIR)/pcgtest: $(CPP_BUILD_DIR)/pcgtest.o $(CPP_BUILD_DIR)/$(LIB_NAME)
	gcc -o $@ $< $(LDFLAGS_test) $(LIB_DEPS)

$(CPP_BUILD_DIR)/pcgdyntest: $(CPP_BUILD_DIR)/pcgtest.o $(CPP_BUILD_DIR)/$(LOADER_NAME) $(CPP_BUILD_DIR)/$(LIB_NAME)
	gcc -o $@ $< -fPIC -w -L $(CPP_BUILD_DIR) -l$(LOADER_SHORT_NAME) -ldl -lpthread

# TODO: dynamic exec shouldn't have to link with OpenCL, etc.  Figure out why the link line below
# causes undefined symbols to OpenCL and others

#	gcc -o $@ $< -fPIC -w -L $(CPP_BUILD_DIR) -l$(LOADER_SHORT_NAME) -ldl $(LIB_DEPS)

.PHONY: run-tests run-prep-data run-test run-dyn-test run-long-test

run-tests: run-test run-dyn-test run-long-test

# Where PCG test data will be downloaded and partitioned
TEST_DATA_DIR = $(TEST_DIR)/data

# Where the long test writes its results
TEST_RESULTS_DIR = $(TEST_DIR)/results

run-prep-data:
	@echo "Generating test data..."
	utils/genMat.sh -f $(TEST_DIR)/shorttest.txt -o $(TEST_DATA_DIR)

run-test: cppTest run-prep-data
	@echo "Running statically built pcgtest..."
	@. $(XILINX_XRT)/setup.sh; \
	export LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH; \
	$(CPP_BUILD_DIR)/pcgtest  $(DEVICE_ID) $(STAGE_XCLBIN_FILE)

#	$(CPP_BUILD_DIR)/pcgtest $(STAGE_XCLBIN_FILE) 5000 1e-12 $(TEST_DATA_DIR) nasa2910 1 $(DEVICE_ID)

run-dyn-test: cppTest run-prep-data
	@echo "Running dynamically loaded pcgdyntest..."
	@. $(XILINX_XRT)/setup.sh; \
	export LD_LIBRARY_PATH=$(PWD)/$(CPP_BUILD_DIR):$$LD_LIBRARY_PATH; \
	$(CPP_BUILD_DIR)/pcgdyntest  $(DEVICE_ID) $(STAGE_XCLBIN_FILE)

#	$(CPP_BUILD_DIR)/pcgdyntest $(STAGE_XCLBIN_FILE) 5000 1e-12 $(TEST_DATA_DIR) nasa2910 1 $(DEVICE_ID)

run-long-test: cppTest
	@echo "Running pcgtest on full data set..."
	$(TEST_DIR)/runAllPcgTests.sh -f $(TEST_DIR)/longtest.txt -o $(TEST_DATA_DIR) -r $(TEST_RESULTS_DIR) \
	    -d $(DEVICE_ID)
	
# Macro to create a .o rule and a .d rule for each .cpp

define BUILD_CPP_RULE

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o): $(1)
	gcc -c $(INCLUDES_test) -o $$@ $$<

$(CPP_BUILD_DIR)/$(notdir $(basename $(1)).d): $(1)
	@set -e; \
	rm -f $$@; \
	gcc -MM -MT $(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o) $(INCLUDES_test) -MF $$@.$$$$$$$$ $$<; \
        sed 's,\($(CPP_BUILD_DIR)/$(notdir $(basename $(1)).o)\)[ :]*,\1 $$@ : ,g' < $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$

endef

# Expand the macro for each source file

$(foreach src,$(SRCS_test),$(eval $(call BUILD_CPP_RULE,$(src))))

# Include generated header dependency rules, but only if building the JNI .so
# (This restriction prevents problems with the dependency on the Java API for the JNI header.)

ifeq ($(MAKECMDGOALS),cppTest2)
-include $(DEPS_test)
endif


#######################################################################################################################
#
# Packaging
#

OSDIST = $(shell lsb_release -si)
DIST_TARGET =
ifeq ($(OSDIST),Ubuntu)
    DIST_TARGET = DEB
else ifeq ($(OSDIST),CentOS)
    DIST_TARGET = RPM
endif

.PHONY: dist

dist: stage
	@cd package; \
	if [ "$(DIST_TARGET)" == "" ]; then \
	    echo "Packaging is supported for only Ubuntu and CentOS."; \
	else \
	    echo "Packaging $(DIST_TARGET) for $(OSDIST)"; \
	    make ; \
	fi


#######################################################################################################################
#
# Java Tests
#

.PHONY: javaTest

javaTest:
	cd test/java && ant

#######################################################################################################################
#
# Clean
#

#### Clean target deletes all generated files ####
.PHONY: clean clean-all clean-xclbin

clean:
	rm -rf Debug Release $(JAVA_LIB_DIR) $(BUILD_DIR) $(STAGE_DIR) $(TEST_RESULTS_DIR)
	@echo "NOTE: Because the XCLBIN file takes so long to generate, it is not removed with this 'clean' target."
	@echo "      To clean the XCLBIN, use 'make clean-xclbin'"
	
clean-all: clean
	rm -rf $(TEST_DATA_DIR)

clean-xclbin:
	rm -f $(XCLBIN_FILE)

help:
	@echo "Valid make targets:"
	@echo "(default)     : Makes deliverable artifacts (.so, etc.), staging area, and test executables"
	@echo "dist          : The above, plus PCG product package (RPM or DEB) for the current OS and architecture"
	@echo "clean         : Removes built artifacts, but not the xclbin, downloaded/partitioned test data, or"
	@echo "                the Python virtual environment (venv) created for processing test data"
	@echo "clean-all     : The above, plus deleting the test data and Python venv, but still NOT the xclbin"
	@echo "doxy-cpp      : Generates CPP documentation using doxygen for testing purposes. The actual release"
	@echo "                documentation is an external flow. The default html output directory is ./build/cppdocs"
	@echo "run-tests     : Run all tests below"
	@echo "run-test      : Run single test with statically linked executable"
	@echo "run-dyn-test  : Run single test using dynamic loader"
	@echo "run-long-test : Run all data sets found in tests/longtest.txt"
