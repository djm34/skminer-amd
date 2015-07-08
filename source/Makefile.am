
ACLOCAL_AMFLAGS = -I m4


EXTRA_DIST	= example.conf m4/gnulib-cache.m4 \
		  ADL_SDK/readme.txt api-example.php miner.php	\
		  API.class API.java api-example.c hexdump.c \
		  doc/API doc/FAQ doc/GPU doc/SCRYPT doc/windows-build.txt


bin_PROGRAMS     = skminer

skminer_CPPFLAGS =  -std=c++0x  $(BOOST_FLAGS)
skminer_LDFLAGS  = $(BOOST_FLAGS)
skminer_LDADD    = $(DLOPEN_FLAGS)  @JANSSON_LIBS@  @BOOST_LIB@ \
		  @OPENCL_LIBS@ @WS2_LIBS@ \
		  @MM_LIBS@ @RT_LIBS@ @MATH_LIBS@ 

skminer_CPPFLAGS +=  @OPENCL_FLAGS@

skminer_CPPFLAGS += $(ADL_CPPFLAGS)

if USE_GIT_VERSION
GIT_VERSION      := $(shell sh -c 'git describe --abbrev=4 --dirty')
skminer_CPPFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

skminer_SOURCES	= app/Application.cpp app/Application.h app/main.cpp app/main.h \
	 base/IPrototype.h \
	 compute/BaseComputeDevice.cpp compute/BaseComputeDevice.h compute/CLDevice.cpp compute/CLDevice.h \
	 compute/CLFuncs.cpp compute/CLFuncs.h compute/CLKernel.cpp compute/CLKernel.h compute/CLMemory.cpp \
	 compute/CLMemory.h compute/CLStructs.h \
	 config/ConfigConnection.cpp config/ConfigConnection.h config/GPUSetting.cpp config/GPUSetting.h \
	 config/RunOptions.cpp config/RunOptions.h \
	 connections/ServerConnection.cpp connections/ServerConnection.h connections/SKServerConnection.cpp \
	 connections/SKServerConnection.h \
	 zcore/bignum.h zcore/CBlock.cpp zcore/CBlock.h zcore/Connection.cpp zcore/Connection.h zcore/DDOS_Filter.cpp \
	 zcore/DDOS_Filter.h zcore/DDOS_Score.cpp zcore/DDOS_Score.h zcore/Inbound.cpp zcore/Inbound.h zcore/Miner.cpp \
	 zcore/Miner.h zcore/Outbound.cpp zcore/Outbound.h zcore/Packet.cpp zcore/Packet.h zcore/Timer.cpp \
	 zcore/Timer.h zcore/types.h zcore/util.h \
	 data/MinerData.cpp data/MinerData.h data/SKMinerData.cpp data/SKMinerData.h \
	 driver/ADL.cpp driver/ADLFuncs.h driver/ADL.h \
	 gpu/ADLGPU.cpp gpu/ADLGPU.h gpu/BaseGPU.cpp gpu/BaseGPU.h gpu/GPUData.cpp gpu/GPUData.h \
	 hash/brg_endian.h hash/brg_types.h hash/skein_port.h hash/uint1024.h \
	 kernel/KernelFuncs.cpp kernel/KernelFuncs.h \
	 threads/MinerThread.cpp threads/MinerThread.h threads/SKMinerThread.cpp threads/SKMinerThread.h \
	 ui/Section.cpp ui/Section.h ui/UIData.cpp ui/UIData.h \
	 utilities/JSONUtils.cpp utilities/JSONUtils.h utilities/StringUtils.cpp utilities/StringUtils.h 



