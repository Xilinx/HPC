#
# Makefile for building mppdyna
# on 'Xeon64 System                     '
# for 'Linux 2.6.32                      '
# using 'Intel Fortran Compiler 19.0 SSE2  '
#
#
MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
XF_PROJ_ROOT ?= $(shell bash -c 'export MK_PATH=$(MK_PATH); echo $${MK_PATH%/pcg/*}')
CUR_DIR := $(patsubst %/,%,$(dir $(MK_PATH)))
XFLIB_DIR = $(XF_PROJ_ROOT)
TEMP_DIR := $(CUR_DIR)/_x_temp
TARGET?=cpu
MODEL_PATH?=$(LS_DYNA_PATH)/small_tbc/small_tbc.k
BUILD_DIR := $(CUR_DIR)/build_output.$(TARGET)
FC=mpiifort
CC=mpigxx
#
FF=-O2 -safe-cray-ptr -assume byterecl,buffered_io,protect_parens -warn nousage -zero -ftz -fp-model precise -mP2OPT_hpo_dist_factor=21 -diag-disable 10212,10010 -traceback -fpp -qopenmp -qopenmp-lib compat -pad -DFORMAT_COO -DLINUX -DIFORT -DNET_SECURITY -DADDR64 -DINTEL -DXEON64 -DFCC80 -DTIMER=cycle_time -DSSE2 -DOVERRIDE -DOPENMP -DHYBRID -DMPP -DMPICH -DAUTODOUBLE -DNEWIO -DMKL18 -DHAVE_BLAS_LAPACK -DHAVE_I8R8_LIBRARY -DMF3_SYM -DNEW_UNITS -DEXTENDED -DBATTERY -DDUALCESE -DUSE_R123 -DLSTCODE  -DBIGID -DENABLE_HASH3 -DFFTW -DROTORDYN -DMUMPS -DLSGPART -DMF3_SYM  -DMCMS -DUSES_CXX -DTS_CPPSTD_98 -DPFEM -DMUMPSVER=stable -DUSES_CPP  -DPTHREADS -i8 -r8 -fimf-arch-consistency=true -qno-opt-dynamic-align -xSSE2 -align array16byte -fPIC

CXXFLAGS += -DHW_CLK=4e-9 -DCG_numTasks=1 -DCG_dataType=double -DCG_numChannels=16 -DCG_parEntries=4 -DCG_instrBytes=64 -DSPARSE_dataType=double -DSPARSE_indexType=uint16_t -DSPARSE_parEntries=4 -DSPARSE_accLatency=8 -DSPARSE_maxRows=4096 -DSPARSE_maxCols=4096 -DSPARSE_hbmChannels=16 -DSPARSE_hbmMemBits=256
CXXFLAGS += -I$(XFLIB_DIR)/L1/hpc/include/
CXXFLAGS += -I$(XFLIB_DIR)/pcg/hw/include
CXXFLAGS += -I$(XFLIB_DIR)/pcg/sw/include/
CXXFLAGS += -I$(XFLIB_DIR)/pcg/sw/include/impl
CXXFLAGS += -I$(XFLIB_DIR)/L2/common/include/sw
CXXFLAGS += -I$(XFLIB_DIR)/L2/sparse/include/sw/fp64
CXXFLAGS += -I$(XFLIB_DIR)/utils/include/sw
CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -std=c++14  -Wno-unknown-pragmas -Wno-unused-label
CXXFLAGS += -fmessage-length=0

LDFLAGS += -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE
#LDFLAGS += -L$(XILINX_VIVADO)/lnx64/tools/fpo_v7_0 -Wl,--as-needed -lgmp -lmpfr -lIp_floating_point_v7_0_bitacc_cmodel
#
LD=mpiifort -sox -fpp -qopenmp -static-intel
#
LF=  -lrt -lpthread -lstdc++ -lstdc++fs -lpthread
CF=  -O3 $(CXXFLAGS) -DAUTODOUBLE -DUNSCORE -Wall

ifeq ($(TARGET), fpga)
	CF += -DUSE_FPGA
endif
#
LMATH=$(LS_DYNA_PATH)/usermat/libmkl_scalapack_ilp64.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_intel_ilp64.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_sequential.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_core.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_blacs_intelmpi_ilp64.a
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_scalapack_ilp64.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_intel_ilp64.a
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_sequential.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_core.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_blacs_intelmpi_ilp64.a
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_scalapack_ilp64.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_intel_ilp64.a
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_sequential.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_core.a 
LMATH+=$(LS_DYNA_PATH)/usermat/libmkl_blacs_intelmpi_ilp64.a
#
LIBS=$(LS_DYNA_PATH)/usermat/libdyna.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblsmpp.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libmpp_lanczos.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libbcsext4.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblsda.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblsdb.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblssecurity.a  
LIBS+=$(LS_DYNA_PATH)/usermat/liblcpack.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libmf2.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libspooles.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblcpack.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libspooles.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libcparse.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblsm.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblscrypt.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libresurf.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblsmppdes.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libdsa.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblsmc.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libbalance.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libsfg.a   
LIBS+=$(LS_DYNA_PATH)/usermat/libzmumps.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libdmumps.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libmumps_common.a    
LIBS+=$(LS_DYNA_PATH)/usermat/libmetis.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libim_rotor_dynamics.a  
LIBS+=$(LS_DYNA_PATH)/usermat/libmcms.a   
LIBS+=$(LS_DYNA_PATH)/usermat/liblsmpp.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblso.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libunits.a  
LIBS+=$(LS_DYNA_PATH)/usermat/libem.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfemster_wrap.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfemster_wrap2d.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfemster_wrap1d.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfemster.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfemster2d.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfemster1d.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libpfem.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libcese.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libchemistry.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libem_tailsit.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libparticles.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libeosfl2.a  
LIBS+=$(LS_DYNA_PATH)/usermat/libpfem.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libmoving.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libdualcese.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libbattery.a 
LIBS+=$(LS_DYNA_PATH)/usermat/liblsmpp.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libmetis.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfftw3.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libfftw3_omp.a  
LIBS+=$(LS_DYNA_PATH)/usermat/libansysl.a   
LIBS+=$(LS_DYNA_PATH)/usermat/libdyna.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libmetis.a 
LIBS+=$(LS_DYNA_PATH)/usermat/libspooles.a
#

