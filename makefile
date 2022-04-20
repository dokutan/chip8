CXXFLAGS = -std=c++20 -Wall -Wextra
LDLIBS = -lSDL2

chip8: src/*
	$(CXX) $(CXXFLAGS) $(LDLIBS) src/chip8.cpp -o chip8	

clean:
	rm -f chip8
