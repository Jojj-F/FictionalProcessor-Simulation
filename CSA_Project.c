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
uint32_t mainMemory[MEMORY_SIZE]; //TODO change to carry binary string <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
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
    char encodedInstruction[64]; 
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
        mainMemory[i] = 0;
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
        mainMemory[index]=encode_instruction(line); //TODO change<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        index++;
    }
    fclose(file);
}


InstrType get_instr_type (int opcode) {
    if (opcode == "0000" || opcode == "0001" || opcode == "0010" || opcode == "0101" || opcode == "1000" || opcode == "1001") 
        return R;
    else if (opcode == "0111")
        return J;
    return I;    
}


char* encode_instruction(char* plainInstruction){
    char*s1;
    char*s2;
    char*s3;
    char*s4;
    char*s4;

    char* inst = encode_opcode(char* s1); //opcode of instruction

    InstrType t = get_instr_type(inst);
        
    
}
char* encode_opcode(char* plainOpcode){  //passing only opcode 
    if (strcmp(plainOpcode, "ADD") == 0) return "0000"; // WB = 0,1 MEMflag = 0,1
    if (strcmp(plainOpcode, "SUB") == 0) return "0001";
    if (strcmp(plainOpcode, "MUL") == 0) return "0010";
    if (strcmp(plainOpcode, "MOVI") == 0) return "0011";
    if (strcmp(plainOpcode, "JEQ") == 0) return "0100";
    if (strcmp(plainOpcode, "AND") == 0) return "0101";
    if (strcmp(plainOpcode, "XORI") == 0) return "0110";
    if (strcmp(plainOpcode, "JMP") == 0) return "0111";
    if (strcmp(plainOpcode, "LSL") == 0) return "1000";
    if (strcmp(plainOpcode, "LSR") == 0) return "1001";
    if (strcmp(plainOpcode, "MOVR") == 0) return "1010";
    if (strcmp(plainOpcode, "MOVM") == 0) return "1011";
    return "error";
}
char* encode_register(char* plainRegister){
    if (strcmp(plainRegister, "R0") == 0) return "00000"; 
    if (strcmp(plainRegister, "R1") == 0) return "00001";
    if (strcmp(plainRegister, "R2") == 0) return "00010";
    if (strcmp(plainRegister, "R3") == 0) return "00011";
    if (strcmp(plainRegister, "R4") == 0) return "00100";
    if (strcmp(plainRegister, "R5") == 0) return "00101";
    if (strcmp(plainRegister, "R6") == 0) return "00110";
    if (strcmp(plainRegister, "R7") == 0) return "00111";
    if (strcmp(plainRegister, "R8") == 0) return "01000";
    if (strcmp(plainRegister, "R9") == 0) return "01001";
    if (strcmp(plainRegister, "R10") == 0) return "01010";
    if (strcmp(plainRegister, "R11") == 0) return "01011";
    if (strcmp(plainRegister, "R12") == 0) return "01100";
    if (strcmp(plainRegister, "R13") == 0) return "01101";
    if (strcmp(plainRegister, "R14") == 0) return "01110";
    if (strcmp(plainRegister, "R15") == 0) return "01111";
    if (strcmp(plainRegister, "R16") == 0) return "10000";
    if (strcmp(plainRegister, "R17") == 0) return "10001";
    if (strcmp(plainRegister, "R18") == 0) return "10010";
    if (strcmp(plainRegister, "R19") == 0) return "10011";
    if (strcmp(plainRegister, "R20") == 0) return "10100";
    if (strcmp(plainRegister, "R21") == 0) return "10101";
    if (strcmp(plainRegister, "R22") == 0) return "10110";
    if (strcmp(plainRegister, "R23") == 0) return "10111";
    if (strcmp(plainRegister, "R24") == 0) return "11000";
    if (strcmp(plainRegister, "R25") == 0) return "11001";
    if (strcmp(plainRegister, "R26") == 0) return "11010";
    if (strcmp(plainRegister, "R27") == 0) return "11011";
    if (strcmp(plainRegister, "R28") == 0) return "11100";
    if (strcmp(plainRegister, "R29") == 0) return "11101";
    if (strcmp(plainRegister, "R30") == 0) return "11110";
    if (strcmp(plainRegister, "R31") == 0) return "11111";
    return "error";
    
}
char* encode_remaining(char* plain, InstrType t){
    
}


char* convertIntToBinary(int n, int size) {
    char* str = malloc(size + 1); 
    for (int i = size - 1; i >= 0; i--) {
        str[i] = (n % 2) + '0';
        n /= 2;
    }
    str[size] = '\0';
    return str;
}
//=====================Program Logic========================

void decode(){

}
void execute(Instruction inst, int cycle){
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
            if (inst.operationResult == registerFile[inst.r2]) {
                pc = pc + 1 + inst.imm;
                right = left;
            }
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
            pc = (pc & 0xF0000000) | inst.address; //pc & 0xF0000000: it masks the 1st 4-bits (31-28) of the current pc  
            right = left; 
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
                mainMemory[inst.operationResult] = registerFile[inst.r1];
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
//=====================Pipeline Logic=======================
int isFull(){
    if(left<right&&right-left==3 || right<left &&right+1==left)
        return 1;
    return 0;
}

void pipeline(){
    for(int i=left;i<=right;i++){
            switch(pipelinedInstructions[i].instructionCycle){
                case 1: decode();
                case 2:break;
                case 3: execute();//execute Note Add This line In jump execute {right=i}
                case 4:break;
                case 5:memory();break;//memory
                case 6:write_back(); //WriteBack
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