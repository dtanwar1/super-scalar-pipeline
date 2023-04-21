

//
//  main.c
//  Pipeline
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include <limits.h>

bool instructionSquashed = false;
#define BTB_COUNT 16

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

#define IF 0
#define ID 1
#define IA 2
#define RR 3
#define ADD 4
#define MUL 5
#define DIV 6
#define BR 7
#define MM1 8
#define MM2 9
#define WB 10
#define COMP 11

int programCounter = 0;
int totalInstructions=0;
char *memArray[16384];
bool calculateStructuralHazard = false;
bool calculateDataHazard = true;


typedef struct Dependecy{
    int depRegister;
    int index;
    int tempResult;
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
} Stage;



typedef struct BTB
{
   int tag;
   int target;
   int pattern;
    
} BTB;

void logStageAndInstruction(int stage, char* instruction){
    char *strStage;
    if(stage == IF){
        strStage ="IF  ";
    }if(stage == ID){
        strStage ="ID  ";
    }if(stage == IA){
        strStage ="IA  ";
    }if(stage == RR){
        strStage ="RR  ";
    }if(stage == ADD){
        strStage ="ADD ";
    }if(stage == MUL){
        strStage ="MUL ";
    }if(stage == DIV){
        strStage ="DIV ";
    }if(stage == BR){
        strStage ="BR  ";
    }if(stage == MM1){
        strStage ="Mem1";
    }if(stage == MM2){
        strStage ="Mem2";
    }if(stage == WB){
        strStage ="WB  ";
    }
    
    //printf("%s           : %s\n", strStage,instruction);
}
void decimalToBinary(int decimal, int bits, char *binary) {
    int i;

    for (i = 0; i < bits; i++) {
        binary[i] = (decimal & (1 << (bits - i - 1))) ? '1' : '0';
    }

    binary[bits] = '\0'; 
}
int checkIfRegisterPresent(Dependency dependentInstructionInfo[11],int regr){
    int index = -1;
    for(int i=0;i<11;i++){
        if(dependentInstructionInfo[i].depRegister == regr){
            index  = i;
            break;
        }
    }
    return index;
}
void resetOldInstruction(int oldStage,Stage *stage){
    Stage *oldInstruction = &stage[oldStage];

    if(oldInstruction->index >-1){
        oldInstruction->index = -1 ;
        oldInstruction->counter = -1;
        oldInstruction->destinationRegister = -1;
        oldInstruction->immediateValue1 = -1;
        oldInstruction->immediateValue2 = -1;
        oldInstruction->opcode ="";
        oldInstruction->orignalInstruction=""; 
        for (int i=0; i<33;i++){
            oldInstruction->pcBinary[i] = 0;
        }
        oldInstruction->pcIndex = -1;
        oldInstruction->sourceRegister1 = -1;
        oldInstruction->sourceRegister2=-1;
        oldInstruction->stallType = -1;
        oldInstruction->tag = -1;
        oldInstruction->tempResult = INT_MIN;
        oldInstruction->type = IF;
        for (int i=0; i<11;i++){
            oldInstruction->dependentInstructionInfo[i].index =-2;
            oldInstruction->dependentInstructionInfo[i].depRegister =-1;
            oldInstruction->tempResult =INT_MIN;
        }
    }

}
void readMemoryMapFromFile(char* filename){
    FILE *fptr;
    char line[16384*4];
    
    if ((fptr = fopen(filename,"r")) != NULL){ 
        
        int loopIndex = 0;

        while (fgets(line, sizeof(line), fptr)) {
            char *ptr = strtok(line, " ");
            
            while (ptr != NULL){
                memArray[loopIndex] = malloc(sizeof(ptr));    
                strncpy(memArray[loopIndex++], ptr, sizeof(ptr));
                ptr = strtok (NULL, " ");
            }
        }
        
       
    }
    else{

        printf("Error! opening file");
        exit(1);

    }
    
    fclose(fptr);
}
void readInstructionFromFile(char* filename,char *abc[300]){
    FILE *fptr;
    

    if ((fptr = fopen(filename,"r")) != NULL){ 
        
        char line[500];
        
        int loopIndex = 0;

        while (fgets(line, sizeof(line), fptr)) {

            char *ptr = strtok(line, "\n");
            //printf("%s\n",ptr);
            abc[loopIndex] = malloc(sizeof(line));
            strncpy(abc[loopIndex], ptr, sizeof(line));
            loopIndex++;

        }
        totalInstructions = loopIndex;
       
    }
    else{

        printf("Error! opening file");
        exit(1);

    }
    fclose(fptr);
}

