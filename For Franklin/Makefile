#######################################################################################
.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo ""
	@echo "  make fpga "
	@echo "      Command to build mmult_fpga."
	@echo ""
	@echo "  make host "
	@echo "      Command to build only the host program of mmult_fpga."
	@echo ""
	@echo "  make clean "
	@echo "      Command to remove the generated files."
	@echo ""
#######################################################################################

HOST_CXX = aarch64-linux-gnu-g++
VPP = ${XILINX_VITIS}/bin/v++
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = u96v2_sbc_base
VITIS_PLATFORM_DIR = ${PLATFORM_REPO_PATHS}
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM_DIR)/u96v2_sbc_base.xpfm

# host compiler global settings
CXXFLAGS += -march=armv8-a+simd -mtune=cortex-a53 -std=c++11 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_VIVADO)/include/ -I$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux/usr/include/xrt/ -O3 -g -Wall -c -fmessage-length=0 --sysroot=$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux
LDFLAGS += -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ -L$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux/usr/lib/ --sysroot=$(VITIS_PLATFORM_DIR)/sw/$(VITIS_PLATFORM)/PetaLinux/sysroot/aarch64-xilinx-linux
ARCH_FLAGS := -march=armv8-a+crc+crypto
# hardware compiler shared settings
VPP_OPTS = --target hw


#
# OpenCL kernel files
#
XO :=encoding_4chunks.xo
XCLBIN := encoding_4chunks.xclbin
ALL_MESSAGE_FILES = $(subst .xo,.mdb,$(XO)) $(subst .xclbin,.mdb,$(XCLBIN))

#
# host files
#
HOST_SOURCES = hls/Host.cpp common/EventTimer.cpp common/Utilities.cpp hls/CDC.cpp hls/CMD.cpp hls/sha256.c hls/utility.h Server/server.cpp common/Utilities.h hls/stopwatch.h hls/LZW.cpp
HOST_OBJECTS =$(HOST_SOURCES:.cpp=.o)
HOST_EXE = host

.PHONY: cpu

$(HOST_EXE): $(HOST_OBJECTS)
	$(HOST_CXX) $(ARCH_FLAGS) -O3 -o "$@" $(+) $(LDFLAGS)

.cpp.o:
	$(HOST_CXX) $(CXXFLAGS) -I./hls/ -I./common -o  "$@" "$<"

#
# primary build targets
#

PHONY: fpga clean
fpga: package/sd_card.img

.NOTPARALLEL: clean

clean-host:
	-$(RM) $(HOST_EXE) $(HOST_OBJECTS) 
	
clean-accelerators:
	-$(RM) $(XCLBIN) $(XO) $(ALL_MESSAGE_FILES)
	-$(RM) *.xclbin.sh *.xclbin.info *.xclbin.link_summary* *.compile_summary
	-$(RMDIR) .Xil hls/proj_mmult

clean-package:
	-${RMDIR} package
	-${RMDIR} package.build

#
 binary container: encoding_4chunks.xclbin
#

$(XO): hls/LZW.cpp
	-@$(RM) $@
	$(VPP) $(VPP_OPTS) -k encoding_4chunks --compile -I"$(<D)" --config design/design.cfg -o"$@" "$<"

$(XCLBIN): $(XO)
	$(VPP) $(VPP_OPTS) --link --config design/design.cfg -o"$@" $(+)

package/sd_card.img: $(HOST_EXE) $(XCLBIN) ./design/xrt.ini
	$(VPP) --package $(VPP_OPTS) --config design/package.cfg $(XCLBIN) \
		--package.out_dir package \
		--package.sd_file $(HOST_EXE) \
		--package.kernel_image $(PLATFORM_REPO_PATHS)/sw/$(VITIS_PLATFORM)/PetaLinux/image/image.ub \
		--package.rootfs $(PLATFORM_REPO_PATHS)/sw/$(VITIS_PLATFORM)/PetaLinux/rootfs/rootfs.ext4 \
		--package.sd_file $(XCLBIN) \
		--package.sd_file ./design/xrt.ini
