BLAS_ddrWidth=16 
BLAS_XddrWidth=16 
BLAS_argInstrWidth=1 

BLAS_dataType=float 
BLAS_gemmMBlocks=1
BLAS_gemmKBlocks=2 
BLAS_gemmNBlocks=1 

BLAS_argPipeline        = 2
BLAS_instructionSizeBytes = 64
BLAS_numKernels         = 3 

BLAS_dataEqIntType = float
BLAS_XdataType     = float 
BLAS_argInstrWidth =   1
BLAS_numInstr      =  64
TEST_MEMCPY        = 0
BLAS_CACHE         = 1
BLAS_XVEC          = 1

#MACROS += -D MLP_TANSIG=1
#MACROS += -D MLP_RELU=1

MACROS += -D TEST_MEMCPY=$(TEST_MEMCPY) \
          -D BLAS_instructionSizeBytes=$(BLAS_instructionSizeBytes) \
          -D BLAS_dataType=$(BLAS_dataType) \
          -D BLAS_dataEqIntType=$(BLAS_dataEqIntType) \
          -D BLAS_ddrWidth=$(BLAS_ddrWidth) \
          -D BLAS_argInstrWidth=$(BLAS_argInstrWidth) \
          -D BLAS_numInstr=$(BLAS_numInstr) \
          -D BLAS_argPipeline=$(BLAS_argPipeline) \
          -D BLAS_runTransp=0 \
          -D BLAS_runGemv=0 \
          -D BLAS_runGemm=0 \
          -D BLAS_runFcn=1 \
          -D BLAS_numKernels=${BLAS_numKernels}\
          -D BLAS_CACHE=${BLAS_CACHE}\
          -D BLAS_XVEC=${BLAS_XVEC} \
          -D BLAS_gemmMBlocks=${BLAS_gemmMBlocks} \
          -D BLAS_gemmKBlocks=${BLAS_gemmKBlocks} \
          -D BLAS_gemmNBlocks=${BLAS_gemmNBlocks} \
          -D BLAS_XdataType=$(BLAS_XdataType) \
          -D BLAS_XddrWidth=$(BLAS_XddrWidth) \
          
CXXFLAGS += ${MACROS}
VPP_FLAGS += ${MACROS}

CONFIG_INFO = $(shell echo ${MACROS} | sed 's/-D //g; s/ -Wno.*//')

dump_config: 
	@echo ${CONFIG_INFO}  | tr " " "\n" > ${BUILD_DIR}/config_info.dat