OBJS += $(LS_DYNA_PATH)/usermat/init_once.o
OBJS += $(LS_DYNA_PATH)/usermat/dynm.o
OBJS += $(LS_DYNA_PATH)/usermat/banner.o
OBJS += $(LS_DYNA_PATH)/usermat/sec.o
OBJS += $(LS_DYNA_PATH)/usermat/adummy_graph.o
OBJS += $(LS_DYNA_PATH)/usermat/orderByMetis.o
OBJS += $(LS_DYNA_PATH)/usermat/mod_impl_matrices.o
OBJS += $(BUILD_DIR)/init_dyn21.o
OBJS += $(BUILD_DIR)/dyn21.o
OBJS += $(BUILD_DIR)/dyn21umat.o
OBJS += $(BUILD_DIR)/dyn21umats.o
OBJS += $(BUILD_DIR)/dyn21umatv.o
OBJS += $(BUILD_DIR)/dyn21umatc.o
OBJS += $(BUILD_DIR)/dyn21tumat.o
OBJS += $(BUILD_DIR)/dyn21utan.o
OBJS += $(BUILD_DIR)/dyn21ueos.o
OBJS += $(BUILD_DIR)/dyn21cnt.o
OBJS += $(BUILD_DIR)/dyn21ushl.o
OBJS += $(BUILD_DIR)/dyn21usld.o
OBJS += $(BUILD_DIR)/dyn21em.o
OBJS += $(BUILD_DIR)/dyn21icfd.o
OBJS += $(BUILD_DIR)/couple2other_user.o
OBJS += $(BUILD_DIR)/dynrfn_user.o
OBJS += $(BUILD_DIR)/userLE_JPCG.o
OBJS += $(BUILD_DIR)/JPCG.o

ifeq ($(TARGET), fpga)
OBJS += $(BUILD_DIR)/gen_signature.o
OBJS += $(BUILD_DIR)/xFPGA.o
OBJS += $(BUILD_DIR)/xcl2.o
OBJS += $(BUILD_DIR)/cgHost.o
endif

EXE := $(BUILD_DIR)/mppdyna
BENCHMAKR = $(XF_PROJ_ROOT)/pcg/plugins/LS-DYNA/benchmark.py

$(EXE): $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBS) $(LMATH) $(LF)

run: $(EXE)
	cd $(BUILD_DIR) && \
	$(EXE) i=$(MODEL_PATH) memory=1G ncpu=1 impcon=1
	cd ..

benchmark: $(EXE)
	$(BENCHMAKR) --model_path $(MODEL_PATH)

clean:
	rm -rf build_output*
	rm -rf *.mod

$(BUILD_DIR)/JPCG.o:
	mkdir -p ${BUILD_DIR}
	$(CC) -c $(CF) -I. -I$(LS_DYNA_PATH) ../../usermat/new_JPCG.cpp -o '$@'

ifeq ($(TARGET), fpga)
$(BUILD_DIR)/gen_signature.o:
	mkdir -p ${BUILD_DIR}
	$(CC) -c $(CF)  $(XFLIB_DIR)/L2/sparse/src/sw/c++/gen_signature.cpp  -o '$@'
$(BUILD_DIR)/xFPGA.o:
	mkdir -p ${BUILD_DIR}
	$(CC) -c $(CF)  $(XFLIB_DIR)/L2/common/src/sw/xFPGA.cpp -o '$@'
$(BUILD_DIR)/xcl2.o:
	mkdir -p ${BUILD_DIR}
	$(CC) -c $(CF)  $(XFLIB_DIR)/L2/common/src/sw/xcl2.cpp -o '$@'
$(BUILD_DIR)/cgHost.o:
	mkdir -p ${BUILD_DIR}
	$(CC) -c $(CF)  $(XFLIB_DIR)/pcg/sw/src/cgHost.cpp -o '$@'
endif

$(BUILD_DIR)/userLE_JPCG.o: ../../usermat/userLE_JPCG.F90
	mkdir -p ${BUILD_DIR}
	$(FC) -c $(FF) -I$(LS_DYNA_PATH)/usermat $^ -o '$@'
$(BUILD_DIR)/userinterface.o: $(LS_DYNA_PATH)/usermat/userinterface.f90
	mkdir -p ${BUILD_DIR}
	$(FC) -c $(FF) -I$(LS_DYNA_PATH)/usermat $^ -o '$@'
$(BUILD_DIR)/%.o: $(LS_DYNA_PATH)/usermat/%.f $(BUILD_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(LS_DYNA_PATH)/usermat $^ -o '$@'
