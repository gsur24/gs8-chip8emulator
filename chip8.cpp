#include "chip8.h"	/* the cpu core implementation */
#include <SDL2/SDL.h>
#include <iostream>
using namespace std;

chip8 myChip8;

void chip8::Set_Debug_On(void) {
	debug = true;
}

void chip8::Set_Debug_Off(void) {
	debug = false;
}

void chip8::initialize() {
	pc = 0x200;	// the program counter starts at 0x200
	opcode = 0;	// reset the opcode
	I = 0;		// reset the index register
	sp = 0;		// reset the stack pointer



	// load the fontset
	// for (int i = 0; i < 80; i++) {
	// 	memory[i] = chip8_fontset[i];
	// }
}

void chip8::emulateCycle() {

	// fetch opcode
	opcode = (memory[pc] << 8) | memory[pc + 1];

	// decode opcode
	switch(opcode & 0xF000) {

		case 0x0000: // multi-case first nibble

			switch (opcode & 0x00FF) {

				case 0x00E0: // 00E0: clear screen
					// TODO
				
				case 0x00EE: // 00EE: return from subroutine
					sp--;
					pc = stack[sp];

					if (debug)
						cerr << opcode << ": returning from subroutine\n";

					break;

				default: // 0NNN: calls RCA 1802 program at address NNN
					break;
			}

		case 0x1000: // 1NNN: jumps to address NNN
			pc = opcode & 0x0FFF;

			if (debug)
				cerr << opcode << ": jumping to address NNN\n";

			break;

		case 0x2000: // 2NNN: call subroutine at NNN
			stack[sp++] = pc;	
			pc = opcode & 0x0FFF;

			if (debug)
				cerr << opcode << ": calling subroutine at NNN\n";

			break;

		case 0x3000: { // 3XNN: skips the next instruction if VX equals NN
			unsigned short x = (opcode & 0x0FF) >> 8;
			unsigned short nn = (opcode & 0x00FF);

			// checking if V[X] == NN
			if (V[x] == nn) { 
				if (debug)
					cerr << opcode << ": skipping next instruction\n";

				// skip the next instruction
				pc += 4;
			}

			else {
				if (debug)
					cerr << opcode << ": not skipping next instruction\n";
				
				pc += 2;
			}

			break;
		}
		case 0x6000: // 6XNN: set VX to NN
			
			if (debug)
				cerr << opcode << ": setting VX to NN\n";

			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
			break;
		
		case 0x7000: // 7XNN: adds NN to VX
			
			if (debug)
				cerr << opcode << ": adding NN to VX\n";

			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] + (opcode & 
			0x00FF)) & 0xFF; 

			pc += 2;
			break;

		case 0x8000: // contains more information in last nibble
			switch (opcode & 0x000F) {
				
				case 0x0000: // 0x8XY0: sets VX to the value of VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 8];
					pc += 2;

					if (debug)
						cerr << opcode << ": setting VX to VY\n";
					break;
				
				default:
					cerr << "Unknown opcode: " << opcode << "\n";
					break;
			}

		case 0xA000: // ANNN: sets I to the address NNN
			if (debug)
				cerr << opcode << ": setting I to address NNN\n";

			I = opcode & 0x0FFF;
			pc += 2;
			break;
		
		case 0xD000:  { // DXYN: Draw a sprite (X, Y) size (8, N) at location I
			unsigned short x = V[(opcode & 0x0F00) >> 8];	// x-coordinate
			unsigned short y = V[(opcode & 0x00F0) >> 4];	// y-coordinate
			unsigned short height = opcode & 0x000F;		// height
			unsigned short pixel;

			V[0xF] = 0;

			for (int yline = 0; yline < height; yline++) {
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++) {

					if((pixel & (0x80 >> xline)) != 0) {
						if(gfx[(x + xline + ((y + yline) * 64))] == 1) 
							V[0xF] = 1;

						gfx[x + xline + ((y + yline) * 64)] ^= 1;	
					}
				}
			}

			// screen now needs to be redrawn
			drawFlag = true;
			pc += 2;
			break;
		}

		default:
			cerr << "Unknown opcode: " << opcode;
			exit(0);
	}

	// execute opcode
}

int main(int argc, char ** argv) {
	char option;

	chip8::Set_Debug_Off();

	while ((option = getopt (argc, argv, "x")) != EOF) {

		switch (option) {
			case 'x': chip8::Set_Debug_On();
				break;
		}
	}

	// Set up the render system and register input callbacks
	// setupGraphics();
	// setupInput();
	// Initialize the Chip8 system and load the game into memory
	// myChip8.initialize();
	// myChip8.loadGame("pong");
}

