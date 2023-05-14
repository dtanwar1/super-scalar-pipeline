
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cpu.h"

#define REG_COUNT 16
 
CPU*
CPU_init()
{
    CPU* cpu = malloc(sizeof(*cpu));
    if (!cpu) {
        return NULL;
    }

    /* Create register files */
    cpu->regs= create_registers(REG_COUNT);
    cpu->totalInstructions =0;
    cpu->structuralHazard=0;
    cpu->cycle=0;
    cpu->dataHazard=0;
    cpu->fullReOrderBuffer=0;
    cpu->fetchStall=0;
    cpu->fullReservationStation=0;
    return cpu;
}

/*
 * This function de-allocates CPU cpu.
 */
void
CPU_stop(CPU* cpu)
{
    free(cpu);
}

/*
 * This function prints the content of the registers.
 */
void
print_registers(CPU *cpu){
    
    
    printf("================================\n\n");

    printf("=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n\n");

    printf("--------------------------------\n");
    for (int reg=0; reg<REG_COUNT; reg++) {
        printf("REG[%2d]   |   Value=%d  \n",reg,cpu->regs[reg].value);
        printf("--------------------------------\n");
    }
    printf("================================\n\n");
}

/*
 *  CPU CPU simulation loop
 */
int
CPU_run(CPU* cpu)
{
    
    print_registers(cpu);
    printf("Number of IR stage stalls due to the full reservation station:%d\n",cpu->fullReservationStation);
    printf("Number of IR stage stalls due to the full reorder buffer:%d\n",cpu->fullReOrderBuffer);
    printf("Number of IR stage stalls due to the data hazards:%d\n",cpu->dataHazard);
    printf("Number of fetch stalls:%d\n",cpu->fetchStall);
    printf("\n");
    //printf("Stalled cycles due to data hazard:%d \n",cpu->dataHazard);
    printf("Total execution cycles:%d \n",cpu->cycle);
    //printf("Total instruction simulated:%d\n",cpu->totalInstructions );
    printf("IPC:%f \n",cpu->ipc);
    printf("...End\n");

   
    return 0;
}

Register*
create_registers(int size){
    Register* regs = malloc(sizeof(*regs) * size);
    if (!regs) {
        return NULL;
    }
    for (int i=0; i<size; i++){
        regs[i].value = 0;
        regs[i].is_writing = false;
        regs[i].tag = 0;
    }
    return regs;
}
void print_display(CPU *cpu, int cycle){
    

    printf("================================\n");
    printf("Clock Cycle #: %d\n", cycle);
    printf("--------------------------------\n");

   for (int reg=0; reg<REG_COUNT; reg++) {
       
        printf("REG[%2d]   |   Value=%d  \n",reg,cpu->regs[reg].value);
        printf("--------------------------------\n");
    }
    printf("\n");

}


