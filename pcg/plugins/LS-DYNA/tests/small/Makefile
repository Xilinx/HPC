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
BUILD_DIR := $(CUR_DIR)/build_output
FC=mpiifort
CC=mpigxx
#
FF=-O2 -safe_cray_ptr -assume byterecl,buffered_io,protect_parens -warn nousage -zero -ftz -fp-model precise -mP2OPT_hpo_dist_factor=21 -diag-disable 10212,10010 -traceback -fpp -qopenmp -qopenmp-lib compat -pad -DLINUX -DIFORT -DNET_SECURITY -DADDR64 -DINTEL -DXEON64 -DFCC80 -DTIMER=cycle_time -DSSE2 -DOVERRIDE -DOPENMP -DHYBRID -DMPP -DMPICH -DAUTODOUBLE -DNEWIO -DMKL18 -DHAVE_BLAS_LAPACK -DHAVE_I8R8_LIBRARY -DMF3_SYM -DNEW_UNITS -DEXTENDED -DBATTERY -DDUALCESE -DUSE_R123 -DLSTCODE  -DBIGID -DENABLE_HASH3 -DFFTW -DROTORDYN -DMUMPS -DLSGPART -DMF3_SYM  -DMCMS -DUSES_CXX -DTS_CPPSTD_98 -DPFEM -DMUMPSVER=stable -DUSES_CPP  -DPTHREADS -i8 -r8 -fimf-arch-consistency=true -qno-opt-dynamic-align -xSSE2 -align array16byte -fPIC

CXXFLAGS += -DHW_CLK=4e-9 -DCG_numTasks=1 -DCG_dataType=double -DCG_numChannels=16 -DCG_parEntries=4 -DCG_instrBytes=64 -DSPARSE_dataType=double -DSPARSE_indexType=uint16_t -DSPARSE_parEntries=4 -DSPARSE_accLatency=8 -DSPARSE_maxRows=4096 -DSPARSE_maxCols=4096 -DSPARSE_hbmChannels=16 -DSPARSE_hbmMemBits=256
CXXFLAGS += -I$(XFLIB_DIR)/L1/hpc/include/
CXXFLAGS += -I$(XFLIB_DIR)/pcg/include
CXXFLAGS += -I$(XFLIB_DIR)/pcg/include/sw
CXXFLAGS += -I$(XFLIB_DIR)/L2/common/include/sw
CXXFLAGS += -I$(XFLIB_DIR)/L2/sparse/include/sw/fp64
CXXFLAGS += -I$(XFLIB_DIR)/utils/include/sw
CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -std=c++11  -Wno-unknown-pragmas -Wno-unused-label
CXXFLAGS += -fmessage-length=0

LDFLAGS += -L$(XILINX_XRT)/lib -lOpenCL -lpthread -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE
LDFLAGS += -L$(XILINX_VIVADO)/lnx64/tools/fpo_v7_0 -Wl,--as-needed -lgmp -lmpfr -lIp_floating_point_v7_0_bitacc_cmodel
#
LD=mpiifort -sox -fpp -qopenmp -static-intel
#
LF=  -lrt -lstdc++ -lpthread -lstdc++ -lpthread
CF=  -O3 -DFORMAT_COO -DUSE_FPGA -DAUTODOUBLE -DUNSCORE -DFORMAT_COO -Wall
#
LMATH= \
libmkl_scalapack_ilp64.a libmkl_intel_ilp64.a \
libmkl_sequential.a libmkl_core.a libmkl_blacs_intelmpi_ilp64.a \
libmkl_scalapack_ilp64.a libmkl_intel_ilp64.a \
libmkl_sequential.a libmkl_core.a libmkl_blacs_intelmpi_ilp64.a \
libmkl_scalapack_ilp64.a libmkl_intel_ilp64.a \
libmkl_sequential.a libmkl_core.a libmkl_blacs_intelmpi_ilp64.a
#
LIBS=libdyna.a liblsmpp.a libmpp_lanczos.a libbcsext4.a liblsda.a liblsdb.a liblssecurity.a  liblcpack.a libmf2.a libspooles.a liblcpack.a libspooles.a libcparse.a liblsm.a liblscrypt.a libresurf.a liblsmppdes.a libdsa.a liblsmc.a libbalance.a libsfg.a   libzmumps.a libdmumps.a libmumps_common.a    libmetis.a libim_rotor_dynamics.a  libmcms.a   liblsmpp.a liblso.a libunits.a  libem.a libfemster_wrap.a libfemster_wrap2d.a libfemster_wrap1d.a libfemster.a libfemster2d.a libfemster1d.a libpfem.a libcese.a libchemistry.a libem_tailsit.a libparticles.a libeosfl2.a  libpfem.a libmoving.a libdualcese.a libbattery.a liblsmpp.a libmetis.a libfftw3.a libfftw3_omp.a  libansysl.a   libdyna.a libmetis.a libspooles.a
#

