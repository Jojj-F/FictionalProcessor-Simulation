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
//=====================Registers==================
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
    int instructionCycle; 
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
    initialize_registers();
    load_program("Temporary"); //TODO change<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}
void initialize_memory() {
    for(int i = 0; i < MEMORY_SIZE; i++)
        mainMemory[i] = 0;
}
void initialize_registers(){
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
        mainMemory[index]=encode_instruction("Temporary"); //TODO change<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        index++;
    }
    fclose(file);
}
char* encode_instruction(char* plainInstruction){

}
char* encode_opcode(char* plainOPCode){

}
char* encode_register(char* plainRegister){

}
char* encode_remaining(char* plain){

}
InstrType get_instr_type (int opcode) {
    if (opcode == 0 || opcode == 1 || opcode == 2 || opcode == 5 || opcode == 8 || opcode == 9) 
        return R;
    else if (opcode == 7)
        return J;
    return I;    
}
//=====================Program Logic========================
void fetch(){

}
void decode(){

}
void execute(){

}
void memory(){

}
void write_back(){

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
//     print_registers();
//     print_memory(0, 10);                      // Instructions region
//     print_memory(DataStart, DataStart + 6); // Start of data segment
// }
// void print_registers(){
//     for(int i = 0; i < RegisterNo; i++){
//         printf("R%-2d: %08X\n", i, registers[i]); //prints the value of each register in hexadecimal digits
//     }
//     printf("PC: %08X\n", PC); //current value of PC
// }

// void print_memory(int from, int to){
//     for(int i = from; i <= to && i < MemorySize; i++){
//         printf("MEM[%4d]: %08X\n", i, memory[i]);
//     }

// }