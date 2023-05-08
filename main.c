

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
bool finalSquashed = false;
int programCounter = 0;
int totalInstructions=0;
char *memArray[16384];
bool calculateStructuralHazard = false;
bool calculateDataHazard = true;
int headIndex = 0;
int tailIndex = 0;
bool enableLogs = false;
myLogger *logger;
bool logHide = false;

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
void createLogForStage(int stage, Stage* instr){
    char stageLog[250] ="";
    char pCounter[5];
    char opcode[8];
    char destinationRegister[20] ="";
    char sourceRegister1[20] ="";
    char sourceRegister2[20] ="";
    char immVal1[20] ="";
    char immVal2[20] ="";
    char path[10]="";

    myLogger *logS = &logger[stage];
    
    switch(stage){
        case IF:
            strcpy(stageLog,instr->orignalInstruction);
            //logS->logInfo = malloc(sizeof(&stageLog));
            strcpy(logS->logInfo,stageLog);
            break;

        case ID:
        case IA:
            strcpy(pCounter,instr->pCounter);
            strncpy(opcode, instr->opcode, sizeof(instr->opcode) -1);

            if(instr->destinationRegister>-1){
                strcpy(destinationRegister, "R");
                sprintf(destinationRegister, "%s%d", destinationRegister,instr->destinationRegister);
            }
            if(instr->sourceRegister1>-1){
                strcpy(sourceRegister1, "R");
                sprintf(sourceRegister1, "%s%d", sourceRegister1, instr->sourceRegister1);
            }
            if(instr->sourceRegister2>-1){
                strcpy(sourceRegister2, "R");
                sprintf(sourceRegister2, "%s%d", sourceRegister2, instr->sourceRegister2);
            }
            if(instr->immediateValue1>INT_MIN){
                sprintf(immVal1, "imm(#%d)",instr->immediateValue1);
            }
            else{
                sprintf(immVal2, "imm()");
            }
            
            if(instr->immediateValue2>INT_MIN){
                sprintf(immVal2, "imm(#%d)", instr->immediateValue2);
            }
            
            
            sprintf(stageLog,"%s%s %s %s %s %s %s %s",
                            pCounter,
                            ":",
                            opcode,
                            strlen(destinationRegister) > 0 ? destinationRegister :" ",
                            strlen(sourceRegister1) > 0 ? sourceRegister1 :" ",
                            strlen(sourceRegister2) > 0 ? sourceRegister2 :" ",
                            strlen(immVal1) > 0 ? immVal1 :" ",
                            strlen(immVal2) > 0 ? immVal2 :"");

            //logS->logInfo = malloc(sizeof(&stageLog));
            strcpy(logS->logInfo,stageLog);
            break;

        case RR:
        case RS0:
        case RS1:
        case RS2:
        case RS3:
        case IS:
        case ADD:
        case MUL1:
        case MUL2:
        case DIV1:
        case DIV2:
        case DIV3:
        case MM1:
        case MM2:
        case MM3:
        case MM4:
            strcpy(pCounter,instr->pCounter);
            strncpy(opcode, instr->opcode, sizeof(instr->opcode) -1);

            if(instr->destinationRegister>-1){
                
                if(instr->robDest >-1 && instr->immediateValue2 >INT_MIN){
                    sprintf(destinationRegister, "%d", instr->immediateValue2);
                }
                else{
                    sprintf(destinationRegister, "%s", instr->destinationRegisterName);
                }
            }
            if(instr->sourceRegister1>-1){
                if(instr->immediateValue1 > INT_MIN){
                    sprintf(sourceRegister1, "%d", instr->immediateValue1);
                }
                else{
                    sprintf(sourceRegister1, "%s", instr->sourceRegister1Name);
                }
                
            }
            if(instr->sourceRegister2>-1){
                if(instr->immediateValue2 > INT_MIN){
                    sprintf(sourceRegister2, "%d", instr->immediateValue2);
                }
                else{
                    sprintf(sourceRegister2, "%s", instr->sourceRegister1Name);
                }
                sprintf(immVal1, "imm()",instr->immediateValue1);
            }

            if(instr->sourceRegister1>-1 && instr->sourceRegister2>-1){
                sprintf(immVal1, "imm()",instr->immediateValue1);
            }
            if(instr->sourceRegister1 <0 && instr->immediateValue1>INT_MIN){

                sprintf(immVal1, "imm(#%d)",instr->immediateValue1);
            }
            if(instr->sourceRegister2 <0 && instr->immediateValue2>INT_MIN && !checkIfBranchInstruction(createOpcodeIndex(instr->opcode))){
                sprintf(immVal2, "imm(#%d)", instr->immediateValue2);
            }
            if(stage!=IS){
                sprintf(stageLog,"%s%s %s %s %s %s %s %s",
                                pCounter,
                                ":",
                                opcode,
                                strlen(destinationRegister) > 0 ? destinationRegister :" ",
                                strlen(sourceRegister1) > 0 ? sourceRegister1 :" ",
                                strlen(sourceRegister2) > 0 ? sourceRegister2 :" ",
                                strlen(immVal1) > 0 ? immVal1 :" ",
                                strlen(immVal2) > 0 ? immVal2 :"");
            }
            else if(stage == IS){
                 
                 if(instr->path == ADD){
                    strcpy(path,"=> Add");
                 }else if(instr->path == MUL1){
                    strcpy(path,"=> Mul 1");
                 }else if(instr->path == DIV1){
                    strcpy(path,"=> Div 1");
                 }else if(instr->path == MM1){
                    strcpy(path,"=> Mem 1");
                 }
                 sprintf(stageLog,"%s%s %s %s %s %s %s %s %s",
                                pCounter,
                                ":",
                                opcode,
                                strlen(destinationRegister) > 0 ? destinationRegister :" ",
                                strlen(sourceRegister1) > 0 ? sourceRegister1 :" ",
                                strlen(sourceRegister2) > 0 ? sourceRegister2 :" ",
                                strlen(immVal1) > 0 ? immVal1 :" ",
                                strlen(immVal2) > 0 ? immVal2 :"",
                                path);
            }

            //logS->logInfo = malloc(sizeof(&stageLog));
            strcpy(logS->logInfo,stageLog);
            break;
        
        case WB1:
        case WB2:
        case WB3:
        case WB4:
            strcpy(pCounter,instr->pCounter);
            strncpy(opcode, instr->opcode, sizeof(instr->opcode) -1);
            if(checkIfBranchInstruction(createOpcodeIndex(instr->opcode))
                || createOpcodeIndex(instr->opcode) == ret){

                     sprintf(stageLog,"%s%s %s",
                                pCounter,
                                ":",
                                "No ROB update");
                
                //logS->logInfo = malloc(sizeof(&stageLog));
                strcpy(logS->logInfo,stageLog);            

            }else{
               if(instr->destinationRegister>-1){
                    sprintf(destinationRegister, "%s", instr->destinationRegisterName);
                }
                
                sprintf(stageLog,"%s %s%s %s %s",
                                pCounter,
                                opcode,
                                ":",
                                "update",
                                strlen(destinationRegister) > 0 ? destinationRegister :" ");
                
                //logS->logInfo = malloc(sizeof(&stageLog));
                strcpy(logS->logInfo,stageLog);
            }                
            break;

        case RE1:
        case RE2:
            strcpy(pCounter,instr->pCounter);
            if(checkIfBranchInstruction(createOpcodeIndex(instr->opcode)) 
                || createOpcodeIndex(instr->opcode) == ret){
                 sprintf(stageLog,"%s%s %s",
                                pCounter,
                                ":",
                                "No register update");
                
                //logS->logInfo = malloc(sizeof(&stageLog));
                strcpy(logS->logInfo,stageLog);
            }
            else{
                
                if(instr->destinationRegister>-1){
                    strcpy(destinationRegister, "R");
                    sprintf(destinationRegister, "%s%d", destinationRegister,instr->destinationRegister);
                }
                
                sprintf(stageLog,"%s%s %s %s %s (%d)",
                                pCounter,
                                ":",
                                "update",
                                "rd",
                                strlen(destinationRegister) > 0 ? destinationRegister :" ",
                                instr->tempResult);
                
                //logS->logInfo = malloc(sizeof(&stageLog));
                strcpy(logS->logInfo,stageLog);
            } 
            break;
    }


}

