#ifndef _CPU_H_
#define _CPU_H_
#include <stdbool.h>
#include <assert.h>

#define BTB_COUNT 16
#define ROB_COUNT 8
#define STAGE_COUNT 30
#define RESERVE_STATION_COUNT 4


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
#define FULL_RESERVATION_STATION 3
#define FULL_REORDER_BUFFER 4

#define IF 0
#define ID 1
#define IA 2
#define RR 3
#define RS3 4
#define RS2 5
#define RS1 6
#define RS0 7
#define IS3 8
#define IS2 9
#define IS1 10
#define IS0 11
#define ADD 12
#define MUL1 13
#define MUL2 14
#define DIV1 15
#define DIV2 16
#define DIV3 17
#define MM1 18
#define MM2 19
#define MM3 20
#define MM4 21
#define WB1 22
#define WB2 23
#define WB3 24
#define WB4 25
#define RE1 26
#define RE2 27
#define COMP1 28
#define COMP2 29







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

typedef struct ROB
{
   char bufferName[5];
   int dest;
   int result;
   bool e;
   bool completed;
   int index;
   Stage *reInstr;
    
} ROB;


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
    int fullReservationStation;
    int fullReOrderBuffer;
    int fetchStall;

	
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