void writeToFile(char *filename ){
    FILE *fp = fopen(filename, "w"); 
    if (fp == NULL) { 
        printf("Error opening file.");
    }
    int i;
    for (i = 0; i < 16384; i++) {
        if (memArray[i] != NULL) { 
            fprintf(fp, "%s ", memArray[i]);
        }
    }
    fclose(fp); // close file
}

int createOpcodeIndex(char* opcode){
    int resopcode = -1;
    char first_three[4];
    strncpy(first_three, opcode, 3);
    first_three[3] = '\0';
    if (strcmp(opcode, "add") == 0)
    {
        resopcode = add;
    }
    if(strcmp(opcode, "sub") == 0){
        resopcode = sub;
    }
    if(strcmp(opcode, "div") == 0){
        resopcode =  div;
    }
     if(strcmp(opcode, "mul") == 0){
        resopcode =  mul;
    }
    if(strcmp(opcode, "ret") == 0){
        resopcode =  ret;
    }
    if(strcmp(opcode, "set") == 0){
        resopcode =  set;
    }
    if(strcmp(opcode, "ld") == 0){
        resopcode =  ld;
    }
    if(strcmp(opcode, "st") == 0){
        resopcode =  st;
    }
    if(strcmp(opcode, "bez") == 0){
        resopcode =  bez;
    }
    if(strcmp(opcode, "bgez") == 0){
        resopcode =  bgez;
    }
    if(strcmp(opcode, "blez") == 0){
        resopcode =  blez;
    }
    if(strcmp(opcode, "bgtz") == 0){
        resopcode =  bgtz;
    }
    if(strcmp(opcode, "bltz") == 0){
        resopcode =  bltz;
    }
    if(strcmp(first_three, "ret") == 0){
        resopcode =  ret;
    }
    return resopcode;
}
Stage*
stage_init()
{   
    Stage* stage = malloc(sizeof(*stage) * (COMP+1));
    if (!stage) {
        return NULL;
    }
    for (int i=IF; i<=COMP; i++){
        stage[i].type = i;
        stage[i].index = -1;
    }
    return stage;
}
bool squashInstruction( Stage  *instr){
    bool instrSquashed = true;
    for(int i = IF; i< BR; i++ ){

        resetOldInstruction(i,instr);
        //   Instruction *filterInstructions = &instr[i]; 

        //     if(filterInstructions->stage >= IF){
        //         filterInstructions->stage = IF;
        //     }
        //     else{
        //         break;
        //     }
    }
    return instrSquashed;
}
bool checkIfBranchInstruction(int resopcode){
    bool isBranch = false;

    if(resopcode == bez
    ||resopcode == bgez
    ||resopcode == bgtz
    ||resopcode == blez
    ||resopcode == bltz){
        isBranch = true;
    }
    return isBranch;

}
bool checkIfInstrShouldBeForwarded(Stage  *instr, int dependentStages[]){
    bool shouldInstrBeForwarded = false; 
    Stage *rrInstruction = &instr[RR];

    for(int i=0;i<8;i++){
        int stage = dependentStages[i];
        

        if(stage == -1){
            break;
        }

        if(stage <=BR){
            Stage *depInstruction = &instr[stage];
            if(createOpcodeIndex(depInstruction->opcode)!=st && createOpcodeIndex(depInstruction->opcode)!=ld){
                int index = checkIfRegisterPresent(rrInstruction->dependentInstructionInfo,depInstruction->destinationRegister);
                if(depInstruction->destinationRegister == rrInstruction->sourceRegister1){
                    
                    shouldInstrBeForwarded = true;
                }
                else if(depInstruction->destinationRegister == rrInstruction->sourceRegister2){
                    
                    shouldInstrBeForwarded = true;
                }
                 else if (checkIfBranchInstruction(createOpcodeIndex(rrInstruction->opcode)) 
                        && depInstruction->destinationRegister == rrInstruction->destinationRegister )
                {
                    shouldInstrBeForwarded = true;
                }
                else if (createOpcodeIndex(rrInstruction->opcode) == st 
                &&  depInstruction->destinationRegister == rrInstruction->destinationRegister) 
                {
                    shouldInstrBeForwarded = true;
                }

                if(shouldInstrBeForwarded && index >-1){
                    rrInstruction->dependentInstructionInfo[index].tempResult = depInstruction->tempResult;
                }
            }

            
        }
        else{
            
            Stage *depInstruction = &instr[stage];
            shouldInstrBeForwarded =false;
            int index = checkIfRegisterPresent(rrInstruction->dependentInstructionInfo,depInstruction->destinationRegister);
            if(stage == COMP && !shouldInstrBeForwarded && index >-1){
                rrInstruction->dependentInstructionInfo[index].tempResult = depInstruction->tempResult;
                
            }
            break;
            
        }
    } 
    return shouldInstrBeForwarded;    
}



