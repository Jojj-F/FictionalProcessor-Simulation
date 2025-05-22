#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

//==========================================Data==========================================
//===================Constants===================
#define MEMORY_SIZE 2048
#define INSTRUCTION_START 0
#define INSTRUCTION_END 1023 // 0  to 1023
#define DATA_START 1024
#define DATA_END 2047
#define REGISTER_NO 32 
#define MAX_PIPELINE_DEPTH  4
//=====================Memory=====================
char mainMemory[MEMORY_SIZE][33]; //TODO change to carry binary string <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//=====================registerFile==================
uint32_t registerFile[REGISTER_NO]; 
uint32_t pc = 0;
//=====================Pipeline===================
Instruction pipelinedInstructions[MAX_PIPELINE_DEPTH];
int left=0; 
int right=-1;
int programCycle=1;
//=====================Types===================
typedef enum {R,I,J} InstrType;
typedef enum {F,D,E,M,W} CyclePhase;
typedef struct {
    char encodedInstruction[33  ]; 
    int instructionCycle; // to be set = 1 
    int operationResult;
    //After Decoding the below values should be filled
    InstrType type;
    int32_t imm; //signed immediate values 
    int opcode, r1, r2, r3, shamt, address;
    int WBflag,MEMflag;
} Instruction; 
//==========================================Code==========================================
//=====================Program Initalizaiton===================
void initialize_program(){
    initialize_memory();
    initialize_registerFile();
    load_program("Temporary"); //TODO change<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}
void initialize_memory() {
    for(int i = 0; i < MEMORY_SIZE; i++)
       strcpy(mainMemory[i], "00000000000000000000000000000000");
}
void initialize_registerFile(){
    for (int i = 0; i < REGISTER_NO; i++) 
        registerFile[i] = 0;
    pc = 0;
}
void load_program(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening instruction file");
        exit(EXIT_FAILURE);
    }
    char line[64];
    int index = 0;
    while (fgets(line, sizeof(line), file)) {
        strcpy(mainMemory[index], encode_instruction(line));
        index++;
    }
    fclose(file);
}


InstrType get_instr_type (char* opcode) {
    if (opcode == "0000" || opcode == "0001" || opcode == "0010" || opcode == "0101" || opcode == "1000" || opcode == "1001") 
        return R;
    else if (opcode == "0111")
        return J;
    return I;    
}


char* encode_instruction(char* plainInstruction){//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    int size=strlen(plainInstruction);
    char opcode[5];char r1[]="00";char r2[]="00";char r3[]="00";
    opcode[4]='\0';
    int getOpCode=1;int wR=1;
    int i=0;
    for(;i<size;i++){
        if(plainInstruction[i]==' '){
            if(i==3&&getOpCode)opcode[i]=' ';
            getOpCode=0;
        }
        else if(getOpCode)
            opcode[i]=plainInstruction[i];  
        else if(plainInstruction[i]=='R'){
            i++;
            if(i+1<size && plainInstruction[i+1]!=' '){
                if(wR==1) r1[0]=plainInstruction[i++];
                else if(wR==2) r2[0]=plainInstruction[i++];
                else r3[0]=plainInstruction[i++];
            }

            if(wR==1) r1[1]=plainInstruction[i];
            else if(wR==2) r2[1]=plainInstruction[i];
            else r3[1]=plainInstruction[i];
            wR++;
        }
        else break;
    }
    int sizeRem=size-i;
    if(sizeRem==0)sizeRem++;
    char rem[sizeRem+1];rem[sizeRem]='\0';
    int ptr=0;
    while(i<size)rem[ptr++]=plainInstruction[i++];
    if(sizeRem==0)rem[0]='0';
    char* encodedOpCode=encode_opcode(opcode);
    char* encodedR1=convertIntToBinary(atoi(r1),5);
    char* encodedR2=convertIntToBinary(atoi(r2),5);
    char* encodedR3=convertIntToBinary(atoi(r3),5);
    InstrType type= get_instr_type(atoi(encodedOpCode));
    char* remaining;
    if(type==R)remaining=convertIntToBinary(atoi(rem),13);
    else if(type==I)remaining=convertIntToBinary(atoi(rem),18);
    else remaining=convertIntToBinary(atoi(rem),28);


    int len_op = strlen(encodedOpCode);
    int len_r1 = strlen(encodedR1);
    int len_r2 = strlen(encodedR2);
    int len_r3 = strlen(encodedR3);
    int len_rem = strlen(remaining);

    int total_len = len_op + len_r1 + len_r2 + len_r3 + len_rem;

    char* merged = malloc(total_len + 1);
    strcpy(merged, encodedOpCode);
    strcat(merged, encodedR1);
    strcat(merged, encodedR2);
    strcat(merged, encodedR3);
    strcat(merged, remaining);
    printf("Merged binary string: %s\n", merged);
    return merged;
}
char* encode_opcode(char* plainOpcode){  //passing only opcode 
    if (strcmp(plainOpcode, "ADD ") == 0) return "0000"; // WB = 0,1 MEMflag = 0,1
    if (strcmp(plainOpcode, "SUB ") == 0) return "0001";
    if (strcmp(plainOpcode, "MUL ") == 0) return "0010";
    if (strcmp(plainOpcode, "MOVI") == 0) return "0011";
    if (strcmp(plainOpcode, "JEQ ") == 0) return "0100";
    if (strcmp(plainOpcode, "AND ") == 0) return "0101";
    if (strcmp(plainOpcode, "XORI") == 0) return "0110";
    if (strcmp(plainOpcode, "JMP ") == 0) return "0111";
    if (strcmp(plainOpcode, "LSL ") == 0) return "1000";
    if (strcmp(plainOpcode, "LSR ") == 0) return "1001";
    if (strcmp(plainOpcode, "MOVR") == 0) return "1010";
    if (strcmp(plainOpcode, "MOVM") == 0) return "1011";
    return "error";
}char* convertIntToBinary(int n, int size) {
    char* str = malloc(size + 1); 
    for (int i = size - 1; i >= 0; i--) {
        str[i] = (n % 2) + '0';
        n /= 2;
    }
    str[size] = '\0';
    return str;
}
//=====================Program Logic========================

