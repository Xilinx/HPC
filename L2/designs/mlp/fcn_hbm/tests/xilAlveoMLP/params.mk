LIB_DIR = ${CUR_DIR}/lib
pythonApi: host
	@mkdir -p ${LIB_DIR}
	@mv ${EXE_FILE} ${LIB_DIR}
	@rm -rf ${BUILD_DIR}

