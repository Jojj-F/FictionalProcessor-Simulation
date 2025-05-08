#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdint.h>


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


//Instruction Types 

typedef enum {R,I,J} InstrType;

typedef struct {
    int opcode, r1, r2, r3, imm, shamt, address;
    InstrType type;
} Instruction; 

//Instruction Structure 


//Array to store the instructions

//Data Structure


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

//Functions for Instruction Parsing

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

//Main method
    int main(){
        initialize_registers();
        initialize_memory();
        test_initialization();
        return 0;
    }

