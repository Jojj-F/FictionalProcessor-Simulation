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
char mainMemory[MEMORY_SIZE][33];
int maxInstructionIndex;
//=====================registerFile==================
uint32_t registerFile[REGISTER_NO]; 
uint32_t pc;

//=====================Types===================
typedef enum {R,I,J} InstrType;
typedef enum {F,D,E,M,W} CyclePhase;
typedef struct {
    char encodedInstruction[33]; 
    int instructionCycle; // to be set = 1 
    int operationResult;
    //After Decoding the below values should be filled
    InstrType type;
    int32_t imm; //signed immediate values 
    int opcode, r1, r2, r3, shamt, address;
    int WBflag,MEMflag;
} Instruction;

//=====================Pipeline===================
Instruction pipelinedInstructions[MAX_PIPELINE_DEPTH];
int left=0; 
int right=-1;
int totalPipelined=0;
int programCycle=1; 
//==========================================Code==========================================
//=====================Printing===============================

void print_registers() {
    printf("Register values :\n");
    for (int i = 1; i <= 6; i++) {
        printf("R%d = %d (0x%X)\n", i, registerFile[i], registerFile[i]);
    }
    printf("Final PC = %u\n", pc);
}

void print_memory() {
    printf("Memory content:\n");
    for (int i = 0; i < MEMORY_SIZE; i++)
        printf("MEM[%d] = %s\n", i, mainMemory[i]);
}


//=====================Helper Functions========================
InstrType get_instr_type (char* opcode) {
    if (strcmp(opcode, "0000") == 0|| strcmp(opcode, "0001") == 0 ||strcmp(opcode, "0010") == 0 || strcmp(opcode, "0101") == 0 || strcmp(opcode, "1000") == 0||strcmp(opcode, "1001") == 0) 
        return R;
    else if (strcmp(opcode, "0111") == 0)
        return J;
    return I;    
}

char* twosComplement(char* str,int size){
    for(int i=size-1;i>=0;i--)
            str[i] = (str[i] == '0') ? '1' : '0';
        
    int carry = 1;
    for (int i = size - 1; i >= 0 && carry; i--) {
        if (str[i] == '0') {
            str[i] = '1';
            carry = 0;
        } else  str[i] = '0';
    }
    return str;
}

char* convertIntToBinary(int n, int size) {
    char* str = malloc(size + 1); 
    int isNegative=0;
    if(n<0){
        isNegative=1;
        n*=-1;
    }
    for (int i = size - 1; i >= 0; i--) {
        str[i] = (n % 2) + '0';
        n /= 2;
    }
    str[size] = '\0';
    if(!isNegative)return str;    
    return twosComplement(str,size);
}

//=====================Program Initalizaiton===================
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
    InstrType type= get_instr_type(encodedOpCode);
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
    return merged;
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
    maxInstructionIndex=index;
    fclose(file);
}

void initialize_program(){
    initialize_memory();
    initialize_registerFile();
    load_program("assembly.txt"); 
}
//=====================Program Logic========================
void fetch(){
    right=(right+1)%4; //first do add then fetch put into pipleined instructions array index right
    Instruction instr;
    strcpy(instr.encodedInstruction, mainMemory[pc]);
    instr.instructionCycle=1;
    pc++;
        printf("Fetching  %d %d \n",right,instr.instructionCycle);

    pipelinedInstructions[right] =instr;
}

