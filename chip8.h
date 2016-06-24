/* Author: Gokul Suresh
 * Description: This is the header file for the chip8 emulator that I worked 
 * on during the summer of 2016 */

/* This header file contains the cpu core implementation for the chip 8
 * emulator */

class chip8 {
	
	unsigned short opcode;		/* used to store the two byte long opcode */
	unsigned char memory[4096];	/* the chip 8 has 4K memory */
	unsigned char V[16];		/* holds the 15 cpu registers + carry flag */
	unsigned short I;			/* the Index register */
	unsigned short pc;			/* the program counter */
	
	unsigned char delay_timer;	/* the delay timer */
	unsigned char sound_timer;	/* the sound timer */

	unsigned short stack[16];	/* the RTS */
	unsigned short sp;			/* the stack pointer */

	

	

public:
	void initialize();
    unsigned char gfx[2048];	/* graphics array */
    unsigned char oldgfx[2048];  /* used to compare with new gfx when drawing */
	void emulateCycle();
	static void Set_Debug_On(void);
	static void Set_Debug_Off(void);
	static int debug;			/* indicates if debug mode is on or off */
	void loadGame(const char * filename);	/* loads the game */
    long drawFlag;				/* indicates whether the screen needs to be
                                 drawn or not */
    unsigned short key[16];		/* used to implement the hex based keypad
                                 for chip 8 */

	
};

int chip8::debug;