void decode(Instruction instr){
    char opcode[5];opcode[4]='\0';
    char er1[6];er1[5]='\0';
    char er2[6];er2[5]='\0';
    char er3[6];er3[5]='\0';

    strncpy(opcode, instr.encodedInstruction, 4);
    instr.type=get_instr_type(opcode);
    instr.opcode=atoi(opcode);
    if(instr.type=R){
        
        memcpy(er1, instr.encodedInstruction + 4, 5); 
        instr.r1=atoi(er1);
        memcpy(er2, instr.encodedInstruction + 9, 5); 
        instr.r2=atoi(er2);
        memcpy(er3, instr.encodedInstruction + 14, 5);
        instr.r3=atoi(er3); 
        char sha[14];sha[13]='\0';
        memcpy(sha, instr.encodedInstruction + 19, 13);
        instr.shamt=atoi(sha); 
 
        
    }else if(instr.type=I){
        memcpy(er1, instr.encodedInstruction + 4, 5); 
        instr.r1=atoi(er1);
        memcpy(er2, instr.encodedInstruction + 9, 5); 
        instr.r2=atoi(er2);
        char im[19];im[18]='\0';
        memcpy(im, instr.encodedInstruction + 14, 18);
        instr.imm=atoi(im); 
    }else{
        char ad[29];ad[28]='\0';
        memcpy(ad, instr.encodedInstruction + 19, 13);
        instr.address=atoi(ad); 
    }

}   
void execute(Instruction inst, int cycle,int i){
    printf("\n[Cycle %d] Executing: %s", cycle, inst.encodedInstruction);

    switch (inst.opcode) {
        case 0:
            inst.operationResult = registerFile[inst.r2] + registerFile[inst.r3];
             inst.WBflag = 1; 
            break;
        case 1:
            inst.operationResult = registerFile[inst.r2] - registerFile[inst.r3];
             inst.WBflag = 1;
            break;
        case 2:
            inst.operationResult = registerFile[inst.r2] * registerFile[inst.r3];
            inst.WBflag = 1;
            break;
        case 3:
            inst.operationResult = inst.imm; 
            inst.WBflag = 1;
            break;
        case 4:
            right=i;
            if (inst.operationResult == registerFile[inst.r2]) 
                pc = pc + 1 + inst.imm;
            
            break;
        case 5:
            inst.operationResult = registerFile[inst.r2] & registerFile[inst.r3];
            inst.WBflag = 1;
            break;
        case 6:
            inst.operationResult = registerFile[inst.r2] ^ inst.imm;
            inst.WBflag = 1;
            break;
        case 7:
            right=i;
            pc = (pc & 0xF0000000) | inst.address; //pc & 0xF0000000: it masks the 1st 4-bits (31-28) of the current pc  
            return;
        case 8:
            inst.operationResult = registerFile[inst.r2] << inst.shamt;
            inst.WBflag = 1;
            break;
        case 9:
            inst.operationResult = registerFile[inst.r2] >> inst.shamt;
            inst.WBflag = 1;
            break;
        case 10:  // LOAD
            inst.operationResult = registerFile[inst.r2] + inst.imm;
            inst.MEMflag = 1;
            inst.WBflag = 1;
            break;
        case 11:  // STORE
            inst.operationResult = registerFile[inst.r2] + inst.imm;
            inst.MEMflag = 1;
            break;
    }
}
void memory(Instruction inst){
     if (left > right) return;  // No instructions to process
    
    Instruction* inst = &pipelinedInstructions[left];
    if (inst.instructionCycle != 4) return;  // Not in memory stage
    
    // Only process if instruction needs memory access
    if (inst.MEMflag) {
        printf("\n[Cycle %d] Memory Access: opcode=%d", programCycle, inst.opcode);
        
        switch(inst.opcode) {
            case 10:  // LOAD
                // Load from memory
                inst.operationResult = mainMemory[inst.operationResult];
                printf(", Loading from address %d", inst.operationResult);
                break;
                
            case 11:  // STORE
                // Store to memory
                strcpy(mainMemory[inst.operationResult] , registerFile[inst.r1]);
                printf(", Storing to address %d", inst.operationResult);
                break;
        }
    }

}
void write_back(Instruction inst){
    if(left > right) return; //No pipeline, no instructions to process

    Instruction* inst = &pipelinedInstructions[left];
    if (inst.instructionCycle != 5) return;  // Not in writeback stage
    
    // Only process if instruction needs writeback
    if (inst.WBflag) {
        printf("\n[Cycle %d] Write Back: opcode=%d", programCycle, inst.opcode);
        inst.operationResult = inst.operationResult;
        printf(", Writing %d to R%d", inst.operationResult, inst.r1);
    }
}
fetch(){
    right=(right+1)%4; //first do add then fetch put into pipleined instructions array index right
    Instruction instr;
    strcpy(instr.encodedInstruction, mainMemory[pc]);
    pc++;
    pipelinedInstructions[right] =instr;
}
//=====================Pipeline Logic=======================
int isFull(){
    if(left<right&&right-left==3 || right<left &&right+1==left)
        return 1;
    return 0;
}

