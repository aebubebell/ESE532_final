CXX = aarch64-linux-gnu-g++
CXXFLAGS = -std=c++11 -Wall -O3 -g -DNDEBUG -pthread
LDFLAGS = -lpthread -pthread

ARCH_FLAGS := -march=armv8-a+crc+crypto

EXECUTABLES = cdc cmd lzw encoder

CDC_SRC = $(wildcard ./CDC.cpp)
CMD_SRC = $(wildcard ./CDC.cpp ./CMD.cpp ./sha256.c)
LZW_SRC = $(wildcard ./CDC.cpp ./CMD.cpp ./LZW.cpp ./sha256.c)
ENCODER_SRC = $(wildcard ./encoder.cpp ./CDC.cpp ./CMD.cpp ./LZW.cpp ./sha256.c)

.PHONY: clean

cdc: $(CDC_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

cmd: $(CMD_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)
lzw: $(LZW_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)
encoder: $(ENCODER_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ARCH_FLAGS) $^ -o $@ $(LDFLAGS)
clean:
	$(RM) -rf perf* *.bin *.o *~ $(EXECUTABLES)
