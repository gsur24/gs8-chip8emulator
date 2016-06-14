#include "chip8.h"	/* the cpu core implementation */

chip8 myChip8;

int main(int argc, char ** argv) {

	// Set up the render system and register input callbacks
	setupGraphics();
	setupInput();

	// Initialize the Chip8 system and load the game into memory
	myChip8.initialize();
	myChip8.loadGame("pong");
}


void chip8::initialize() {
	pc = 0x200;	// the program counter starts at 0x200
	opcode = 0;	// reset the opcode
	I = 0;		// reset the index register
	sp = 0;		// reset the stack pointer



	// load the fontset
	for (int i = 0; i < 80; i++) {
		memory[i] = chip8_fontset[i];
	}
}
