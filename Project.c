#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>



Instruction pipelinedInstructions[4]; //4 instructions running in parallel
int left=0;
int right=-1;


/////////////////////Added By Youssef Maybe Redundant^^^^^^///////////////////



// Implement the memory: 2048 * 32-bit word-addressable memory 
// Create register structure (R0-R31, PC)
// Initialize memory regions (Instructions: 0 - 1023, Data: 1024-2047)


#define MemorySize 2048
#define InstructionLimit 1024 // 0  to 1023
#define DataStart 1024
#define RegisterNo 32 

uint32_t memory[MemorySize]; // Unsigned 32-bit integer
uint32_t registers[RegisterNo];
uint32_t PC = 0;

Instruction parsed_instructions [InstructionLimit]; //array of instructions with limit: 1024

int current_cycle = 1;


//Instruction Types 
typedef enum {R,I,J} InstrType;

typedef struct {
    int opcode, r1, r2, r3, shamt, address;
    int32_t imm; //signed immediate values 
    char raw[64]; 
    InstrType type;
    int result;
    int WBflag;
    int MEMflag;
    int cycle; 
} Instruction; 

typedef enum {F,D,E,M,W} cycle;


InstrType get_instr_type (int opcode) {
    if (opcode == 0 || opcode == 1 || opcode == 2 || opcode == 5 || opcode == 8 || opcode == 9) 
        return R;
    else if (opcode == 7)
        return J;
    return I;    
}

void fetch(){
    right=(right+1)%4; //first do add then fetch put into pipleined instructions array index right
    pipelinedInstructions[right] = parsed_instructions[PC];
    PC++;
}


//Functions for Instruction Parsing
void parse_instruction(char *line, int index) {
    Instruction inst;
    char instruction[10]; // a char array to store the instruction name
    sscanf(line, "%s", instruction);

    inst.opcode = get_opcode(instruction);
    strcpy(inst.raw, line); // used for printing later

    switch (inst.opcode) {
        case 0:
        case 1:
        case 2:
        case 5:
            sscanf(line, "%*s R%d R%d R%d", &inst.r1, &inst.r2, &inst.r3);
            break;
        case 8:
        case 9:
            sscanf(line, "%*s R%d R%d %d", &inst.r1, &inst.r2, &inst.shamt);
            break;
        case 3:
            sscanf(line, "%*s R%d %d", &inst.r1, &inst.imm);
            break;
        case 4:
        case 6:
        case 10:
        case 11:
            sscanf(line, "%*s R%d R%d %d", &inst.r1, &inst.r2, &inst.imm);
            break;
        case 7:
            sscanf(line, "%*s %d", &inst.address);
            break;
    }

    parsed_instructions[index] = inst;
    memory[index] = inst.opcode; // Simulated simple load (actual encoding would differ)
    PC++; 
}


// Execute instructions
void execute_instruction(Instruction inst, int cycle) {
    printf("\n[Cycle %d] Executing: %s", cycle, inst.raw);

    switch (inst.opcode) {
        case 0:
            registers[inst.r1] = registers[inst.r2] + registers[inst.r3];
            // result 
            break;
        case 1:
            registers[inst.r1] = registers[inst.r2] - registers[inst.r3];
            break;
        case 2:
            registers[inst.r1] = registers[inst.r2] * registers[inst.r3];
            break;
        case 3:
            registers[inst.r1] = inst.imm; 
            break;
        case 4:
            if (registers[inst.r1] == registers[inst.r2]) {
                PC = PC + 1 + inst.imm;
                return;
            }
            break;
        case 5:
            registers[inst.r1] = registers[inst.r2] & registers[inst.r3];
            break;
        case 6:
            registers[inst.r1] = registers[inst.r2] ^ inst.imm;
            break;
        case 7:
            PC = (PC & 0xF0000000) | inst.address; //PC & 0xF0000000: it masks the 1st 4-bits (31-28) of the current PC  
            return;
        case 8:
            registers[inst.r1] = registers[inst.r2] << inst.r3;
            break;
        case 9:
            registers[inst.r1] = registers[inst.r2] >> inst.r3;
            break;
        case 10:
            registers[inst.r1] = memory[registers[inst.r2] + inst.imm];
            break;
        case 11:
            memory[registers[inst.r2] + inst.imm] = registers[inst.r1];
            break;
    }
}

// Load instructions from text file
void load_instructions(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening instruction file");
        exit(EXIT_FAILURE);
    }

    char line[64];
    int index = 0;
    while (fgets(line, sizeof(line), file)) {
        parse_instruction(line, index); 
        index++;
    }
    fclose(file);
}

char* encode_opcode(const char* ins){
    if (strcmp(ins, "ADD") == 0) return "0000"; // WB = 0,1 MEMflag = 0,1
    if (strcmp(ins, "SUB") == 0) return "0001";
    if (strcmp(ins, "MUL") == 0) return "0010";
    if (strcmp(ins, "MOVI") == 0) return "0011";
    if (strcmp(ins, "JEQ") == 0) return "0100";
    if (strcmp(ins, "AND") == 0) return "0101";
    if (strcmp(ins, "XORI") == 0) return "0110";
    if (strcmp(ins, "JMP") == 0) return "0111";
    if (strcmp(ins, "LSL") == 0) return "1000";
    if (strcmp(ins, "LSR") == 0) return "1001";
    if (strcmp(ins, "MOVR") == 0) return "1010";
    if (strcmp(ins, "MOVM") == 0) return "1011";
    return "error"; 
}



//Initialization Functions
    void initialize_memory() {
        for(int i = 0; i < MemorySize; i++){
            memory[i] = 0;
        }
    }

    void initialize_registers(){
        for (int i = 0; i < RegisterNo; i++) {
            registers[i] = 0;
        }
        PC = 0;
    }

//Printing Functions
    void print_registers(){
        for(int i = 0; i < RegisterNo; i++){
            printf("R%-2d: %08X\n", i, registers[i]); //prints the value of each register in hexadecimal digits
        }
        printf("PC: %08X\n", PC); //current value of PC
    }

    void print_memory(int from, int to){
        for(int i = from; i <= to && i < MemorySize; i++){
            printf("MEM[%4d]: %08X\n", i, memory[i]);
        }

    }

//Data Structure

//Pipeline logic

//Main loop (pipeline)

//Instruction Execution Logic

//Execution Functions

//WB stage

//MEM stage

//Final State (for both registers & memory)

//Testing
    void test_initialization(){
        printf("\n>>> Running Milestone 1 Test\n");
        print_registers();
        print_memory(0, 10);                      // Instructions region
        print_memory(DataStart, DataStart + 6); // Start of data segment
    }


    void preidk(){
        initialize_registers();
        initialize_memory();
        test_initialization();
        //When to stop? 
    }
    
    void idk(){
        for(int i=left;i<=right;i++){
            switch(pipelinedInstructions[i].cycle){
                case 1: //Decode
                case 2:break;
                case 3: //execute Note Add This line In jump execute {right=i}
                case 4:break;
                case 5: //memory
                case 6: //WriteLine
            }
            pipelinedInstructions[i].cycle++;
        }
        if(pipelinedInstructions[left].cycle==7)left=(left+1)%4;
        if(current_cycle%2==1 &&!isFull()){
            fetch();
        }
        current_cycle++;
    }
    int isFull(){
        if(left<right&&right-left==3 || right<left &&right+1==left)
            return 1;
        return 0;
    }

//Main method
    int main(){
        preidk();
        int whenToStop=1; //define when to stop
        while(whenToStop){
        idk();
        }
        return 0;
    }