void decode(Instruction* instr){
    char opcode[5];
    char er1[6];
    char er2[6];
    char er3[6];

    strncpy(opcode, instr->encodedInstruction, 4);
    opcode[4]='\0';
    instr->type = get_instr_type(opcode);
    instr->opcode = strtol(opcode, NULL, 2);
    if(instr->type == R){
        
        memcpy(er1, instr->encodedInstruction + 4, 5); 
        er1[5] = '\0';
        instr->r1 = strtol(er1, NULL, 2);
        memcpy(er2, instr->encodedInstruction + 9, 5); 
        er2[5] = '\0';
        instr->r2 = strtol(er2, NULL, 2);
        memcpy(er3, instr->encodedInstruction + 14, 5);
        er3[5] = '\0';
        instr->r3 = strtol(er3, NULL, 2);
        char sha[14];
        memcpy(sha, instr->encodedInstruction + 19, 13);
        sha[13] = '\0';
        instr->shamt = strtol(sha, NULL, 2); 
        
    }else if(instr->type == I){
        memcpy(er1, instr->encodedInstruction + 4, 5); 
        er1[5] = '\0';
        instr->r1 = strtol(er1, NULL, 2);
        memcpy(er2, instr->encodedInstruction + 9, 5); 
        er2[5] = '\0';
        instr->r2 = strtol(er2, NULL, 2);
        char im[19];
        memcpy(im, instr->encodedInstruction + 14, 18);
        im[18] = '\0';
        if(im[0] == '1') instr->imm =strtol(twosComplement(im,18), NULL, 2) * -1;
        else instr->imm = strtol(im, NULL, 2);
    }else{
        char ad[29];
        memcpy(ad, instr->encodedInstruction + 4, 28);
        ad[28] = '\0';
        instr->address = strtol(ad, NULL, 2);
    }
    printf("[Cycle %d] Stage: Decode | Instruction: %s | R1=%d R2=%d R3=%d IMM=%d SHAMT=%d ADDR=%d\n", programCycle, instr->encodedInstruction, instr->r1, instr->r2, instr->r3, instr->imm, instr->shamt, instr->address); // instruction output per cycle 

}