void pipeline(){
    for(int i=left;i<=right;i++){
            switch(pipelinedInstructions[i].instructionCycle){
                case 1: decode(pipelinedInstructions[i]);
                case 2:break;
                case 3: execute(pipelinedInstructions[i],pipelinedInstructions[i].instructionCycle,i);//execute Note Add This line In jump execute {right=i}
                case 4:break;
                case 5:memory(pipelinedInstructions[i]);break;//memory
                case 6:write_back(pipelinedInstructions[i]); //WriteBack
            }
            pipelinedInstructions[i].instructionCycle++;
        }
        if(pipelinedInstructions[left].instructionCycle==7)left=(left+1)%4;
        if(programCycle%2==1 &&!isFull()){
            fetch();
        }
        programCycle++;
}
void run(){
    int runProgram=1;//TODO change<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    while(runProgram)
        pipeline();
}
int main(){
    initialize_program();
    run();
    return 0;
}

//=====================Uncategorized=======================

// void test_initialization(){
//     printf("\n>>> Running Milestone 1 Test\n");
//     print_registerFile();
//     print_memory(0, 10);                      // Instructions region
//     print_memory(DataStart, DataStart + 6); // Start of data segment
// }
// void print_registerFile(){
//     for(int i = 0; i < RegisterNo; i++){
//         printf("R%-2d: %08X\n", i, registerFile[i]); //prints the value of each register in hexadecimal digits
//     }
//     printf("pc: %08X\n", pc); //current value of pc
// }

// void print_memory(int from, int to){
//     for(int i = from; i <= to && i < MemorySize; i++){
//         printf("MEM[%4d]: %08X\n", i, memory[i]);
//     }

// }