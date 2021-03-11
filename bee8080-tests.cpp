#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cstdint>
#include "bee8080.h"
using namespace bee8080;
using namespace std;

vector<uint8_t> memory;

// Intel 8080 machine code to inject at 0x0000
array<uint8_t, 0x23> patch_code = {
    0x3E, 0x01, // mvi a, 1
    0xD3, 0x00, // out 0, a // Value of 0x01 written to port 0 stops test execution
    0x00, // nop
    0x79, // mov a, c
    0xD3, 0x01, // out 1, a // Send command value to control port
    0xDB, 0x01, // in 1 // Receive status byte from control port
    0x47, // mov b, a
    0xE6, 0x01, // ani 1 // Return from function if bit 0 is clear
    0xC8, // rz
    0x78, // mov a, b
    0xE6, 0x02, // ani 2 // Check if bit 1 is set
    0xCC, 0x18, 0x00, // cz c_write // If bit 1 is clear, call c_write function
    0xC4, 0x1C, 0x00, // cnz c_write_str // Otherwise, call c_write_str function
    0xC9, // ret // Return from function
          // c_write:
    0x7B, //       mov a, e
    0xD3, 0x02, // out 2, a // Send character (in E register) to data port
    0xC9, //       ret // Return from function
          // c_write_str:
    0x7A, //       mov a, d
    0xD3, 0x02, // out 2, a // Send MSB of string address to data port
    0x7B, //       mov a, e
    0xD3, 0x02, // out 2, a // Send LSB of string address to data port
    0xC9 //       ret // Return from function
};

void patchisr()
{
    for (int i = 0; i < static_cast<int>(patch_code.size()); i++)
    {
	memory[i] = patch_code[i];
    }
}

bool loadfile(string filename)
{
    ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

    if (!file.is_open())
    {
	cout << "Error" << endl;
	return false;
    }

    memory.resize(0x10000, 0);
    streampos size = file.tellg();
    file.seekg(0, ios::beg);
    file.read((char*)&memory[0x100], size);
    file.close();
    patchisr();
    cout << "Success" << endl;
    return true;
}

class TestInterface : public Bee8080Interface
{
    public:
	TestInterface(bool& test_bool) : is_test_done(test_bool)
	{

	}

	~TestInterface()
	{

	}

	uint8_t readByte(uint16_t addr)
	{
	    return memory[addr];
	}

	void writeByte(uint16_t addr, uint8_t val)
	{
	    memory[addr] = val;
	}

	uint8_t portIn(uint8_t port)
	{
	    uint8_t temp = 0x00;

	    if (port == 0x01)
	    {
		temp = ((!is_single_char << 1) | is_active);
	    }

	    return temp;
	}

	void portOut(uint8_t port, uint8_t val)
	{
	    if (port == 0x00)
	    {
		if (val == 0x01)
		{
		    is_test_done = true;
		}
	    }
	    else if (port == 0x01)
	    {
		switch (val)
		{
		    case 0x02:
		    {
		        is_active = true;
		        is_single_char = true;
		    }
		    break;
		    case 0x09:
		    {
		        is_active = true;
		        is_single_char = false;
		    }
		    break;
		    default: cout << "Invalid command of " << hex << (int)(val) << endl; break;
		}
	    }
	    else if (port == 0x02)
	    {
		if (!is_active)
		{
		    cout << "Error: Please send a valid command to port 1." << endl;
		    return;
		}

		if (is_single_char)
		{
		    cout.put(val);
		}
		else
		{
		    if (!is_msb_sent)
		    {
			str_address = (val << 8);
			is_msb_sent = true;
		    }
		    else
		    {
			str_address |= val;
		
			for (uint16_t addr = str_address; readByte(addr) != '$'; addr++)
			{
			    cout.put(readByte(addr));
			}

			is_msb_sent = false;
		    }
		}
	    }
	}

    private:
	bool is_active = false;
	bool is_single_char = false;
	uint16_t str_address = 0;
	bool is_msb_sent = false;

	bool& is_test_done;
};

void run_test(Bee8080 &core, string filename, uint64_t cycles_expected)
{
    if (!loadfile(filename))
    {
	return;
    }

    bool is_test_done = false;
    TestInterface inter(is_test_done);
    core.setinterface(&inter);

    core.init(0x100);

    cout << "*** TEST: " << filename << endl;

    int cycles = 0;
    uint64_t num_instrs = 0;

    while (!is_test_done)
    {
	num_instrs += 1;
	cycles += core.runinstruction();
    }

    uint64_t diff = (cycles_expected - cycles);
    cout << endl;
    cout << "*** " << num_instrs << " instruction executed on " << dec << (int)(cycles) << " cycles";
    cout << " (expected=" << dec << (int)(cycles_expected) << ", diff=" << dec << (int)(diff) << ")" << endl;
    cout << endl;
    core.shutdown();
    memory.clear();
}

int main(int argc, char *argv[])
{
    Bee8080 core;
    run_test(core, "TEST.COM", 266);
    run_test(core, "TST8080.COM", 4924LU);
    return 0;
}