void execute(Instruction* inst, int cycle,int i){
    printf("\n[Cycle %d] Executing: %s", cycle, inst->encodedInstruction);

    switch (inst->opcode) {
        case 0:
            inst->operationResult = registerFile[inst->r2] + registerFile[inst->r3];
            inst->WBflag = 1; 
            break;
        case 1:
            inst->operationResult = registerFile[inst->r2] - registerFile[inst->r3];
            inst->WBflag = 1;
            break;
        case 2:
            inst->operationResult = registerFile[inst->r2] * registerFile[inst->r3];
            inst->WBflag = 1;
            break;
        case 3:
            inst->operationResult = inst->imm; 
            inst->WBflag = 1;
            break;
        case 4:
            right = i;
            if (registerFile[inst->r1] == registerFile[inst->r2]) 
                pc = pc + 1 + inst->imm;
            int inst_count =  inst_count - ((right-i)+MAX_PIPELINE_DEPTH)%MAX_PIPELINE_DEPTH;
            // Flush the next 2 instructions in the pipeline 
            // pipelinedInstructions[(i + 1) % MAX_PIPELINE_DEPTH].instructionCycle = 7;
            // pipelinedInstructions[(i + 2) % MAX_PIPELINE_DEPTH].instructionCycle = 7;

            // printf(" [Branch Taken] Flushed instructions at positions %d and %d\n", (i + 1) % MAX_PIPELINE_DEPTH, (i + 2) % MAX_PIPELINE_DEPTH);
            break;
        case 5:
            inst->operationResult = registerFile[inst->r2] & registerFile[inst->r3];
            inst->WBflag = 1;
            break;
        case 6:
            inst->operationResult = registerFile[inst->r2] ^ inst->imm;
            inst->WBflag = 1;
            break;
        case 7:
            right = i;
            pc = (pc & 0xF0000000) | inst->address; // pc & 0xF0000000: masks the 1st 4-bits (31-28) of current pc  
            int inst_count =  inst_count - ((right-i)+MAX_PIPELINE_DEPTH)%MAX_PIPELINE_DEPTH;
            // Flush the next two instructions
            // pipelinedInstructions[(i + 1) % MAX_PIPELINE_DEPTH].instructionCycle = 7;
            // pipelinedInstructions[(i + 2) % MAX_PIPELINE_DEPTH].instructionCycle = 7;

            // printf(" [JUMP] Flushed instructions at positions %d and %d\n", (i + 1) % MAX_PIPELINE_DEPTH, (i + 2) % MAX_PIPELINE_DEPTH);
            return;
        case 8:
            inst->operationResult = registerFile[inst->r2] << inst->shamt;
            inst->WBflag = 1;
            break;
        case 9:
            inst->operationResult = registerFile[inst->r2] >> inst->shamt;
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

void memory(Instruction* inst){
    // Only process if instruction needs memory access
    if (inst->MEMflag) {
        printf("\n[Cycle %d] Memory Access: opcode=%d", programCycle, inst->opcode);
        
        switch(inst->opcode) {
            case 10:  // MOVR (LOAD)
                // Load from memory
                int address = inst->operationResult;
                if (address < DATA_START || address > DATA_END) {
                    printf(" [ERROR] Invalid LOAD address %d\n", address);
                    break;
                }

                char* result = mainMemory[address]; // accessing the result from the memory 
                inst->operationResult = (int32_t)strtol(result, NULL, 2); //converting the binary result t int
                printf(", Loaded from  MEM[%d] = %s (as %d)", address, result, inst->operationResult);
                break;
                
            case 11:  // MOVM (STORE)
                // Store to memory
                
                int address = inst->operationResult;
                if (address < DATA_START || address > DATA_END) {
                    printf(" [ERROR] Invalid STORE address %d\n", address);
                    break;
                }                

                char* binaryVal = convertIntToBinary(registerFile[inst->r1], 32);

                strcpy(mainMemory[address], binaryVal);
                printf(", Stored to  R%d = %d (as %s) to MEM[%d]", inst->r1, registerFile[inst->r1], binaryVal, address);
                free(binaryVal); // prevent memory leak
                break;
            
                default:
                // For instructions that reach MEM but don't use it
                printf(" (No memory access for this instruction)");                    
        }
    }
}

void write_back(Instruction* inst){
    if (left > right) return; // No pipeline, no instructions to process

    // Use the passed inst pointer, do not redeclare!
    if (inst->instructionCycle != 5) return;  // Not in writeback stage
    
    // Only process if instruction needs writeback
    if (inst->WBflag) {
        printf("\n[Cycle %d] Write Back: opcode=%d", programCycle, inst->opcode);
        
       if (inst->r1 != 0) { //Register 0 handling 
            registerFile[inst->r1] = inst->operationResult;
            printf(", Writing %d to R%d", inst->operationResult, inst->r1);
        } else {
            // Still print that a write was attempted to R0 (required for logging)
            printf(", Attempted to write %d to R0 (ignored, R0 is always 0)", inst->operationResult);
        }
    }
    
}

//=====================Pipeline Logic=======================
int isFull(){
    return totalPipelined==MAX_PIPELINE_DEPTH;
}
int isEmpty(){
    return totalPipelined==0;
}
int fatalerror=0;
void pipeline() {
    int i = left;
    while (!isEmpty()) {
        int cycle = pipelinedInstructions[i].instructionCycle;
        if(cycle>=7){
            fatalerror=1;
            printf("FATAL ERROR OCCURED %d %d %d %d \n",i,left,right,cycle);
            return;
        }
        // // Enforce your constraints:
        // if (programCycle % 2 == 1) {
        //     // Odd cycle: allow IF (instructionCycle == 1) but skip MEM (5)
        //     if (cycle == 5) {
        //         i = (i + 1) % MAX_PIPELINE_DEPTH;
        //         if (i == (right + 1) % MAX_PIPELINE_DEPTH) break;
        //         continue;
        //     }
        // } else {
        //     // Even cycle: allow MEM (5), but skip IF (1)
        //     if (cycle == 1) {
        //         i = (i + 1) % MAX_PIPELINE_DEPTH;
        //         if (i == (right + 1) % MAX_PIPELINE_DEPTH) break;
        //         continue;
        //     }
        // }
        printf("%d %d %d %d \n",i,left,right,cycle);
        switch(cycle){
            case 1: decode(&pipelinedInstructions[i]);
            case 2: break;
            case 3: execute(&pipelinedInstructions[i], cycle, i);
            case 4: break;
            case 5: memory(&pipelinedInstructions[i]); break;
            case 6: write_back(&pipelinedInstructions[i]);
        }
        pipelinedInstructions[i].instructionCycle++;
        if (i == right) break;

        i = (i + 1) % MAX_PIPELINE_DEPTH;
    }
    if (!isEmpty()&&pipelinedInstructions[left].instructionCycle == 7){
    
        left = (left + 1) % MAX_PIPELINE_DEPTH;totalPipelined--;}

    if (programCycle % 2 == 1 && !isFull()&&pc<maxInstructionIndex) {
        fetch();
        totalPipelined++;
    }

    programCycle++;
}

void run(){
    while(!fatalerror&&(pc<maxInstructionIndex || !isEmpty()))
        pipeline();
}
int main(){
    initialize_program();
    run();
    print_registers();
    print_memory();
    return 0;
}