void pushDataToNewStage(int presentStage,int newStage,Stage *stage){
    Stage *presentInstruction = &stage[presentStage];
    Stage *newInstruction = &stage[newStage];
    if(presentInstruction->index >-1){
        newInstruction->index = presentInstruction->index ;
        newInstruction->counter = presentInstruction->counter;
        newInstruction->destinationRegister = presentInstruction->destinationRegister;
        newInstruction->immediateValue1 = presentInstruction->immediateValue1;
        newInstruction->immediateValue2 = presentInstruction->immediateValue2;       
        newInstruction->opcode = malloc(sizeof(presentInstruction->opcode));
        strcpy(newInstruction->opcode, presentInstruction->opcode);
        newInstruction->orignalInstruction = malloc(sizeof(presentInstruction->orignalInstruction));
        strcpy(newInstruction->orignalInstruction, presentInstruction->orignalInstruction);
        for (int i=0; i<33;i++){
            newInstruction->pcBinary[i] = presentInstruction->pcBinary[i];
        }
        newInstruction->pcIndex = presentInstruction->pcIndex;
        newInstruction->sourceRegister1 = presentInstruction->sourceRegister1;
        newInstruction->sourceRegister2=presentInstruction->sourceRegister2;
        newInstruction->stallType = presentInstruction->stallType;
        newInstruction->tag = presentInstruction->tag;
        newInstruction->tempResult = presentInstruction->tempResult;
        newInstruction->type = newStage;
        for (int i=0; i<11;i++){
            newInstruction->dependentInstructionInfo[i].depRegister = presentInstruction->dependentInstructionInfo[i].depRegister;
            newInstruction->dependentInstructionInfo[i].index = presentInstruction->dependentInstructionInfo[i].index;
        }

        resetOldInstruction(presentStage,stage);
    }
}

int getStalledInstructionIndex(Stage *pipelineInstructions){
    int stalledInstrIndex = -1;
    for(int i=IF; i<COMP;i++){
        Stage *filterInstructions = &pipelineInstructions[i]; 

        if(filterInstructions->stallType >NO_HAZARD){
            stalledInstrIndex = filterInstructions->index;
            break;
        }
    }
    return stalledInstrIndex;
}



