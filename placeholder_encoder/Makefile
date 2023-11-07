CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -g -DNDEBUG -pthread
LDFLAGS = -lpthread -pthread

EXECUTABLES = cdc cmd lzw

CDC_SRC = $(wildcard ./CDC.cpp)
CMD_SRC = $(wildcard ./CDC.cpp ./CMD.cpp ./sha256.c)
LZW_SRC = $(wildcard ./CDC.cpp ./CMD.cpp ./LZW.cpp ./sha256.c)

.PHONY: clean

cdc: $(CDC_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

cmd: $(CMD_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)
lzw: $(LZW_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)
clean:
	$(RM) -rf perf* *.bin *.o *~ $(EXECUTABLES)
