#ifndef _CPU_H_
#define _CPU_H_
#include <stdbool.h>
#include <assert.h>



typedef struct Register
{
    int value;          // contains register value
    bool is_writing;    // indicate that the register is current being written
	                    // True: register is not ready
						// False: register is ready
} Register;

/* Model of CPU */
typedef struct CPU
{
	/* Integer register file */
	Register *regs;
	int cycle;
	int totalInstructions;
	float ipc;
	int structuralHazard;
	int dataHazard;

	
} CPU;

CPU*
CPU_init();

Register*
create_registers(int size);

int
CPU_run(CPU* cpu);

void
CPU_stop(CPU* cpu);

void 
print_display(CPU *cpu);

#endif