void resetStalledInstructions(int indexOfStalledInstr,Stage *pipelineInstructions){
    Stage *filterInstructions = NULL;
    for(int i=IF;i<COMP;i++){

        filterInstructions = &pipelineInstructions[i];

        if(filterInstructions->index == indexOfStalledInstr){

            if(calculateStructuralHazard){
                filterInstructions->stallType = NO_HAZARD;
            } 

            if(calculateDataHazard){
                if(filterInstructions->dependentInstructionInfo!=NULL){
                    for(int i=0; i<11; i++){
                        int dependentIndex = filterInstructions->dependentInstructionInfo[i].index;

                        if(dependentIndex < -1){
                            break;
                        }
                        if(dependentIndex>-1){

                            if(pipelineInstructions[COMP].index == dependentIndex){
                                filterInstructions->stallType = NO_HAZARD;
                                filterInstructions->dependentInstructionInfo[i].index = -1;
                            }
                        }
                    }
                }

            }

            break;
        }
    
    }     
      
    
}
bool writeBack(Stage  *instr,CPU *cpu){
     bool toStop = false;
     Stage *wbInstruction = &instr[WB]; 

     if(wbInstruction->index > -1){

            logStageAndInstruction(WB,wbInstruction->orignalInstruction);
            switch (createOpcodeIndex(wbInstruction->opcode))
            {
                            case add:    
                            case mul:
                            case sub:
                            case div:
                            case set:
                                    cpu->regs[wbInstruction->destinationRegister].is_writing = true;
                                    cpu->regs[wbInstruction->destinationRegister].value = wbInstruction->tempResult;
                                break;
                                    
                            case ret:
                                toStop = true;

                            case ld:
                                cpu->regs[wbInstruction->destinationRegister].is_writing = true;
                                cpu->regs[wbInstruction->destinationRegister].value = wbInstruction->tempResult;

                                if(calculateStructuralHazard){
                                    //int idIndex =getIndexBasedOnStage(ID,instr,stage);
                                    Stage *idInstruction = &instr[ID]; 
                                    if(idInstruction->index >-1){ 
                                        idInstruction->stallType = STRUCTURAL_HAZARD;
                                    }
                                }

                             break;    
                            default:
                                break;
                                

            } 

            pushDataToNewStage(WB,COMP,instr);                     
        
     }
    return toStop;

}
void fetch(char *instrFromFile[300],Stage  *pipelineInstr,BTB *btb){

    int index = 0;

    if(programCounter>0){
        index = programCounter/4;
    }
    
    if(instructionSquashed){
        index = -1;
        instructionSquashed = false;
    }
    if(index >-1){
        bool stall = false;

        

        if(index <totalInstructions){
 
            Stage *fetchInstruction = &pipelineInstr[IF]; 
            Stage *rrInstruction = &pipelineInstr[RR]; 
            
            if(rrInstruction->index >-1 ){

                if(rrInstruction->stallType == DATA_HAZARD){
                    stall = true;
                    fetchInstruction->orignalInstruction = instrFromFile[index];
                    logStageAndInstruction(IF,fetchInstruction->orignalInstruction);
                }
            }

            if(fetchInstruction->stallType == STRUCTURAL_HAZARD){  //ToVisit.
                stall = true;
            }
            
            if(!stall){
                fetchInstruction->index = index;
                fetchInstruction->orignalInstruction = malloc(sizeof(&instrFromFile[index]));
                strcpy(fetchInstruction->orignalInstruction, instrFromFile[index]);
                logStageAndInstruction(IF,fetchInstruction->orignalInstruction);
                fetchInstruction->counter = programCounter;
                decimalToBinary(fetchInstruction->counter,32,fetchInstruction->pcBinary);
                fetchInstruction->pcIndex = (fetchInstruction->counter  >> 2) & 0b1111;
                fetchInstruction->tag = fetchInstruction->counter  >> 6;

                fetchInstruction->sourceRegister1 = -1;
                fetchInstruction->sourceRegister2 =-1;
                fetchInstruction->destinationRegister =-1;            
                fetchInstruction->immediateValue1 =-1;
                fetchInstruction->immediateValue2 =-1;            
                fetchInstruction->stallType =NO_HAZARD;
                fetchInstruction->tempResult = INT_MIN;
                fetchInstruction->opcode = "";


                char substr[5];

                sscanf(fetchInstruction->orignalInstruction, "%*s %s", substr);

                bool isBranch =checkIfBranchInstruction(createOpcodeIndex(substr));
                

                for(int i=0;i<11;i++){
                    fetchInstruction->dependentInstructionInfo[i].index = -2;
                    fetchInstruction->dependentInstructionInfo[i].depRegister = -1;
                    fetchInstruction->dependentInstructionInfo[i].tempResult =INT_MIN;
                }


                if(isBranch && btb[fetchInstruction->pcIndex].pattern>=4){
                    programCounter = btb[fetchInstruction->pcIndex].target;
                }
                else{
                    programCounter = programCounter+4;
                }
                pushDataToNewStage(IF,ID,pipelineInstr);

                
            }
        }
    }
    
}
void decode(Stage  *instr){
   
    Stage *decodeInstruction = &instr[ID]; 
    bool stall = false;

    if(decodeInstruction->index > -1){
        
        Stage *rrInstruction = &instr[RR]; 
        
        
        if(rrInstruction->index>-1 ){
            
            if(rrInstruction->stallType == DATA_HAZARD){
                logStageAndInstruction(ID,decodeInstruction->orignalInstruction);   
                stall = true;
            }
        }

        if(decodeInstruction->stallType == STRUCTURAL_HAZARD){
            stall = true;
        }

        if(!stall){

                    logStageAndInstruction(ID,decodeInstruction->orignalInstruction); 
                    char* backupOrgInstr = malloc(sizeof(decodeInstruction->orignalInstruction));
                    strcpy(backupOrgInstr, decodeInstruction->orignalInstruction);  
                    char *ptr = strtok(backupOrgInstr, " ");
                    int loopIndex = 0;
                    while(ptr!=NULL){

                        if(loopIndex == 1){
                            decodeInstruction->opcode = malloc(sizeof(ptr));
                            strcpy(decodeInstruction->opcode, ptr);

                        }
                        if(loopIndex >= 2){
                            char* value = malloc(sizeof(ptr) -1);
                            strncpy(value, ptr + 1, sizeof(ptr) -1);
                            //printf("'%s'\n", value);

                            switch (createOpcodeIndex(decodeInstruction->opcode))
                            {
                                case add:
                                case sub:
                                case mul:
                                case div:
                                case set:
                                case ld:
                                case st:
                                case bez:
                                case bgez:
                                case bgtz:
                                case blez:
                                case bltz:
                                

                                    if(strchr(ptr,'R')){
                                    
                                        if(decodeInstruction->destinationRegister == -1){
                                            decodeInstruction->destinationRegister = atoi(value);
                                        }else{

                                            if(decodeInstruction->sourceRegister1 == -1){
                                                decodeInstruction->sourceRegister1 = atoi(value); 
                                            }
                                            else{
                                                decodeInstruction->sourceRegister2 = atoi(value); 
                                            }  
                                        }

                                        
                                    }
                                    else{
                                        if(decodeInstruction->immediateValue1 == -1 && decodeInstruction->sourceRegister1 == -1){
                                            decodeInstruction->immediateValue1 = atoi(value);
                                        }else{
                                            decodeInstruction->immediateValue2 = atoi(value);   
                                        }
                                    
                                    }
                                
                                break;
                            
                                default:
                                    break;
                            }
                        }
                        //printf("'%s'\n", ptr);
                        ptr = strtok(NULL, " ");

                        loopIndex++;
                    }
                    
                    pushDataToNewStage(ID,IA,instr);
                    
        }
   }
    

}
void instructionAnalyse(Stage  *instr){
    
    Stage *analyseInstruction = &instr[IA]; 
    bool stall = false;
    if(analyseInstruction->index > -1){
        
        Stage *rrInstruction = &instr[RR]; 
        if(rrInstruction->index >-1 ){
            if(rrInstruction->stallType == DATA_HAZARD){
                stall = true;
            }
        }

        logStageAndInstruction(IA,analyseInstruction->orignalInstruction);

        if(!stall){
            
            
            
                        
            if(calculateDataHazard){
                

                if(!checkIfBranchInstruction(createOpcodeIndex(analyseInstruction->opcode))){

                    int arrayIndex = 0;

                    for(int i=WB; i>RR;i--){    

                        
                        Stage *filterInstructions = &instr[i]; 

                        if(filterInstructions->index >-1){
                            

                            if(createOpcodeIndex(filterInstructions->opcode) !=st
                                && createOpcodeIndex(filterInstructions->opcode) !=ret
                                && !checkIfBranchInstruction(createOpcodeIndex(filterInstructions->opcode))){

                                if(filterInstructions->destinationRegister == analyseInstruction->sourceRegister1
                                    ||filterInstructions->destinationRegister == analyseInstruction->sourceRegister2){
                                        int regIndx = -1;
                                        regIndx = checkIfRegisterPresent(analyseInstruction->dependentInstructionInfo,filterInstructions->destinationRegister);
                                        if(regIndx>-1){
                                            analyseInstruction->dependentInstructionInfo[regIndx].index = filterInstructions->index;
                                        }
                                        else{
                                            analyseInstruction->dependentInstructionInfo[arrayIndex].index = filterInstructions->index;
                                            analyseInstruction->dependentInstructionInfo[arrayIndex].depRegister = filterInstructions->destinationRegister;
                                            arrayIndex++;
                                        }
                                        
                                        
                                }

                                else if(createOpcodeIndex(analyseInstruction->opcode) == st && filterInstructions->destinationRegister == analyseInstruction->destinationRegister){
                                     int regIndx = -1;
                                        regIndx = checkIfRegisterPresent(analyseInstruction->dependentInstructionInfo,filterInstructions->destinationRegister);
                                        if(regIndx>-1){
                                            analyseInstruction->dependentInstructionInfo[regIndx].index = filterInstructions->index;
                                        }
                                        else{
                                            analyseInstruction->dependentInstructionInfo[arrayIndex].index = filterInstructions->index;
                                            analyseInstruction->dependentInstructionInfo[arrayIndex].depRegister = filterInstructions->destinationRegister;
                                            arrayIndex++;
                                        }
                                }
                            }
                        }
                        
                    } 
                }
                else{

                    int arrayIndex = 0;

                    for(int i=WB; i>RR;i--){   

                        Stage *filterInstructions = &instr[i]; 

                        if(filterInstructions->index >-1){
                            

                            if(createOpcodeIndex(filterInstructions->opcode) !=st
                                && createOpcodeIndex(filterInstructions->opcode) !=ret
                                && !checkIfBranchInstruction(createOpcodeIndex(filterInstructions->opcode))){

                                if(filterInstructions->destinationRegister == analyseInstruction->destinationRegister){
                                        int regIndx = checkIfRegisterPresent(analyseInstruction->dependentInstructionInfo,filterInstructions->destinationRegister);
                                        if(regIndx>-1){
                                            analyseInstruction->dependentInstructionInfo[regIndx].index = filterInstructions->index;
                                        }
                                        else{
                                            analyseInstruction->dependentInstructionInfo[arrayIndex].index = filterInstructions->index;
                                            analyseInstruction->dependentInstructionInfo[arrayIndex].depRegister = filterInstructions->destinationRegister;
                                            arrayIndex++;
                                        }
                                        
                                        
                                }
                            }

                        }      
                    } 
                }

                
                
            }
            pushDataToNewStage(IA,RR,instr);
      
        }
    }
}
void registerRead(Stage  *instr,CPU *cpu){
    Stage *rrInstruction = &instr[RR]; 
    int prevIndex = -1;
    int prevStage = -1;
    bool shouldBeForwarded = false;
    int addIndex = 0;

    if(rrInstruction->index > -1){
        int dependentStages[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

        if(rrInstruction->dependentInstructionInfo!=NULL){
            bool breakLoop = false;
            
            for(int i=0; i< 11;i++){

                int dependentIndex = rrInstruction->dependentInstructionInfo[i].index;
                
                if(dependentIndex <-1){
                    break;
                }
                if(dependentIndex > -1){
                    prevIndex = dependentIndex;    

                    for(int j= COMP;j>ADD;j--){

                        Stage *prevInstruction = &instr[j];
                        if(prevInstruction->index>-1){

                            if(prevInstruction->index == dependentIndex){ 
                                dependentStages[addIndex] = j;
                                addIndex++;
                            }
                        }
                    }                        
                }
                
            }
        }
        
            shouldBeForwarded = checkIfInstrShouldBeForwarded(instr,dependentStages);
            
        

            logStageAndInstruction(RR,rrInstruction->orignalInstruction);

            
            
            if(shouldBeForwarded){
                rrInstruction->stallType = NO_HAZARD;
            }
            else{
                rrInstruction->stallType = addIndex > 0? DATA_HAZARD : NO_HAZARD;
            }

            if(rrInstruction->sourceRegister1 !=-1){

                if(shouldBeForwarded){
                    int matchIndex = checkIfRegisterPresent(rrInstruction->dependentInstructionInfo,rrInstruction->sourceRegister1);

                    if(matchIndex>-1){
                        rrInstruction->immediateValue1 = rrInstruction->dependentInstructionInfo[matchIndex].tempResult;
                    }
                    else{
                        rrInstruction->immediateValue1 = cpu->regs[rrInstruction->sourceRegister1].value;
                    }
                }
                else{
                    rrInstruction->immediateValue1 = cpu->regs[rrInstruction->sourceRegister1].value;   
                }
            }
            if(rrInstruction->sourceRegister2 !=-1){
                if(shouldBeForwarded){
                    int matchIndex = checkIfRegisterPresent(rrInstruction->dependentInstructionInfo,rrInstruction->sourceRegister2);

                    if(matchIndex>-1){
                        rrInstruction->immediateValue2 = rrInstruction->dependentInstructionInfo[matchIndex].tempResult;
                    }
                    else{
                        rrInstruction->immediateValue2 = cpu->regs[rrInstruction->sourceRegister2].value;
                    }
                }
                else{
                    rrInstruction->immediateValue2 = cpu->regs[rrInstruction->sourceRegister2].value;   
                }

            }

            // if(createOpcodeIndex(rrInstruction->opcode) == st){
            //         rrInstruction->immediateValue2 = cpu->regs[rrInstruction->destinationRegister].value;
            // }
            if(checkIfBranchInstruction(createOpcodeIndex(rrInstruction->opcode)) || createOpcodeIndex(rrInstruction->opcode) == st){
                if(shouldBeForwarded){
                    int matchIndex = checkIfRegisterPresent(rrInstruction->dependentInstructionInfo,rrInstruction->destinationRegister);

                    if(matchIndex>-1){
                        rrInstruction->immediateValue2 = rrInstruction->dependentInstructionInfo[matchIndex].tempResult;
                    }
                    else{
                        rrInstruction->immediateValue2 =  cpu->regs[rrInstruction->destinationRegister].value;
                    }
                   
                }
                else{
                    rrInstruction->immediateValue2 = cpu->regs[rrInstruction->destinationRegister].value;
                }

            }


            if(rrInstruction->immediateValue1 == INT_MIN || rrInstruction->immediateValue2 == INT_MIN){
                rrInstruction->stallType =  DATA_HAZARD;
            }


          
            if(rrInstruction->stallType !=  DATA_HAZARD){
                pushDataToNewStage(RR,ADD,instr);
            }
            
        
    }

}


void adder(Stage  *instr){
    Stage *addInstruction = &instr[ADD]; 

    if(addInstruction->index > -1){

            logStageAndInstruction(ADD,addInstruction->orignalInstruction);
            
            switch (createOpcodeIndex(addInstruction->opcode))
            {
                            case add:
                                    addInstruction->tempResult = addInstruction->immediateValue1 + addInstruction->immediateValue2;
                                    break;
                                
                            case sub:
                                    addInstruction->tempResult = addInstruction->immediateValue1 - addInstruction->immediateValue2;
                                 break;   
                            
                            case set:
                                    addInstruction->tempResult = addInstruction->immediateValue1;
                                break;

                            default:
                                break;
                                

            }          
            pushDataToNewStage(ADD,MUL,instr);
                            
        
    }
}

void multiplier(Stage  *instr){
    Stage *mulInstruction = &instr[MUL]; 

    if(mulInstruction->index > -1){

            logStageAndInstruction(MUL,mulInstruction->orignalInstruction);
            switch (createOpcodeIndex(mulInstruction->opcode))
            {
                            case mul:
                                    mulInstruction->tempResult = mulInstruction->immediateValue1 * mulInstruction->immediateValue2;
                                break;
                        
                            default:
                                break;
                                

            }      
            pushDataToNewStage(MUL,DIV,instr);        
                            
        
    }
}

void divider(Stage  *instr){

    Stage *divInstruction = &instr[DIV]; 

    if(divInstruction->index > -1){


            logStageAndInstruction(DIV,divInstruction->orignalInstruction);
            switch (createOpcodeIndex(divInstruction->opcode))
            {
                            case div:
                                    divInstruction->tempResult = divInstruction->immediateValue1 / divInstruction->immediateValue2;
                                break;
                        
                            default:
                                break;
                                

            }    

            pushDataToNewStage(DIV,BR,instr);              
                            
        
    }
}
void branch(Stage  *instr,BTB *btb){

    Stage *branchInstruction = &instr[BR]; 
    

    if(branchInstruction->index > -1){


        
            logStageAndInstruction(BR,branchInstruction->orignalInstruction);

            if(checkIfBranchInstruction(createOpcodeIndex(branchInstruction->opcode))){

                bool toPredict = false;

                switch (createOpcodeIndex(branchInstruction->opcode))
                {
                                case bez:
                                    if(branchInstruction->immediateValue2 == 0){
                                        toPredict = true;
                                    }
                                    break;
                                case bgez:
                                    if(branchInstruction->immediateValue2 >= 0){
                                        toPredict = true;
                                    }
                                    break;

                                case blez:
                                    if(branchInstruction->immediateValue2 <= 0){
                                        toPredict = true;
                                    }
                                    break;
                                case bgtz:
                                    if(branchInstruction->immediateValue2 > 0){
                                        toPredict = true;
                                    }
                                    break;
                                case bltz:
                                    if(branchInstruction->immediateValue2 < 0){
                                        toPredict = true;
                                    }
                                    break;

                                default:
                                    break;
                }   
                
            
                int pc = branchInstruction->pcIndex;

                if(toPredict){
                    if(pc >-1){
                        if(btb[pc].pattern >=4){
                            btb[pc].pattern = btb[pc].pattern >=7? btb[pc].pattern: btb[pc].pattern+1;
                        }
                        else{
                            btb[pc].pattern = btb[pc].pattern+1; 
                            btb[pc].tag =branchInstruction->tag;
                            btb[pc].target = branchInstruction->immediateValue1;
                            instructionSquashed = squashInstruction(instr);
                            programCounter =branchInstruction->immediateValue1;
                            
                        }
                    }
                }
                else{
                    if(pc>-1){
                        int orignalBtb = btb[pc].pattern;
                        btb[pc].pattern = btb[pc].pattern ==0? btb[pc].pattern :btb[pc].pattern-1;
                        if(orignalBtb>=4){
                            instructionSquashed = squashInstruction(instr);
                            programCounter =branchInstruction->counter+4;
                        }
                        else{
                            btb[pc].tag =branchInstruction->tag;
                            btb[pc].target = branchInstruction->immediateValue1;
                        }

                    }
                }
            }
           
            pushDataToNewStage(BR,MM1,instr);     
        
    }
}

void mem1(Stage  *instr){

    Stage *mm1Instruction = &instr[MM1]; 

    if(mm1Instruction->index > -1){


        
            logStageAndInstruction(MM1,mm1Instruction->orignalInstruction);

            switch (createOpcodeIndex(mm1Instruction->opcode))
            {
                            case ld:
                            case st:
                                     mm1Instruction->immediateValue1 = ((mm1Instruction->immediateValue1)/4);
                                break;

                            default:
                                break;                                
            }   

            pushDataToNewStage(MM1,MM2,instr);             
                            
        
    }
}
void mem2(Stage  *instr){

    Stage *mm2Instruction = &instr[MM2]; 

    if(mm2Instruction->index > -1){
 
            logStageAndInstruction(MM2,mm2Instruction->orignalInstruction);
            char snum[20];
            switch (createOpcodeIndex(mm2Instruction->opcode))
            {
                            case ld:
                                mm2Instruction->tempResult = atoi(memArray[mm2Instruction->immediateValue1]);
                                 if(calculateStructuralHazard){
                                        //tocheck
                                        // int ifIndex = getIndexBasedOnStage(IF,instr,stage);
                                        // if(ifIndex >-1){
                                        //     Instruction *stallInstruction = &instr[getIndexBasedOnStage(IF,instr,stage)];
                                        //     stallInstruction->stallType = STRUCTURAL_HAZARD;
                                        // }
                                        
                                    }
                                break;

                            case st:
                                sprintf(snum, "%d", mm2Instruction->immediateValue2);
                                strncpy(memArray[mm2Instruction->immediateValue1], snum, sizeof(snum));
                                break;
                                                        
                            default:
                                break;                                
            }   

            pushDataToNewStage(MM2,WB,instr);              
                            
        
    }
}

BTB*
BTB_init()
{   
    BTB* btb = malloc(sizeof(*btb) * BTB_COUNT);
    if (!btb) {
        return NULL;
    }
    for (int i=0; i<BTB_COUNT; i++){
        btb[i].tag = -1;
        btb[i].target = -1;
        btb[i].pattern = 3;
        
    }
    return btb;
}

void printBTBPrediction(BTB *btb){
    printf("\n");
    printf("============ BTB =================================\n\n");

    for (int b=0; b<BTB_COUNT; b++) {
        printf("|	 BTB[%2d]	|	Tag=%2d   |   Target=%2d   |\n", b, btb[b].tag, btb[b].target);   
    }
    printf("\n");

    printf("============ Prediction Table  ==================\n\n");

    for (int b=0; b<BTB_COUNT; b++) {
        printf("|	 PT[ %2d]	|  Pattern=%2d   |\n", b, btb[b].pattern);   
        
    }
    printf("\n");
    
}

void processPipeline(char *instrFromFile[300],CPU *cpu){
    
    cpu->cycle = 1;
    int indexOfStalledInstr = -1;
    BTB *btb = BTB_init();
    Stage *pipelineInstructions = stage_init();
    while(true){

        
        printf("================================\n");
        printf("Clock Cycle #: %d\n",cpu->cycle);
        printf("--------------------------------\n");
        

        
        // if(cpu->cycle == 45){
        //     printf("%d\n",cpu->cycle);
        // }
        resetOldInstruction(COMP,pipelineInstructions);
        bool toStop = writeBack(pipelineInstructions,cpu);
        mem2(pipelineInstructions);
        mem1(pipelineInstructions);
        branch(pipelineInstructions,btb);
        divider(pipelineInstructions);
        multiplier(pipelineInstructions);
        adder(pipelineInstructions);
        registerRead(pipelineInstructions,cpu);
        instructionAnalyse(pipelineInstructions);
        decode(pipelineInstructions);
        fetch(instrFromFile, pipelineInstructions,btb);

        indexOfStalledInstr = getStalledInstructionIndex(pipelineInstructions);

        if(indexOfStalledInstr>-1 ){

            if(calculateStructuralHazard){ cpu->structuralHazard++; }
            if(calculateDataHazard){cpu->dataHazard++;}
            resetStalledInstructions(indexOfStalledInstr,pipelineInstructions);
            
        }

        if(pipelineInstructions[COMP].index>-1){
            cpu->totalInstructions++;
        }
       
        print_display(cpu);
        //printBTBPrediction(btb);

        if(toStop){
            
            cpu->ipc = ((float)cpu->totalInstructions/(float)cpu->cycle);
            // printf("%d\n",cpu->cycle);
            break;
        }
        cpu->cycle++;
    }

}



void run_cpu_fun(char* filename){
    char *instrFromFile[300];
    CPU *cpu = CPU_init();
    readInstructionFromFile(filename,instrFromFile);
    readMemoryMapFromFile("memory_map.txt");
    processPipeline(instrFromFile,cpu);
    char opMemFileName[30]  = "mmap_";
    sprintf(opMemFileName, "%s%s", opMemFileName, filename);
    writeToFile(opMemFileName);
    CPU_run(cpu);
    CPU_stop(cpu);
}






int main(int argc, const char * argv[]) {
    
   if (argc<=1) {
        fprintf(stderr, "Error : missing required args\n");
        return -1;
    }

    char* filename = (char*)argv[1];
    
    

    
    run_cpu_fun(filename);
    
    return 0;
}









