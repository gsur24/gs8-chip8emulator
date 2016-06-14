all: chip8.exe

chip8.exe: chip8.o
	gcc -o chip8.exe chip8.o

chip8.o: chip8.cpp
	gcc -c chip8.cpp

clean:
	rm *~
	rm chip8.o chip8.exe