OBJS += $(TEMP_DIR)/init_once.o
OBJS += $(TEMP_DIR)/init_dyn21.o
OBJS += $(TEMP_DIR)/dynm.o
OBJS += $(TEMP_DIR)/banner.o
OBJS += $(TEMP_DIR)/sec.o
OBJS += $(TEMP_DIR)/dyn21.o
OBJS += $(TEMP_DIR)/dyn21umat.o
OBJS += $(TEMP_DIR)/dyn21umats.o
OBJS += $(TEMP_DIR)/dyn21umatv.o
OBJS += $(TEMP_DIR)/dyn21umatc.o
OBJS += $(TEMP_DIR)/dyn21tumat.o
OBJS += $(TEMP_DIR)/dyn21utan.o
OBJS += $(TEMP_DIR)/dyn21ueos.o
OBJS += $(TEMP_DIR)/dyn21cnt.o
OBJS += $(TEMP_DIR)/dyn21ushl.o
OBJS += $(TEMP_DIR)/dyn21usld.o
OBJS += $(TEMP_DIR)/dyn21em.o
OBJS += $(TEMP_DIR)/dyn21icfd.o
OBJS += $(TEMP_DIR)/couple2other_user.o
OBJS += $(TEMP_DIR)/dynrfn_user.o
OBJS += $(TEMP_DIR)/adummy_graph.o
OBJS += $(TEMP_DIR)/orderByMetis.o
OBJS += $(TEMP_DIR)/mod_impl_matrices.o
OBJS += $(TEMP_DIR)/userLE_JPCG.o
OBJS += $(TEMP_DIR)/JPCG.o
OBJS += $(TEMP_DIR)/gen_signature.o
OBJS += $(TEMP_DIR)/xFPGA.o
OBJS += $(TEMP_DIR)/xcl2.o
OBJS += $(TEMP_DIR)/cgHost.o

