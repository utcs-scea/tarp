GASNET_PKG_CONFIG_PATH ?= ${WORK}/mono-install/lib/pkgconfig/
GASNET_DEBUG_PKG_CONFIG_PATH ?= ${WORK}/debug-install/lib/pkgconfig/

GASNET_VERSION ?= gasnet-ibv-par

GASNET_PKG_CONFIG_INCLUDE ?= $(shell PKG_CONFIG_PATH=${GASNET_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config --cflags ${GASNET_VERSION})
GASNET_PKG_CONFIG_LIBS ?= $(shell PKG_CONFIG_PATH=${GASNET_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config --libs --cflags ${GASNET_VERSION})

GASNET_DEBUG_PKG_CONFIG_INCLUDE ?= $(shell PKG_CONFIG_PATH=${GASNET_DEBUG_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config --cflags ${GASNET_VERSION})
GASNET_DEBUG_PKG_CONFIG_LIBS ?= $(shell PKG_CONFIG_PATH=${GASNET_DEBUG_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config --libs --cflags ${GASNET_VERSION})

GASNET_CXX ?= $(shell PKG_CONFIG_PATH=${GASNET_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config ${GASNET_VERSION} --variable=GASNET_CXX)

GASNET_LD ?= $(shell PKG_CONFIG_PATH=${GASNET_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config ${GASNET_VERSION} --variable=GASNET_LD)

DEBUG_GASNET_CXX ?= $(shell PKG_CONFIG_PATH=${GASNET_DEBUG_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config ${GASNET_VERSION} --variable=GASNET_CXX)

DEBUG_GASNET_LD ?= $(shell PKG_CONFIG_PATH=${GASNET_DEBUG_PKG_CONFIG_PATH}:${PKG_CONFIG_PATH} pkg-config ${GASNET_VERSION} --variable=GASNET_LD)

PMIX_LIBRARY_PATH ?= /opt/apps/pmix/3.2.3/lib
IMPI_LIBRARY_PATH ?= /opt/intel/compilers_and_libraries_2020.4.304/linux/mpi/intel64/lib

INCLUDE_DIR ?= $(realpath include)
SRC_DIR ?= src

all-test: gasnet-test debug-gasnet-test

debug-tarp.o: src/init_tarp.cpp include/tarp.hpp
	${DEBUG_GASNET_CXX} -std=c++2b -g -O0 -c -o $@ $< ${GASNET_DEBUG_PKG_CONFIG_INCLUDE} -I${INCLUDE_DIR}

debug-gasnet-test.o: test/e2e-test/gasnet-test.cpp
	${DEBUG_GASNET_CXX} -std=c++2b -g -O0 -c -o $@ $? ${GASNET_DEBUG_PKG_CONFIG_INCLUDE} -I${INCLUDE_DIR}

debug-gasnet-test: debug-gasnet-test.o debug-tarp.o
	${DEBUG_GASNET_LD} -std=c++2b -g -O0 -o $@ $? ${GASNET_DEBUG_PKG_CONFIG_LIBS} -lstdc++

tarp.o: src/init_tarp.cpp include/tarp.hpp
	${GASNET_CXX} -std=c++2b -c -o $@ $< ${GASNET_PKG_CONFIG_INCLUDE} -I${INCLUDE_DIR}

gasnet-test.o: test/e2e-test/gasnet-test.cpp
	${GASNET_CXX} -std=c++2b -c -o $@ $? ${GASNET_PKG_CONFIG_INCLUDE} -I${INCLUDE_DIR}

gasnet-test: gasnet-test.o tarp.o
	${GASNET_LD} -std=c++2b -O3 -o $@ $? ${GASNET_PKG_CONFIG_LIBS} -lstdc++

clean:
	rm -f *.ll *.out *.o gasnet-test debug-gasnet-test
