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

// Forward declarations of types
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

// Function declarations
void initialize_memory(void);
void initialize_registerFile(void);
void load_program(const char *filename);
char* encode_instruction(char* plainInstruction);
char* encode_opcode(char* plainOpcode);
char* encode_register(char* plainRegister);
char* encode_remaining(char* plain);
InstrType get_instr_type(int opcode);
void fetch(void);
void decode(void);
void execute(void);
void memory(void);
void write_back(void);
void pipeline(void);
void run(void);

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



char* encode_instruction(char* plainInstruction){
    
}
char* encode_opcode(char* plainOpcode){

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
    if (pc >= INSTRUCTION_END) {
        printf("\n[Cycle %d] Program completed - PC reached end of instruction memory", programCycle);
        return;
    }
    
    // Create new instruction and add it to pipeline
    Instruction newInst;
    // Copy the instruction from memory to the new instruction struct
    sprintf(newInst.encodedInstruction, "%08X", mainMemory[pc]);
    newInst.instructionCycle = 1;
    newInst.operationResult = 0;
    newInst.WBflag = 0;
    newInst.MEMflag = 0;
    
    // Add to pipeline
    right = (right + 1) % MAX_PIPELINE_DEPTH;
    pipelinedInstructions[right] = newInst;
    
    // Increment PC
    pc++;
    
    printf("\n[Cycle %d] Fetch: Instruction at PC=%d", programCycle, pc-1);
}
void decode(){
    if (left > right) return;  // No instructions to decode
    
    Instruction* inst = &pipelinedInstructions[left];
    if (inst->instructionCycle != 2) return;  // Not in decode stage
    
    // Convert instruction string to integer
    uint32_t instruction;
    sscanf(inst->encodedInstruction, "%x", &instruction);
    
    // Extract opcode (bits 31-28)
    inst->opcode = (instruction >> 28) & 0xF;
    
    // Determine instruction type
    inst->type = get_instr_type(inst->opcode);
    
    // Decode based on instruction type
    switch(inst->type) {
        case R:
            inst->r1 = (instruction >> 23) & 0x1F;  // bits 27-23
            inst->r2 = (instruction >> 18) & 0x1F;  // bits 22-18
            inst->r3 = (instruction >> 13) & 0x1F;  // bits 17-13
            inst->shamt = instruction & 0x1FFF;     // bits 12-0
            break;
            
        case I:
            inst->r1 = (instruction >> 23) & 0x1F;  // bits 27-23
            inst->r2 = (instruction >> 18) & 0x1F;  // bits 22-18
            inst->imm = instruction & 0x3FFFF;      // bits 17-0
            // Sign extend immediate if needed
            if (inst->imm & 0x20000) {  // If sign bit is 1
                inst->imm |= 0xFFFC0000;  // Extend with 1's
            }
            break;
            
        case J:
            inst->address = instruction & 0x0FFFFFFF;  // bits 27-0
            break;
    }
    
    printf("\n[Cycle %d] Decode: opcode=%d, type=%d", programCycle, inst->opcode, inst->type);
    
    // For R-type and I-type instructions, read operands from register file
    if (inst->type != J) {
        printf(", r1=R%d(val=%d)", inst->r1, registerFile[inst->r1]);
        printf(", r2=R%d(val=%d)", inst->r2, registerFile[inst->r2]);
        if (inst->type == R) {
            printf(", r3=R%d(val=%d)", inst->r3, registerFile[inst->r3]);
        }
    }
}
void execute(){
    if (left > right) return;  // No instructions to execute
    
    Instruction* inst = &pipelinedInstructions[left];
    if (inst->instructionCycle != 3) return;  // Not in execute stage

    printf("\n[Cycle %d] Executing: opcode=%d", programCycle, inst->opcode);

    switch (inst->opcode) {
        case 0:  // ADD
            inst->operationResult = registerFile[inst->r2] + registerFile[inst->r3];
            inst->WBflag = 1;
            break;
        case 1:  // SUB
            inst->operationResult = registerFile[inst->r2] - registerFile[inst->r3];
            inst->WBflag = 1;
            break;
        case 2:  // MUL
            inst->operationResult = registerFile[inst->r2] * registerFile[inst->r3];
            inst->WBflag = 1;
            break;
        case 3:  // MOVI
            inst->operationResult = inst->imm;
            inst->WBflag = 1;
            break;
        case 4:  // BEQ
            if (registerFile[inst->r1] == registerFile[inst->r2]) {
                pc = pc + 1 + inst->imm;
                right = left;  // Clear pipeline after branch
            }
            break;
        case 5:  // AND
            inst->operationResult = registerFile[inst->r2] & registerFile[inst->r3];
            inst->WBflag = 1;
            break;
        case 6:  // XORI
            inst->operationResult = registerFile[inst->r2] ^ inst->imm;
            inst->WBflag = 1;
            break;
        case 7:  // JMP
            pc = (pc & 0xF0000000) | inst->address;
            right = left;  // Clear pipeline after jump
            break;
        case 8:  // LSL
            inst->operationResult = registerFile[inst->r2] << inst->r3;
            inst->WBflag = 1;
            break;
        case 9:  // LSR
            inst->operationResult = registerFile[inst->r2] >> inst->r3;
            inst->WBflag = 1;
            break;
        case 10:  // LOAD
            inst->operationResult = registerFile[inst->r2] + inst->imm;
            inst->MEMflag = 1;
            inst->WBflag = 1;
            break;
        case 11:  // STORE
            inst->operationResult = registerFile[inst->r2] + inst->imm;
            inst->MEMflag = 1;
            break;
    }
}
void memory(){
    if (left > right) return;  // No instructions to process
    
    Instruction* inst = &pipelinedInstructions[left];
    if (inst->instructionCycle != 4) return;  // Not in memory stage
    
    // Only process if instruction needs memory access
    if (inst->MEMflag) {
        printf("\n[Cycle %d] Memory Access: opcode=%d", programCycle, inst->opcode);
        
        switch(inst->opcode) {
            case 10:  // LOAD
                // Load from memory
                inst->operationResult = mainMemory[inst->operationResult];
                printf(", Loading from address %d", inst->operationResult);
                break;
                
            case 11:  // STORE
                // Store to memory
                mainMemory[inst->operationResult] = registerFile[inst->r1];
                printf(", Storing to address %d", inst->operationResult);
                break;
        }
    }
}
void write_back(){
    if (left > right) return;  // No instructions to process
    
    Instruction* inst = &pipelinedInstructions[left];
    if (inst->instructionCycle != 5) return;  // Not in writeback stage
    
    // Only process if instruction needs writeback
    if (inst->WBflag) {
        printf("\n[Cycle %d] Write Back: opcode=%d", programCycle, inst->opcode);
        registerFile[inst->r1] = inst->operationResult;
        printf(", Writing %d to R%d", inst->operationResult, inst->r1);
    }
}
//=====================Pipeline Logic=======================
int isFull(){
    if(left<right&&right-left==3 || right<left &&right+1==left)
        return 1;
    return 0;
}

void pipeline(){
    // Process instructions in pipeline from oldest to newest
    if (left <= right) {
        // Process write back
        if (pipelinedInstructions[left].instructionCycle == 5) {
            write_back();
        }
        // Process memory
        if (pipelinedInstructions[left].instructionCycle == 4) {
            memory();
        }
        // Process execute
        if (pipelinedInstructions[left].instructionCycle == 3) {
            execute();
        }
        // Process decode
        if (pipelinedInstructions[left].instructionCycle == 2) {
            decode();
        }
        
        // Increment cycle counter for instruction in pipeline
        for(int i = left; i <= right; i++) {
            pipelinedInstructions[i].instructionCycle++;
        }
        
        // Remove completed instructions
        if (pipelinedInstructions[left].instructionCycle > 6) {
            left = (left + 1) % MAX_PIPELINE_DEPTH;
        }
    }
    
    // Fetch new instruction if pipeline not full
    if (!isFull() && programCycle % 2 == 1) {
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