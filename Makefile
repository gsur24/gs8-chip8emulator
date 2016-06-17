all: chip8.o

chip8.o: chip8.cpp
	g++ -o chip8 chip8.cpp 
	chmod u+x chip8

clean:
	rm -f chip8 *~