$(TEMP_DIR):
	mkdir -p $(TEMP_DIR)
	mkdir -p $(BUILD_DIR)
	cd $(TEMP_DIR) && \
	cp -r $(LS_DYNA_PATH)/usermat/* ./
	rm -rf Makefil
	rm -rf JPCG.*
	rm -rf mppdyna*
	cd ..

$(BUILD_DIR)/mppdyna: $(TEMP_DIR) $(OBJS)
	set PATH=$(TEMP_DIR):$(PATH)
	cd $(TEMP_DIR) && \
	$(LD) -o $(BUILD_DIR)/mppdyna $(LDFLAGS) $(OBJS) $(LIBS) $(LMATH) $(LF)
	cd ..

run: $(BUILD_DIR)/mppdyna
	cd $(BUILD_DIR) && \
	./mppdyna i=$(LS_DYNA_PATH)/run/small_tbc/small_tbc.k memory=1G ncpu=1 impcon=1
	cd ..

clean:
	rm -rf $(TEMP_DIR)
	rm -rf $(BUILD_DIR)
	rm -rf *.mod

$(TEMP_DIR)/gen_signature.o:
	$(CC) -c $(CF) $(CXXFLAGS) $(XFLIB_DIR)/L2/sparse/src/sw/c++/gen_signature.cpp  -o '$@'
$(TEMP_DIR)/xFPGA.o:
	$(CC) -c $(CF) $(CXXFLAGS) $(XFLIB_DIR)/L2/common/src/sw/xFPGA.cpp -o '$@'
$(TEMP_DIR)/xcl2.o:
	$(CC) -c $(CF) $(CXXFLAGS) $(XFLIB_DIR)/L2/common/src/sw/xcl2.cpp -o '$@'
$(TEMP_DIR)/cgHost.o:
	$(CC) -c $(CF) $(CXXFLAGS) $(XFLIB_DIR)/pcg/src/sw/c++/cgHost.cpp -o '$@'
$(TEMP_DIR)/JPCG.o:
	$(CC) -c $(CF) $(CXXFLAGS) ../../usermat/JPCG.cpp -o '$@'
$(TEMP_DIR)/userLE_JPCG.o:
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/userLE_JPCG.F90 -o '$@'
$(TEMP_DIR)/init_dyn21.o: $(TEMP_DIR)/init_dyn21.f $(TEMP_DIR)/nhisparm.inc
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/init_dyn21.f -o '$@'
$(TEMP_DIR)/userinterface.o: $(TEMP_DIR)/userinterface.f90
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/userinterface.f90 -o '$@'
$(TEMP_DIR)/dyn21.o: $(TEMP_DIR)/dyn21.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21.f -o '$@'
$(TEMP_DIR)/dyn21umat.o: $(TEMP_DIR)/dyn21umat.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21umat.f -o '$@'
$(TEMP_DIR)/dyn21umats.o: $(TEMP_DIR)/dyn21umats.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21umats.f -o '$@'
$(TEMP_DIR)/dyn21umatv.o: $(TEMP_DIR)/dyn21umatv.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21umatv.f -o '$@'
$(TEMP_DIR)/dyn21umatc.o: $(TEMP_DIR)/dyn21umatc.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21umatc.f -o '$@'
$(TEMP_DIR)/dyn21tumat.o: $(TEMP_DIR)/dyn21tumat.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21tumat.f -o '$@'
$(TEMP_DIR)/dyn21utan.o: $(TEMP_DIR)/dyn21utan.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21utan.f -o '$@'
$(TEMP_DIR)/dyn21ueos.o: $(TEMP_DIR)/dyn21ueos.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21ueos.f -o '$@'
$(TEMP_DIR)/dyn21cnt.o: $(TEMP_DIR)/dyn21cnt.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21cnt.f -o '$@'
$(TEMP_DIR)/dyn21ushl.o: $(TEMP_DIR)/dyn21ushl.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21ushl.f -o '$@'
$(TEMP_DIR)/dyn21usld.o: $(TEMP_DIR)/dyn21usld.f $(TEMP_DIR)/nhisparm.inc $(TEMP_DIR)/userinterface.o
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21usld.f -o '$@'
$(TEMP_DIR)/dyn21em.o: $(TEMP_DIR)/dyn21em.f
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21em.f -o '$@'
$(TEMP_DIR)/dyn21icfd.o: $(TEMP_DIR)/dyn21icfd.f
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dyn21icfd.f -o '$@'
$(TEMP_DIR)/couple2other_user.o: $(TEMP_DIR)/couple2other_user.f 
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/couple2other_user.f -o '$@'
$(TEMP_DIR)/dynrfn_user.o: $(TEMP_DIR)/dynrfn_user.f 
	$(FC) -c $(FF) -I$(TEMP_DIR) $(TEMP_DIR)/dynrfn_user.f -o '$@'
