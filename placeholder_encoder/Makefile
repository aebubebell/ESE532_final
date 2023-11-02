CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -g -DNDEBUG -pthread
LDFLAGS = -lpthread -pthread

EXECUTABLES = cdc

CDC_SRC = $(wildcard ./CDC.cpp)

.PHONY: clean

cdc: $(CDC_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) -rf perf* *.bin *.o *~ $(EXECUTABLES)
