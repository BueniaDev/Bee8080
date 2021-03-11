#ifndef BEE8080_H
#define BEE8080_H

#include <iostream>
#include <cstdint>
using namespace std;

namespace bee8080
{
    // Interface between emulated 8080 and any emulated memory/peripherals
    class Bee8080Interface
    {
	public:
	    Bee8080Interface();
	    ~Bee8080Interface();

	    // Reads a byte from memory
	    virtual uint8_t readByte(uint16_t addr) = 0;
	    // Writes a byte to memory
	    virtual void writeByte(uint16_t addr, uint8_t val) = 0;
	    // Reads a byte from an I/O port
	    virtual uint8_t portIn(uint8_t port) = 0;
	    // Writes a byte to an I/O port
	    virtual void portOut(uint8_t port, uint8_t val) = 0;
    };

    // Class for 8080's internal registers
    class Bee8080Register
    {
	public:
	    Bee8080Register();
	    ~Bee8080Register();

	    // Fetches value of 16-bit register
	    uint16_t getreg();
	    // Sets value of 16-bit register
	    void setreg(uint16_t val);

	    // Fetches value of 8-bit high register
	    uint8_t gethi();
	    // Sets value of 8-bit high register
	    void sethi(uint8_t val);

	    // Fetches value of 8-bit low register
	    uint8_t getlo();
	    // Sets value of 8-bit low register
	    void setlo(uint8_t val);

	private:
	    // Private declarations of 8-bit registers
	    uint8_t hi = 0;
	    uint8_t lo = 0;
    };

    // Class for the actual 8080 emulation logic
    class Bee8080
    {
	public:
	    Bee8080();
	    ~Bee8080();

	    // Register declarations
	    Bee8080Register af; // AF (accumulator and flags registers, aka. PSW)
	    Bee8080Register bc; // BC (B and C registers)
	    Bee8080Register de; // DE (D and E registers)
	    Bee8080Register hl; // HL (H and L registers)
	    uint16_t pc; // Program counter
	    uint16_t sp; // Stack pointer

	    // Initializes the CPU
	    // Takes an optional argument to set the initial value of the program counter
	    void init(uint16_t init_pc = 0);

	    // Stops the emulated CPU
	    void shutdown();

	    // Sets a custom interface for the emulated 8080
	    void setinterface(Bee8080Interface *cb);

	    // Runs the CPU for one instruction
	    int runinstruction();

	private:
	    // Private declaration of interface class
	    Bee8080Interface *inter = NULL;

	    // Contains the main logic for the 8080 instruction set
	    int executenextopcode(uint8_t opcode);

	    // Prints the unrecognized instruction and then exits
	    void unrecognizedopcode(uint8_t opcode);

	    // Internal functions for memory and I/O access

	    // Reads byte from memory
	    uint8_t readByte(uint16_t addr);
	    // Writes byte to memory
	    void writeByte(uint16_t addr, uint8_t val);

	    // Reads 16-bit word from memory
	    uint16_t readWord(uint16_t addr);
	    // Writes 16-bit word to memory
	    void writeWord(uint16_t addr, uint16_t val);

	    // Fetches next byte from memory (and updates the program counter)
	    uint8_t getimmByte();

	    // Fetches next word from memory (and updates the program counter)
	    uint16_t getimmWord();

	    // Reads byte from I/O port
	    uint8_t portIn(uint8_t port);

	    // Writes byte to I/O port
	    void portOut(uint8_t port, uint8_t val);

	    // Logic code for jump instructions
	    int jump(uint16_t val, bool cond = true);

	    // Logic code for call instructions
	    int call(bool cond = true);

	    // Logic code for return instructions
	    int ret();

	    // Logic code for conditional return instructions
	    int ret_cond(bool cond = true);

	    // Logic code for pushing registers onto the stack
	    int push_stack(uint16_t val);

	    // Logic code for pushing register A and the flags onto the stack
	    int push_psw();

	    // Logic code for popping registers off of the stack
	    uint16_t pop_stack();

	    // Logic code for popping register A and the flags off of the stack
	    void pop_psw();

	    // Logic code for exchanging the values of the DE and HL registers
	    int xchg();

	    // Logic code for arithmetic/logical operations
	    // TODO: Implement arithmetic operations
	    void ana(uint8_t val); // AND operation
	    void ora(uint8_t val); // ORA operation
	    void xra(uint8_t val); // XRA operation

	    // Logic code for setting individual flags
	    void setcarry(bool val); // Sets carry flag
	    void sethalf(bool val); // Sets auxillary-carry (aka. half-carry) flag
	    void setzero(bool val); // Sets zero flag
	    void setsign(bool val); // Sets sign flag
	    void setparity(bool val); // Sets parity flag

	    // Logic code for fetching individual flags
	    bool iscarry(); // Fetches carry flag
	    bool ishalf(); // Fetches auxillary-carry (aka. half-carry) flag
	    bool iszero(); // Fetches zero flag
	    bool issign(); // Fetches sign flag
	    bool isparity(); // Fetches parity flag

	    // Bit manipulation functions
	    bool testbit(uint8_t reg, int bit);
	    uint8_t setbit(uint8_t reg, int bit);
	    uint8_t resetbit(uint8_t reg, int bit);
	    uint8_t changebit(uint8_t reg, int bit, bool val);

	    // Function for calculating the parity of a byte
	    bool parity(uint8_t val);
    };
};



#endif // BEE8080_H