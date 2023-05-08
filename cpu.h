#ifndef _CPU_H_
#define _CPU_H_
#include <stdbool.h>
#include <assert.h>

#define BTB_COUNT 16
#define ROB_COUNT 8
#define STAGE_COUNT 26
#define RESERVE_STATION_COUNT 4

#define ADDER_UNIT_COUNT 1
#define MUL_UNIT_COUNT 2
#define DIV_UNIT_COUNT 3
#define MM_UNIT_COUNT 4

#define add 0
#define sub 1
#define div 2
#define mul 3
#define ret 4
#define set 5
#define ld 6
#define st 7
#define bez 8
#define bgez 9
#define blez 10
#define bgtz 11
#define bltz 12

#define NO_HAZARD 0
#define STRUCTURAL_HAZARD 1
#define DATA_HAZARD 2
#define FULL_RESERVATION_STATION 2

#define IF 0
#define ID 1
#define IA 2
#define RR 3
#define RS3 4
#define RS2 5
#define RS1 6
#define RS0 7
#define IS 8
#define ADD 9
#define MUL1 10
#define MUL2 11
#define DIV1 12
#define DIV2 13
#define DIV3 14
#define MM1 15
#define MM2 16
#define MM3 17
#define MM4 18
#define WB1 19
#define WB2 20
#define WB3 21
#define WB4 22
#define RE1 23
#define RE2 24
#define COMP 25





typedef struct ROB
{
   char bufferName[5];
   int dest;
   int result;
   bool e;
   bool completed;
   int index;
    
} ROB;

typedef struct Dependecy{
    int depRegister;
    int index;
    int robIndex;
    int tempResult;
    char name[5];
}Dependency;



typedef struct Stage
{
    int type;
    int index;
    char *orignalInstruction;
    char *opcode;
    int destinationRegister;
    int sourceRegister1;
    int sourceRegister2;    
    int immediateValue1;
    int immediateValue2;
    Dependency dependentInstructionInfo[11];
    int counter;
    int tempResult;
    int stallType;
    int tag;
    int pcIndex;
    char pcBinary[33];

    char logInstruction[250];
    char pCounter[5];
    int path;
    char destinationRegisterName[5];
    char sourceRegister2Name[5];
    char sourceRegister1Name[5];
    int robSr2;
    int robSr1;
    int robDest;
} Stage;


typedef struct myLogger
{
    int type;
    int index;
    char logInfo[250];
}  myLogger;


// typedef struct OutOfOrderStage
// {
//     Stage *adderUnit;
//     Stage *multiplierUnit;
//     Stage *dividerUnit;
//     Stage *memoryUnit;
// }  OutOfOrderStage;

typedef struct BTB
{
   int tag;
   int target;
   int pattern;
    
} BTB;





typedef struct Register
{
    int value;          // contains register value
    bool is_writing;    // indicate that the register is current being written
	                    // True: register is not ready
						// False: register is ready
    int tag;                    
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
