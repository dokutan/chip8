CXXFLAGS = -std=c++20 -Wall -Wextra -O3
LDLIBS = -lSDL2 -llua

chip8: src/*
	$(CXX) $(CXXFLAGS) $(LDLIBS) src/chip8.cpp -o chip8	

clean:
	rm -f chip8