void incrementHeadIndex(){
    headIndex = headIndex >=7? 0: headIndex+1;   
}
void incrementTailIndex(){
    tailIndex = tailIndex >=7? 0: tailIndex+1;
}
void logStageAndInstruction(int stage, char* instruction){
    // char *strStage;
    // if(stage == IF){
    //     strStage ="IF  ";
    // }if(stage == ID){
    //     strStage ="ID  ";
    // }if(stage == IA){
    //     strStage ="IA  ";
    // }if(stage == RR){
    //     strStage ="RR  ";
    // }if(stage == ADD){
    //     strStage ="ADD ";
    // }if(stage == MUL){
    //     strStage ="MUL ";
    // }if(stage == DIV){
    //     strStage ="DIV ";
    // }if(stage == BR){
    //     strStage ="BR  ";
    // }if(stage == MM1){
    //     strStage ="Mem1";
    // }if(stage == MM2){
    //     strStage ="Mem2";
    // }if(stage == WB){
    //     strStage ="WB  ";
    // }
    
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
        oldInstruction->robDest=-1;
        oldInstruction->immediateValue1 = INT_MIN;
        oldInstruction->immediateValue2 = INT_MIN;
        oldInstruction->opcode ="";
        oldInstruction->orignalInstruction=""; 
        for (int i=0; i<33;i++){
            oldInstruction->pcBinary[i] = 0;
        }
        oldInstruction->pcIndex = -1;
        oldInstruction->sourceRegister1 = -1;
        oldInstruction->sourceRegister2=-1;
        oldInstruction->robSr1=-1;
        oldInstruction->robSr2=-1;

        oldInstruction->stallType = -1;
        oldInstruction->tag = -1;
        oldInstruction->tempResult = INT_MIN;
        oldInstruction->type = IF;

        oldInstruction->path = -1;
        strcpy(oldInstruction->sourceRegister1Name,"");
        strcpy(oldInstruction->sourceRegister2Name,"");
        strcpy(oldInstruction->destinationRegisterName,"");

        

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


void
log_init()
{   
    logger = malloc(sizeof(*logger) * (STAGE_COUNT));
    if (!logger) {
        
    }
    for (int i=0; i<STAGE_COUNT; i++){
        logger[i].index = -1;
        strcpy(logger[i].logInfo,"");
        logger[i].type = -1;
    }
}

Stage*
inorder_stage_init()
{   
    Stage* stage = malloc(sizeof(*stage) * (STAGE_COUNT));
    if (!stage) {
        return NULL;
    }
    for (int i=IF; i<22; i++){
        stage[i].type = i;
        stage[i].index = -1;
        
    }
    return stage;
}

bool squashInstruction( Stage  *instr){
    bool instrSquashed = true;
    for(int i = IF; i< RR; i++ ){

        resetOldInstruction(i,instr);
        
    }
    return instrSquashed;
}

void branch(Stage  *instr,BTB *btb){

    
     Stage *rrInstruction = &instr[RR]; 
    

    if(rrInstruction->index > -1 ){


            if(checkIfBranchInstruction(createOpcodeIndex(rrInstruction->opcode)) ){

                rrInstruction->stallType = NO_HAZARD;
                
                bool toPredict = false;

                switch (createOpcodeIndex(rrInstruction->opcode))
                {
                                case bez:
                                    if(rrInstruction->immediateValue2 == 0 && rrInstruction->immediateValue2 != INT_MIN){
                                        toPredict = true;
                                    }
                                    break;
                                case bgez:
                                    if(rrInstruction->immediateValue2 >= 0 && rrInstruction->immediateValue2 != INT_MIN){
                                        toPredict = true;
                                    }
                                    break;

                                case blez:
                                    if(rrInstruction->immediateValue2 <= 0 && rrInstruction->immediateValue2 != INT_MIN){
                                        toPredict = true;
                                    }
                                    break;
                                case bgtz:
                                    if(rrInstruction->immediateValue2 > 0 && rrInstruction->immediateValue2 != INT_MIN){
                                        toPredict = true;
                                    }
                                    break;
                                case bltz:
                                    if(rrInstruction->immediateValue2 < 0 && rrInstruction->immediateValue2 != INT_MIN){
                                        toPredict = true;
                                    }
                                    break;

                                default:
                                    break;
                }   
                
            
                int pc = rrInstruction->pcIndex;

                if(toPredict){
                    if(pc >-1){
                        if(btb[pc].pattern >=4){
                            btb[pc].pattern = btb[pc].pattern >=7? btb[pc].pattern: btb[pc].pattern+1;
                        }
                        else{
                            btb[pc].pattern = btb[pc].pattern+1; 
                            btb[pc].tag =rrInstruction->tag;
                            btb[pc].target = rrInstruction->immediateValue1;
                            instructionSquashed = squashInstruction(instr);
                            programCounter =rrInstruction->immediateValue1;
                            
                        }
                    }
                }
                else{
                    if(pc>-1){
                        int orignalBtb = btb[pc].pattern;
                        btb[pc].pattern = btb[pc].pattern ==0? btb[pc].pattern :btb[pc].pattern-1;
                        if(orignalBtb>=4){
                            instructionSquashed = squashInstruction(instr);
                            programCounter =rrInstruction->counter+4;
                        }
                        else{
                            btb[pc].tag =rrInstruction->tag;
                            btb[pc].target = rrInstruction->immediateValue1;
                        }

                    }
                }
            }
            if(rrInstruction->immediateValue2 >INT_MIN){
                rrInstruction->stallType = NO_HAZARD;
            }
            else{
                rrInstruction->stallType = DATA_HAZARD;
            }
           
               
        
    }
    
}


void copyData(Stage* newInstruction, Stage* presentInstruction){

    newInstruction->index = presentInstruction->index ;
    newInstruction->counter = presentInstruction->counter;
    
    
    strcpy(newInstruction->pCounter, presentInstruction->pCounter); 

    newInstruction->opcode = malloc(sizeof(presentInstruction->opcode));
    strcpy(newInstruction->opcode, presentInstruction->opcode);
    
    newInstruction->orignalInstruction = malloc(sizeof(presentInstruction->orignalInstruction));
    strcpy(newInstruction->orignalInstruction, presentInstruction->orignalInstruction);

    for (int i=0; i<33;i++){
        newInstruction->pcBinary[i] = presentInstruction->pcBinary[i];
    }
    newInstruction->pcIndex = presentInstruction->pcIndex;
    
    newInstruction->stallType = presentInstruction->stallType;
    newInstruction->tag = presentInstruction->tag;
    newInstruction->tempResult = presentInstruction->tempResult;
    newInstruction->path = presentInstruction->path;
    

    newInstruction->sourceRegister1 = presentInstruction->sourceRegister1;
    newInstruction->sourceRegister2=presentInstruction->sourceRegister2;
    newInstruction->destinationRegister = presentInstruction->destinationRegister;
    newInstruction->robDest = presentInstruction->robDest;
    newInstruction->robSr1 = presentInstruction->robSr1;
    newInstruction->robSr2 = presentInstruction->robSr2;

    strcpy(newInstruction->sourceRegister1Name, presentInstruction->sourceRegister1Name);
    strcpy(newInstruction->sourceRegister2Name, presentInstruction->sourceRegister2Name);
    strcpy(newInstruction->destinationRegisterName, presentInstruction->destinationRegisterName);

    newInstruction->immediateValue1 = presentInstruction->immediateValue1;
    newInstruction->immediateValue2 = presentInstruction->immediateValue2; 
    
    for (int i=0; i<11;i++){
        newInstruction->dependentInstructionInfo[i].depRegister = presentInstruction->dependentInstructionInfo[i].depRegister;
        newInstruction->dependentInstructionInfo[i].index = presentInstruction->dependentInstructionInfo[i].index;
        strcpy(newInstruction->dependentInstructionInfo[i].name, presentInstruction->dependentInstructionInfo[i].name);
        newInstruction->dependentInstructionInfo[i].robIndex = presentInstruction->dependentInstructionInfo[i].robIndex;
    }
}

void pushDataToNewStage(int presentStage,int newStage,Stage *stage){
    Stage *presentInstruction = &stage[presentStage];
    Stage *newInstruction = NULL;

    newInstruction = &stage[newStage];    
    if(presentInstruction->index >-1){
        newInstruction->type = newStage;
        copyData(newInstruction,presentInstruction);
        resetOldInstruction(presentStage,stage);
    }
}

int getStalledInstructionIndex(Stage *pipelineInstructions){
    int stalledInstrIndex = -1;
    for(int i=IF; i<RE1;i++){
        Stage *filterInstructions = &pipelineInstructions[i]; 

        if(filterInstructions->stallType >NO_HAZARD){
            stalledInstrIndex = filterInstructions->index;
            break;
        }
    }
    return stalledInstrIndex;
}

int getStageFromIndex(int index,Stage* instr){
    int stage = -1;

    if(index >-1){
        for(int i= IF ;i<STAGE_COUNT;i++){
            if(instr[i].index == index){
                stage = i;
            }
        }
    }
    return stage;
}


void resetStalledInstructions(int indexOfStalledInstr,Stage *pipelineInstructions){
    Stage *filterInstructions = NULL;
    for(int i=IF;i<RE1;i++){

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

                            if(pipelineInstructions[RE1].index == dependentIndex){
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
bool checkIfReservationStationFull(Stage *instr){
        bool isFull= false;
        Stage *rs0Instruction = &instr[RS0]; 
        Stage *rs1Instruction = &instr[RS1]; 
        Stage *rs2Instruction = &instr[RS2]; 
        Stage *rs3Instruction = &instr[RS3];

        if(rs0Instruction->index >-1 && rs1Instruction->index>-1 && rs2Instruction->index>-1 && rs3Instruction->index>-1){
            isFull = true;
        }
        return isFull;
}
void renameInstruction(Stage *rrInstruction,ROB *rob,CPU *cpu){

    for(int i=0;i<ROB_COUNT;i++){

        if(rrInstruction->destinationRegister>-1 && rrInstruction->robDest >-1){
            break;
        }
        else if(rrInstruction->robDest <0 && rrInstruction->destinationRegister>-1 && strlen(rrInstruction->destinationRegisterName) == 0){
            strcpy(rrInstruction->destinationRegisterName,rob[tailIndex].bufferName);
            rob[tailIndex].completed = false;
            rob[tailIndex].dest = rrInstruction->destinationRegister;
            rob[tailIndex].index = rrInstruction->index;
            cpu->regs[rrInstruction->destinationRegister].is_writing = false;
            cpu->regs[rrInstruction->destinationRegister].tag = tailIndex;
            incrementTailIndex();
            break;
            
        }

        
    }

    if(rrInstruction->sourceRegister1>-1){

        if(rrInstruction->robSr1 >-1){
            strcpy(rrInstruction->sourceRegister1Name,rob[rrInstruction->robSr1].bufferName);

            if(rob[rrInstruction->robSr1].completed){
                rrInstruction->immediateValue1 = rob[rrInstruction->robSr1].result;
            }

        }else{
            rrInstruction->immediateValue1 = cpu->regs[rrInstruction->sourceRegister1].value;
        }

    }
    if(rrInstruction->sourceRegister2>-1){

        if(rrInstruction->robSr2 >-1){
            strcpy(rrInstruction->sourceRegister2Name,rob[rrInstruction->robSr2].bufferName);

            if(rob[rrInstruction->robSr2].completed){
                rrInstruction->immediateValue2 = rob[rrInstruction->robSr2].result;
            }
            
        }
        else{
            rrInstruction->immediateValue2 = cpu->regs[rrInstruction->sourceRegister2].value;
        }

    }
    if(rrInstruction->destinationRegister>-1 
        && (checkIfBranchInstruction(createOpcodeIndex(rrInstruction->opcode)) 
        || createOpcodeIndex(rrInstruction->opcode) == st)){

         if(rrInstruction->robDest >-1){
            strcpy(rrInstruction->destinationRegisterName,rob[rrInstruction->robDest].bufferName);

            if(rob[rrInstruction->robDest].completed){
                rrInstruction->immediateValue2 = rob[rrInstruction->robDest].result;
            }
            
        }
        else{
            rrInstruction->immediateValue2 = cpu->regs[rrInstruction->sourceRegister2].value;
        }

    }
    
       
}
bool dispatch(Stage *instr, ROB* rob){
        Stage *rs0Instruction = &instr[RS0]; 
        Stage *rs1Instruction = &instr[RS1]; 
        Stage *rs2Instruction = &instr[RS2]; 
        Stage *rs3Instruction = &instr[RS3]; 

        if(rs0Instruction->index > -1 ){

            if(rs0Instruction->immediateValue1==INT_MIN){
                if(rs0Instruction->robSr1>-1){
                    int instrIndex =rob[rs0Instruction->robSr1].index;
                    int stageI =getStageFromIndex(instrIndex,instr);
                    if(stageI == WB1 || stageI == WB2 || stageI == WB3 || stageI == WB4){
                        Stage *dependentInstr = &instr[stageI];
                        rs0Instruction->immediateValue1 = dependentInstr->tempResult;
                    }
                }
            }
            if(rs0Instruction->immediateValue2 == INT_MIN){
                if(rs0Instruction->robSr2>-1){
                    int instrIndex =rob[rs0Instruction->robSr2].index;
                    int stageI =getStageFromIndex(instrIndex,instr);
                    if(stageI == WB1 || stageI == WB2 || stageI == WB3 || stageI == WB4){
                        Stage *dependentInstr = &instr[stageI];
                        rs0Instruction->immediateValue2 = dependentInstr->tempResult;
                    }
                }                
            }
           
            if(rs0Instruction->immediateValue1 >INT_MIN && rs1Instruction->immediateValue2 >INT_MIN){
                pushDataToNewStage(RS0,IS,instr);
            }
            else if(createOpcodeIndex(rs0Instruction->opcode) == ret){
                pushDataToNewStage(RS0,IS,instr);   
            }

            if(rs0Instruction->index <0 && rs1Instruction->index >-1){
                pushDataToNewStage(RS1,RS0,instr);
                rs0Instruction = &instr[RS0];
                createLogForStage(RS0,rs0Instruction);
            }    
            if(rs1Instruction->index <0 && rs2Instruction->index >-1){
                pushDataToNewStage(RS2,RS1,instr);
                rs1Instruction = &instr[RS1];
                createLogForStage(RS1,rs1Instruction);
            }
            if(rs2Instruction->index <0 && rs3Instruction->index >-1){
                pushDataToNewStage(RS3,RS2,instr);
                rs2Instruction = &instr[RS2];
                createLogForStage(RS2,rs2Instruction);
            }
        }
}
bool addToReserveStation(Stage *instr){

    Stage *rrInstruction = &instr[RR]; 
    bool spaceFull = false;

    bool toStall = false;

    if(rrInstruction->stallType >0){
        toStall = true;
    }

    if(rrInstruction->index >-1 && !toStall){
        Stage *rs0Instruction = &instr[RS0]; 
        Stage *rs1Instruction = &instr[RS1]; 
        Stage *rs2Instruction = &instr[RS2]; 
        Stage *rs3Instruction = &instr[RS3]; 
        if(rs0Instruction->index <0){
            pushDataToNewStage(RR,RS0,instr);
            rs0Instruction = &instr[RS0];
            createLogForStage(RS0,rs0Instruction);
        }
        else if(rs1Instruction->index <0){
            pushDataToNewStage(RR,RS1,instr);
            rs1Instruction = &instr[RS1];
            createLogForStage(RS1,rs1Instruction);
        }
        else if(rs2Instruction->index <0){
            pushDataToNewStage(RR,RS2,instr);
            rs2Instruction = &instr[RS2];
            createLogForStage(RS2,rs2Instruction);
        }
        else if(rs3Instruction->index <0){
            pushDataToNewStage(RR,RS3,instr);
            rs3Instruction = &instr[RS3];
            createLogForStage(RS3,rs3Instruction);
        }
        else{
            spaceFull = true;
        }
    }

    
}

void fetch(char *instrFromFile[300],Stage  *pipelineInstr,BTB *btb){

    int index = 0;

    if(programCounter>0){
        index = programCounter/4;
    }

    if(finalSquashed){
        index = -1;
        logHide = true;
    }
    
    if(instructionSquashed){
        index = -1;
        instructionSquashed = false;
        logHide = true;
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
                    //logStageAndInstruction(IF,fetchInstruction->orignalInstruction);
                    createLogForStage(IF,fetchInstruction);
                }
            }

            if(fetchInstruction->stallType == STRUCTURAL_HAZARD){  //ToVisit.
                stall = true;
            }
            
            if(!stall){
                fetchInstruction->index = index;
                fetchInstruction->orignalInstruction = malloc(sizeof(&instrFromFile[index]));
                strcpy(fetchInstruction->orignalInstruction, instrFromFile[index]);

                //logStageAndInstruction(IF,fetchInstruction->orignalInstruction);
                
                fetchInstruction->counter = programCounter;
                decimalToBinary(fetchInstruction->counter,32,fetchInstruction->pcBinary);
                fetchInstruction->pcIndex = (fetchInstruction->counter  >> 2) & 0b1111;
                fetchInstruction->tag = fetchInstruction->counter  >> 6;

                fetchInstruction->sourceRegister1 = -1;
                fetchInstruction->sourceRegister2 =-1;
                fetchInstruction->destinationRegister =-1;            
                fetchInstruction->immediateValue1 =INT_MIN;
                fetchInstruction->immediateValue2 =INT_MIN; 
                fetchInstruction->robDest = -1;
                fetchInstruction->robSr1=-1;
                fetchInstruction->robSr2 = -1;

                strcpy(fetchInstruction->sourceRegister1Name,"");
                strcpy(fetchInstruction->sourceRegister2Name,"");
                strcpy(fetchInstruction->destinationRegisterName,"");


                fetchInstruction->path = -1;

                fetchInstruction->stallType =NO_HAZARD;
                fetchInstruction->tempResult = INT_MIN;
                fetchInstruction->opcode = "";


                char substr[5];

                sscanf(fetchInstruction->orignalInstruction, "%*s %s", substr);

                bool isBranch =checkIfBranchInstruction(createOpcodeIndex(substr));
                

                for(int i=0;i<11;i++){
                    fetchInstruction->dependentInstructionInfo[i].index = -1;
                    fetchInstruction->dependentInstructionInfo[i].depRegister = -1;
                    fetchInstruction->dependentInstructionInfo[i].tempResult =INT_MIN;
                    fetchInstruction->dependentInstructionInfo[i].robIndex = -1;
                    strcpy(fetchInstruction->dependentInstructionInfo[i].name,"");
                }


                if(isBranch && btb[fetchInstruction->pcIndex].pattern>=4){
                    programCounter = btb[fetchInstruction->pcIndex].target;
                }
                else{
                    programCounter = programCounter+4;
                }
                createLogForStage(IF,fetchInstruction);
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
                //logStageAndInstruction(ID,decodeInstruction->orignalInstruction);   
                //createLogForStage(ID,decodeInstruction);
                stall = true;
            }
        }

        if(decodeInstruction->stallType == STRUCTURAL_HAZARD){
            stall = true;
        }

        

                    //logStageAndInstruction(ID,decodeInstruction->orignalInstruction); 
                    char* backupOrgInstr = malloc(sizeof(decodeInstruction->orignalInstruction));
                    strcpy(backupOrgInstr, decodeInstruction->orignalInstruction);  
                    char *ptr = strtok(backupOrgInstr, " ");
                    int loopIndex = 0;
                    while(ptr!=NULL){

                        if(loopIndex == 0){
                            strcpy(decodeInstruction->pCounter, ptr);
                        }

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
                                        if(decodeInstruction->immediateValue1 == INT_MIN && decodeInstruction->sourceRegister1 == -1){
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
                    createLogForStage(ID,decodeInstruction);
                    
                    if(!stall){
                        pushDataToNewStage(ID,IA,instr);
                    }
                    
        
   }
    

}
void instructionAnalyse(Stage  *instr, ROB *rob){
    
    Stage *analyseInstruction = &instr[IA]; 
    bool stall = false;
    if(analyseInstruction->index > -1){
        
        Stage *rrInstruction = &instr[RR]; 
        if(rrInstruction->index >-1 ){
            if(rrInstruction->stallType == DATA_HAZARD){
                stall = true;
            }
        }

        //logStageAndInstruction(IA,analyseInstruction->orignalInstruction);
        createLogForStage(IA,analyseInstruction);

        if(!stall){
                             
            if(calculateDataHazard){
                
                    int arrayIndex = 0;

                    for(int i=0; i<ROB_COUNT;i++){    

                        int dest =rob[i].dest;
                        
                        int stage = getStageFromIndex(rob[i].index,instr);

                        if(dest>-1 && stage< RE1 ){
                            if(checkIfBranchInstruction(createOpcodeIndex(analyseInstruction->opcode)) && dest == analyseInstruction->destinationRegister){
                                // analyseInstruction->dependentInstructionInfo[arrayIndex].robIndex = i;
                                // analyseInstruction->dependentInstructionInfo[arrayIndex].depRegister = dest;  
                                // analyseInstruction->dependentInstructionInfo[arrayIndex].index = rob[i].index;  
                                analyseInstruction->robDest = i;
                                //strcpy(analyseInstruction->dependentInstructionInfo[arrayIndex].name,rob[i].bufferName);
                                
                            }
                            else if(createOpcodeIndex(analyseInstruction->opcode) == st && dest == analyseInstruction->destinationRegister){
                                // analyseInstruction->dependentInstructionInfo[arrayIndex].robIndex = i;
                                // analyseInstruction->dependentInstructionInfo[arrayIndex].depRegister = dest;  
                                // analyseInstruction->dependentInstructionInfo[arrayIndex].index = rob[i].index;
                                analyseInstruction->robDest = i;
                                //strcpy(analyseInstruction->dependentInstructionInfo[arrayIndex].name,rob[i].bufferName);
                            }
                            else{
                                if(dest == analyseInstruction->sourceRegister1 ){
                                    // analyseInstruction->dependentInstructionInfo[arrayIndex].robIndex = i;
                                    // analyseInstruction->dependentInstructionInfo[arrayIndex].depRegister = dest;
                                    // analyseInstruction->dependentInstructionInfo[arrayIndex].index = rob[i].index;
                                    analyseInstruction->robSr1 = i;
                                    //strcpy(analyseInstruction->dependentInstructionInfo[arrayIndex].name,rob[i].bufferName);   
                                }
                                if(dest == analyseInstruction->sourceRegister2){
                                    // analyseInstruction->dependentInstructionInfo[arrayIndex].robIndex = i;
                                    // analyseInstruction->dependentInstructionInfo[arrayIndex].depRegister = dest;
                                    // analyseInstruction->dependentInstructionInfo[arrayIndex].index = rob[i].index;
                                    analyseInstruction->robSr2 = i;
                                    //strcpy(analyseInstruction->dependentInstructionInfo[arrayIndex].name,rob[i].bufferName);   
                                }
                            }
                        }
                        
                    } 
                
                
            }
                                        
            createLogForStage(IA,analyseInstruction);
            pushDataToNewStage(IA,RR,instr);
      
        }
    }
}



void registerRead(Stage  *instr,CPU *cpu,ROB *rob,BTB *btb){
    Stage *rrInstruction = &instr[RR]; 
    bool stall = false;
    if(rrInstruction->index >-1){
        

        bool isFull = checkIfReservationStationFull(instr);

        
        if(isFull){
            stall = true;
            rrInstruction->stallType == FULL_RESERVATION_STATION;
            createLogForStage(RR,rrInstruction);
        }

        if(!stall){        
            renameInstruction(rrInstruction,rob,cpu);

            if(checkIfBranchInstruction(createOpcodeIndex(rrInstruction->opcode))){
                branch(instr,btb);
            }
            else if(createOpcodeIndex(rrInstruction->opcode) == ret){
                finalSquashed =squashInstruction(instr);
            }
            
            createLogForStage(RR,rrInstruction);
            addToReserveStation(instr); 
        }  
        dispatch(instr,rob);                         
    }
}
void issue(Stage  *instr, ROB* rob){
    Stage *issueInstruction = &instr[IS];

    if(issueInstruction->index >-1){

        

        switch (createOpcodeIndex(issueInstruction->opcode))
        {
            case set:
            case add:
            case sub:
            case bez:
            case bgez:
            case bgtz:
            case blez:
            case bltz:
            case ret:
                if(checkIfBranchInstruction(createOpcodeIndex(issueInstruction->opcode))){
                    //  rob[tailIndex].completed = false;
                    //  incrementTailIndex();
                }
                issueInstruction->path = ADD;
                createLogForStage(IS,issueInstruction);  
                pushDataToNewStage(IS,ADD,instr);                          
                break;

            case mul:
                issueInstruction->path = MUL1;
                createLogForStage(IS,issueInstruction);  
                pushDataToNewStage(IS,MUL1,instr);
                break;

            case div:
                issueInstruction->path = DIV1;
                createLogForStage(IS,issueInstruction);  
                pushDataToNewStage(IS,DIV1,instr);
                break;
            
            case ld:
            case st:
                issueInstruction->path = MM1;
                createLogForStage(IS,issueInstruction);  
                pushDataToNewStage(IS,MM1,instr);
                break;
        
            default:
                break;
        }
    }
} 
void adder(Stage  *instr){
    Stage *addInstruction = &instr[ADD]; 

    if(addInstruction->index > -1){

            //logStageAndInstruction(ADD,addInstruction->orignalInstruction);
            createLogForStage(ADD,addInstruction);
            
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
            pushDataToNewStage(ADD,WB1,instr);
                            
        
    }
}

void multiplier1(Stage  *instr){
    Stage *mulInstruction1 = &instr[MUL1]; 
    
    if(mulInstruction1->index > -1){
        createLogForStage(MUL1,mulInstruction1);

            switch (createOpcodeIndex(mulInstruction1->opcode))
            {
                            case mul:
                                break;
                        
                            default:
                                break;
                                

            }      
            pushDataToNewStage(MUL1,MUL2,instr);        
                            
        
    }
}
void multiplier2(Stage  *instr){
    Stage *mulInstruction2 = &instr[MUL2]; 
    Stage *wb1Instruction = &instr[WB1]; 
    Stage *wb2Instruction = &instr[WB2]; 
    
    if(mulInstruction2->index > -1){

            createLogForStage(MUL2,mulInstruction2);
            switch (createOpcodeIndex(mulInstruction2->opcode))
            {
                            case mul:
                                    mulInstruction2->tempResult = mulInstruction2->immediateValue1 * mulInstruction2->immediateValue2;
                                break;
                        
                            default:
                                break;
                                

            }      
            
            
            
                pushDataToNewStage(MUL2,WB2,instr);
                                                        
    }
}


void divider1(Stage  *instr){

    Stage *divInstruction1 = &instr[DIV1]; 

    if(divInstruction1->index > -1){

            createLogForStage(DIV1,divInstruction1);
            
            switch (createOpcodeIndex(divInstruction1->opcode))
            {
                            case div:
                                    //divInstruction1->tempResult = divInstruction1->immediateValue1 / divInstruction1->immediateValue2;
                                break;
                        
                            default:
                                break;
                                

            }    

            pushDataToNewStage(DIV1,DIV2,instr);              
                            
        
    }
}
void divider2(Stage  *instr){

    Stage *divInstruction2 = &instr[DIV2]; 

    if(divInstruction2->index > -1){
            
            createLogForStage(DIV2,divInstruction2);
            switch (createOpcodeIndex(divInstruction2->opcode))
            {
                            case div:
                                    //divInstruction1->tempResult = divInstruction1->immediateValue1 / divInstruction1->immediateValue2;
                                break;
                        
                            default:
                                break;
                                

            }    

            pushDataToNewStage(DIV2,DIV3,instr);              
                            
        
    }
}
void divider3(Stage  *instr){

    Stage *divInstruction3 = &instr[DIV3]; 
    Stage *wb1Instruction = &instr[WB1]; 
    Stage *wb2Instruction = &instr[WB2]; 

    if(divInstruction3->index > -1){

            createLogForStage(DIV3,divInstruction3);
            
            switch (createOpcodeIndex(divInstruction3->opcode))
            {
                            case div:
                                    divInstruction3->tempResult = divInstruction3->immediateValue1 / divInstruction3->immediateValue2;
                                break;
                        
                            default:
                                break;
                                

            }    

            
                pushDataToNewStage(DIV3,WB3,instr);
            

                            
        
    }
}
void mem1(Stage  *instr){

    Stage *mm1Instruction = &instr[MM1]; 

    if(mm1Instruction->index > -1){

            //logStageAndInstruction(MM1,mm1Instruction->orignalInstruction);

            createLogForStage(MM1,mm1Instruction);

            switch (createOpcodeIndex(mm1Instruction->opcode))
            {
                            case ld:
                            case st:
                                     //mm1Instruction->immediateValue1 = ((mm1Instruction->immediateValue1)/4);
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
    
            createLogForStage(MM2,mm2Instruction);
            switch (createOpcodeIndex(mm2Instruction->opcode))
            {
                            case ld:
                            case st:
                                    // mm2Instruction->immediateValue1 = ((mm2Instruction->immediateValue1)/4);
                                break;

                            default:
                                break;                                
            }   

            pushDataToNewStage(MM2,MM3,instr);             
                            
        
    }
}
void mem3(Stage  *instr){

    Stage *mm3Instruction = &instr[MM3]; 

    if(mm3Instruction->index > -1){


            createLogForStage(MM3,mm3Instruction);
            //logStageAndInstruction(MM1,mm1Instruction->orignalInstruction);

            switch (createOpcodeIndex(mm3Instruction->opcode))
            {
                            case ld:
                            case st:
                                     mm3Instruction->immediateValue1 = ((mm3Instruction->immediateValue1)/4);
                                break;

                            default:
                                break;                                
            }   

            pushDataToNewStage(MM3,MM4,instr);             
                            
        
    }
}

void mem4(Stage  *instr){

    Stage *mm4Instruction = &instr[MM4]; 

    if(mm4Instruction->index > -1){

            createLogForStage(MM4,mm4Instruction);
            //logStageAndInstruction(MM2,mm2Instruction->orignalInstruction);
            char snum[20];
            switch (createOpcodeIndex(mm4Instruction->opcode))
            {
                            case ld:
                                mm4Instruction->tempResult = atoi(memArray[mm4Instruction->immediateValue1]);
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
                                sprintf(snum, "%d", mm4Instruction->immediateValue2);
                                strncpy(memArray[mm4Instruction->immediateValue1], snum, sizeof(snum));
                                break;
                                                        
                            default:
                                break;                                
            }   

            pushDataToNewStage(MM4,WB1,instr);              
                            
        
    }
}
void wb(Stage* writeback, ROB *rob){

        if(strcmp(writeback->destinationRegisterName,rob[headIndex].bufferName) == 0){
        
            switch (createOpcodeIndex(writeback->opcode))
            {
                                case add:    
                                case mul:
                                case sub:
                                case div:
                                case set:
                                case ld:
                                        
                                        rob[headIndex].result = writeback->tempResult;
                                        rob[headIndex].completed = true;
                                        int tempHead = headIndex+1;
                                        if(tempHead == tailIndex){
                                            rob[tailIndex].completed = false;
                                            incrementTailIndex();
                                        }

                                    break;
                                        
                                case ret:
                                    break;    
                                default:
                                    
                                    break;
                                    

            } 
           

            
        }
        else{
            rob[headIndex].completed = true;
        }
}
void writeBack(Stage *instr, ROB* rob){
    Stage *wb1Instruction = &instr[WB1];
    Stage *wb2Instruction = &instr[WB2];
    Stage *wb3Instruction = &instr[WB3];
    Stage *wb4Instruction = &instr[WB4];
    Stage *re1Instr = &instr[RE1];
    Stage *re2Instr = &instr[RE1];


    if(wb1Instruction->index >-1){
        createLogForStage(WB1,wb1Instruction);
        wb(wb1Instruction,rob);
        if(re1Instr->index<0){
            pushDataToNewStage(WB1,RE1,instr);
        }else if(re2Instr->index <0){
            pushDataToNewStage(WB1,RE2,instr);
        }
    }
    else if(wb2Instruction->index >-1){
        createLogForStage(WB2,wb2Instruction);
        wb(wb2Instruction,rob);
        if(re1Instr->index<0){
            pushDataToNewStage(WB2,RE1,instr);
        }else if(re2Instr->index <0){
            pushDataToNewStage(WB2,RE2,instr);
        }
    }
    else if(wb3Instruction->index >-1){
        createLogForStage(WB3,wb3Instruction);
        wb(wb3Instruction,rob);
        if(re1Instr->index<0){
            pushDataToNewStage(WB3,RE1,instr);
        }else if(re2Instr->index <0){
            pushDataToNewStage(WB3,RE2,instr);
        } 
    }
    else if(wb4Instruction->index >-1){
        createLogForStage(WB4,wb4Instruction);
        wb(wb4Instruction,rob);
        if(re1Instr->index<0){
            pushDataToNewStage(WB4,RE1,instr);
        }else if(re2Instr->index <0){
            pushDataToNewStage(WB4,RE2,instr);
        } 
    }
    
    
}

bool retire1(Stage *instr,ROB* rob,CPU *cpu){

    Stage *retire1Instruction = &instr[RE1];
    bool toStop = false;

    if(retire1Instruction->index >-1){

        createLogForStage(RE1,retire1Instruction);


        if(strcmp(retire1Instruction->destinationRegisterName,rob[headIndex].bufferName) == 0
           && rob[headIndex].completed){
            cpu->regs[retire1Instruction->destinationRegister].is_writing = true;
            cpu->regs[retire1Instruction->destinationRegister].value = rob[headIndex].result;
            cpu->regs[retire1Instruction->destinationRegister].tag = -1;
            rob[headIndex].result = -1;
            rob[headIndex].dest = -1;
            rob[headIndex].index = -1;
            rob[headIndex].completed = false;
            incrementHeadIndex();

            
        
        }
        else{
            rob[headIndex].completed = false;
            incrementHeadIndex();
        }

        if(createOpcodeIndex(retire1Instruction->opcode) == ret){
            toStop = true;
        }
        pushDataToNewStage(RE1,COMP,instr);  
        

    }
    return toStop;

}

void retire2(Stage *instr,ROB* rob,CPU *cpu){
    Stage *retire2Instruction = &instr[RE2];

    if(retire2Instruction->index >-1){

        createLogForStage(RE2,retire2Instruction);

        if(strcmp(retire2Instruction->destinationRegisterName,rob[headIndex].bufferName) == 0
           && rob[headIndex].completed){

            cpu->regs[retire2Instruction->destinationRegister].value = rob[headIndex].result;
            cpu->regs[retire2Instruction->destinationRegister].is_writing = true;
            cpu->regs[retire2Instruction->destinationRegister].tag = -1;
            rob[headIndex].result = -1;
            rob[headIndex].dest = -1;
            rob[headIndex].completed = false;
            incrementHeadIndex();

            
        }else{

            rob[headIndex].completed = false;
            incrementHeadIndex();
        }
        pushDataToNewStage(RE2,COMP,instr);  
        
    }
    
}


ROB*
ROB_init()
{   
    ROB* robIn = malloc(sizeof(*robIn) * ROB_COUNT);
    if (!robIn) {
        return NULL;
    }
    
    for (int i=0; i<ROB_COUNT; i++){
        char robName[5]  = "ROB";    
        char index[2];
        sprintf(index, "%d", i);
        sprintf(robName, "%s%s", robName, index);
        robIn[i].result = -1;
        robIn[i].dest = -1;
        robIn[i].completed = true;
        robIn[i].e = false;
        robIn[i].index = -1;
        strcpy(robIn[i].bufferName,robName);
    }
    return robIn;
}

void printROB(ROB *rob){
    printf("\n");
    printf("------------ Reorder Buffer----------\n");

    for(int r= 0; r<ROB_COUNT;r++){
        int exception = rob[r].e == true ? 1 : 0;
        int completed = rob[r].completed == true ? 1 : 0;
        printf("| %s [dest: %d, result: %d, (e: %d, completed: %d)]", rob[r].bufferName,rob[r].dest,rob[r].result,exception,completed);

        if(r == headIndex && r==tailIndex){
            printf(" <- head & tail\n");
        }
        else{
            if(r == headIndex){
                printf(" <- head\n");
            }
            if(r == tailIndex){
                printf(" <- tail\n");
            }
        }
        printf("\n");

    }
    printf("\n");

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
// void printReserveStation(){
//     printf("\n");
//     printf("============ BTB =================================\n\n");

//     for (int b=0; b<BTB_COUNT; b++) {
//         printf("|	 BTB[%2d]	|	Tag=%2d   |   Target=%2d   |\n", b, btb[b].tag, btb[b].target);   
//     }
//     printf("\n");

//     printf("============ Prediction Table  ==================\n\n");

//     for (int b=0; b<BTB_COUNT; b++) {
//         printf("|	 PT[ %2d]	|  Pattern=%2d   |\n", b, btb[b].pattern);   
        
//     }
//     printf("\n");
    
// }
void printLogs(int cycle,ROB *rob,CPU* cpu){

    if(enableLogs){
        printf("====================================================================\n");
        printf("Clock Cycle #: %d\n",cycle);
        printf("| RE   : %s             ",logger[RE1].logInfo);
        printf("| RE   : %s             \n",logger[RE2].logInfo);
        printf("---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n");
        printf("| WB   : %s             ",logger[WB1].logInfo);
        printf("| WB   : %s             ",logger[WB2].logInfo);
        printf("| WB   : %s             ",logger[WB3].logInfo);
        printf("| WB   : %s             \n",logger[WB4].logInfo);
        printf("| Add  : %s             ",logger[ADD].logInfo);
        printf("| Mul2 : %s             ",logger[MUL2].logInfo);
        printf("| DIV3 : %s             ",logger[DIV3].logInfo);
        printf("| MEM4 : %s            |\n",logger[MM4].logInfo);

        printf("|                       ");   
        printf("| Mul1 : %s             ",logger[MUL1].logInfo);
        printf("| DIV2 : %s             ",logger[DIV2].logInfo);
        printf("| MEM3 : %s            |\n",logger[MM3].logInfo);
        
        printf("|                       ");
        printf("|                       ");
        printf("| DIV1 : %s             ",logger[DIV1].logInfo);
        printf("| MEM2 : %s            |\n",logger[MM2].logInfo);

        printf("|                       ");
        printf("|                       ");
        printf("|                       ");
        printf("| MEM1 : %s            |\n",logger[MM1].logInfo);

        printf("---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n");
        printf("| IS     %s  %s\n",logger[IS].logInfo,strlen(logger[IS].logInfo) >0?"|":"");
        printf("| IR   : %s\n",logger[RR].logInfo);
        if(!logHide){
            printf("| IA   : %s\n",logger[IA].logInfo);
            printf("| ID   : %s\n",logger[ID].logInfo);
            printf("| IF   : %s\n\n",logger[IF].logInfo);
            
        }
        else{
            logHide = false;
        }
        printf("---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n");
        printf("------------ Reserve Station ----------\n");
        printf("%s%s\n",strlen(logger[RS0].logInfo) >0?"|":"",logger[RS0].logInfo);
        if(strlen(logger[RS1].logInfo) >0)
            printf("%s%s\n",strlen(logger[RS1].logInfo) >0?"|":"",logger[RS1].logInfo);
        if(strlen(logger[RS2].logInfo) >0)
            printf("%s%s\n",strlen(logger[RS2].logInfo) >0?"|":"",logger[RS2].logInfo);
        if(strlen(logger[RS3].logInfo) >0)
            printf("%s%s\n",strlen(logger[RS3].logInfo) >0?"|":"",logger[RS3].logInfo);

        printROB(rob);
        printf("------------ STATE OF ARCHITECTURAL REGISTER FILE ----------\n");
        printf("R# [(status 0=invalid, 1=valid), tag, value]\n"); 

        for (int i=0; i<16; i++){
            
            printf("| R%d [(%d) %d, %d] ",i,cpu[0].regs[i].is_writing,cpu[0].regs[i].tag,cpu[0].regs[i].value);

            if(((i+1)%4) == 0){
                printf("|\n");
            }
        }
       printf("--------------------------------------------------------------------------------\n\n\n");
    }

    for(int i=IF;i<=RE2;i++){
        myLogger *log = &logger[i];
        log->index = -1;
        log->type = -1;
        if(strlen(log->logInfo) >0){
            strcpy(log->logInfo,"");
        }
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

void processPipeline(char *instrFromFile[300],CPU *cpu){
    
    cpu->cycle = 1;
    int indexOfStalledInstr = -1;
    BTB *btb = BTB_init();
    Stage *pipelineInstructions = inorder_stage_init();
    log_init();
    ROB *rob = ROB_init();
    //reserveStation_init();
    bool toStop =false;
    while(true){

        
        // printf("================================\n");
        // printf("Clock Cycle #: %d\n",cpu->cycle);
        // printf("--------------------------------\n");
        

        
        if(cpu->cycle == 900){
            printf("%d\n",cpu->cycle);
        }
        resetOldInstruction(COMP,pipelineInstructions);
        toStop = retire1(pipelineInstructions,rob,cpu);
        retire2(pipelineInstructions,rob,cpu);
        writeBack(pipelineInstructions,rob);
        mem4(pipelineInstructions);        
        divider3(pipelineInstructions);
        multiplier2(pipelineInstructions);
        adder(pipelineInstructions);
        
        mem3(pipelineInstructions);
        divider2(pipelineInstructions);
        multiplier1(pipelineInstructions);

        mem2(pipelineInstructions);
        divider1(pipelineInstructions);

        mem1(pipelineInstructions);
                
        issue(pipelineInstructions,rob);
        registerRead(pipelineInstructions,cpu,rob,btb);
        instructionAnalyse(pipelineInstructions,rob);
        decode(pipelineInstructions);
        fetch(instrFromFile, pipelineInstructions,btb);

        indexOfStalledInstr = getStalledInstructionIndex(pipelineInstructions);

        if(indexOfStalledInstr>-1 ){

            if(calculateStructuralHazard){ cpu->structuralHazard++; }
            if(calculateDataHazard){cpu->dataHazard++;}
            //resetStalledInstructions(indexOfStalledInstr,pipelineInstructions);
            
        }

        if(pipelineInstructions[COMP].index>-1){
            cpu->totalInstructions++;
        }

        //print_display(cpu);
        printLogs(cpu->cycle,rob,cpu);
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
    
//    if (argc<=1) {
//         fprintf(stderr, "Error : missing required args\n");
//         return -1;
//     }

    char* filename = "program1.txt";
    
    

    
    run_cpu_fun(filename);
    
    return 0;
}









