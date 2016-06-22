#include "chip8.h"	/* the cpu core implementation */
#include <iostream>
#include <stdio.h>
#include <SDL.h>

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
}

void chip8::loadGame(const char * filename) {

	long bufferSize;
	FILE * pFile;

	// opening the file in read-only binary mode
	pFile = fopen (filename, "rb");

	if (pFile == NULL) {
		cerr << "ERROR READING FILE\n";
		exit(0);
	}

	// check the size of the file
	fseek(pFile , 0 , SEEK_END);
	bufferSize = ftell(pFile);
	rewind(pFile);

	// dynamically allocate memory for the buffer
	char * buffer = (char*)malloc(sizeof(char) * bufferSize);

	// copy the file into the buffer
	fread(buffer, 1, bufferSize, pFile);
	
	// copy the buffer into memory
	for (int i = 0; i < bufferSize; i++) {
		memory[i + 512] = buffer[i];
	}
	
	//close the file
	fclose (pFile);

	// deallocate memory for the buffer
	free(buffer);
}

void chip8::emulateCycle() {

	// fetch opcode
	opcode = (memory[pc] << 8) | memory[pc + 1];

	// decode opcode
	switch(opcode & 0xF000) {

		case 0x0000: // multi-case first nibble

			switch (opcode & 0x00FF) {

				case 0x00E0: // 00E0: clear screen
					if (debug)
						cerr << opcode << ": clearing the screen\n";
					for (int i = 0; i < 2048; i++) 
						gfx[i] = 0x0;
					drawFlag = true;
					pc += 2;
				break;
				
				case 0x00EE: // 00EE: return from subroutine
					--sp;
					pc = stack[sp];
					pc += 2;

					if (debug)
						cerr << opcode << ": returning from subroutine\n";

				break;

				default: // 0NNN: calls RCA 1802 program at address NNN
					cerr << opcode << " not implemented\n";
					exit(0);
			}
		break;	

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
			int x = (opcode & 0x0F00) >> 8;

			// checking if V[X] == NN
			if (V[x] == (opcode & 0x00FF)) { 
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

		}
		break;

		case 0x4000: // 4XNN: skips the next instruction if VX doesn't equal NN

			if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
				pc += 4;
				if (debug)
					cerr << opcode << ": skipping next instruction\n";
			}

			else {
				pc += 2;
				if (debug)
					cerr << opcode << ": not skipping next instruction\n";
			}

		break;	

		case 0x5000: //5XY0: skips the next instruction if VX equals VY
			
			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
				pc += 4;
				if (debug) 
					cerr << opcode << ": skipping next instruction\n";
			}

			else {
				pc += 2;
				if (debug)
					cerr << opcode << ": not skipping next instruction\n";
			}
		break;	

		case 0x6000: // 6XNN: set VX to NN
			
			if (debug)
				cerr << opcode << ": setting VX to NN\n";

			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
		break;
		
		case 0x7000: // 7XNN: adds NN to VX
			
			if (debug)
				cerr << opcode << ": adding NN to VX\n";

			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;	

			pc += 2;
		break;

		case 0x8000: // contains more information in last nibble
			switch (opcode & 0x000F) {
				
				case 0x0000: // 0x8XY0: sets VX to the value of VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;

					if (debug)
						cerr << opcode << ": setting VX to VY\n";
				break;

				case 0x0001: // 8XY1: Sets VX to (VX or VY)
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					if (debug)
						cerr << opcode << ": Setting VX to (VX OR VY)\n";
					pc += 2;
				break;	

				case 0x0002: // 8XY2: Sets VX to (VX and VY)
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					if (debug)
						cerr << opcode << ": Setting VX to (VX AND VY)\n";
					pc += 2;
				break;

				case 0x0003: // 8XY3: Sets VX to (VX XOR VY)
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					if (debug)
						cerr << opcode << ": Setting VX to (VX XOR VY)\n";
					pc += 2;
				break;
					
				case 0x0004: // 8XY4: Adds VY to VX. Includes carry	if necessary
					if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode 
						& 0x0F00) >> 8])) {
						V[0xF] =  1;	// carry set to 1
					}

					else 
						V[0xF] = 0;		// carry set to 0
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;		

				case 0x0005: // 8XY5: VY is subtracted from VX
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
						V[0xF] = 0;
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					if (debug)
						cerr << opcode << ": subtracting VY from VX\n";
					pc += 2;	
				break;

				case 0x0006: // 8XY6: shifts VX right by one
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
					if (debug)
						cerr << opcode << ": shifting VX right by one\n";
				break;

				case 0x0007: // 8XY7: Sets VX to VY minus VX
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
						V[0xF] = 0;
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
					if (debug)
						cerr << opcode << ": V[X] = V[Y] - V[X]\n";
				break;		
					
				default:
					cerr << "Unknown opcode: " << opcode << "\n";
					exit(0);
			}
		break;	

		case 0xA000: // ANNN: sets I to the address NNN
			if (debug)
				cerr << opcode << ": setting I to address NNN\n";

			I = opcode & 0x0FFF;
			pc += 2;
		break;

		case 0xC000: // CXNN: Sets VX to a random number and NN
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
			if (debug)
				cerr << opcode << ": setting VX to a random number\n";
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

		case 0xE000:	// multi-case opcode
			switch (opcode & 0x00FF) {

				// TODO
			}

		case 0xF000:	// multi-case opcode
			
			switch(opcode & 0x00FF) {

				case 0x0007:	// FX07: sets V[x] to the value of delay timer
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					if (debug)
						cerr << opcode << ": setting V[x] to value of delay timer\n";
					pc += 2;
				break;		

				case 0x0015:	// FX15: sets the delay timer to V[x]
					delay_timer = V[(opcode & 0x0F00) >> 8];
					if (debug)
						cerr << opcode << ": setting delay timer to V[x]\n";
					pc += 2;
				break;	

				case 0x0029: {	// FX29: sets I to the location of the sprite
					int x = (opcode & 0x0F00) >> 8;
					unsigned char character = V[x];
					I = (0x050 + (character*5));
					if (debug) 
						cerr << opcode << ": setting I to location of sprite\n";
					pc += 2;
				}
				break;

				case 0x0033: {	// Stores a binary coded decimal value VX in I
								// I+1, and I+2
					int x = (opcode & 0x0F00) >> 8;
					int value = V[x];
					int hundreds = (value- (value % 100)) / 100;
				    value -= hundreds * 100;
					int tens = (value - (value % 10)) / 10;
					value -= tens * 10;
					memory[I] = (char)hundreds;
					memory[I + 1] = (char)tens;
					memory[I + 2] = (char)value;
					if (debug) {
						cerr << opcode << ": storing binary coded decimal value"
						<< " in I, I+1, I+2\n";
					}
					pc += 2;
				}
				break;

				case 0x0065: {	// fills V0 to VX with values from I
					int x = (opcode & 0x0F00) >> 8;
					for (int i = 0; i < x; i++) {
						V[i] = memory[I + i];
					}

					if (debug) {
						cerr << opcode << ": filling V0 to VX with values from "
						<< "I\n";
					}

					pc += 2;
				}
				break;

				default:
					cerr << "Unknown opcode: " << opcode << "\n";
					exit(0);

			}
		break;

		default:
			cerr << "Unknown opcode: " << opcode << "\n";
			exit(0);
	}

	// update timers
	if (delay_timer > 0) {
		--delay_timer;
	}

	if (sound_timer > 0) {
		if (sound_timer == 1)
			cout << "BEEP!\n";
		--sound_timer;
	}
}

int main(int argc, char ** argv) {
	char option;

	chip8::Set_Debug_On();

	// checking if debug mode (x) was passed as a command line parameter
	while ((option = getopt (argc, argv, "x")) != EOF) {

		switch (option) {
			case 'x': chip8::Set_Debug_On();
					  break;
		}
	}
    
    // The SDL window
    SDL_Window * window = NULL;
    
    // The surface contained by the window
    SDL_Surface * screenSurface = NULL;
    
    // Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    
    else {
        // Create the window
        window = SDL_CreateWindow("GS8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
        
        if (window == NULL) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        
        else {
            // Get the window surface
            screenSurface = SDL_GetWindowSurface(window);
            
            // Fill the surface white
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
            
            //Update the surface
            SDL_UpdateWindowSurface( window );
            
            //Wait two seconds
            SDL_Delay( 2000 );
        }
    }
	
	// initialize the chip
	myChip8.initialize();
	myChip8.loadGame("PONG2");
	
	// emulation loop
	while (true) {
		myChip8.emulateCycle();

	}
    
    
    

}

