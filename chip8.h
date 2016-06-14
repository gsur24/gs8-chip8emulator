/* This header file contains the cpu core implementation for the chip 8
 * emulator */

class chip8 {
	
	unsigned short opcode;		/* used to store the two byte long opcode */
	unsigned char memory[4096];	/* the chip 8 has 4K memory */
	unsigned char V[16];		/* holds the 15 cpu registers + carry flag */
	unsigned short I;			/* the Index register */
	unsigned short pc;			/* the program counter */

public:
	void initialize();
	
};
