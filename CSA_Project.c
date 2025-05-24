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
typedef struct {
    int instructionID;
    char encodedInstruction[33]; 
    int instructionCycle; // to be set = 1 
    int operationResult;
    int oldpc;
    //After Decoding the below values should be filled
    InstrType type;
    int32_t imm; //signed immediate values 
    int opcode, r1, r2, r3, shamt, address;
    int WBflag,MEMflag;
    char *instrName;
} Instruction;

//=====================Pipeline===================
Instruction pipelinedInstructions[MAX_PIPELINE_DEPTH];
int left=0; 
int right=-1;
int totalPipelined=0;
int programCycle=1; 
int totalFetched=0;
int totalDataHazardDelay=0;
//==========================================Code==========================================
//=====================Printing===============================
void print_registers() {
    printf("\nRegister values :\n");
    for (int i = 1; i <= 12; i++) {
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
char* getInstructionName(int opcode){
    switch(opcode){
        case 0: return "ADD ";
        case 1: return "SUB ";
        case 2: return "MUL ";
        case 3: return "MOVI";
        case 4: return "JEQ ";
        case 5: return "AND ";
        case 6: return "XORI";
        case 7: return "JMP ";
        case 8: return "LSL ";
        case 9: return "LSR ";
        case 10: return "MOVR";
        case 11: return "MOVM";
        default: return "Undefined Opcode";
    }
}
char* getInstructionPhase(int instrucitonCycle){
    switch(instrucitonCycle){
        case 1: return "Fetch";
        case 2: return "Decode";
        case 3: return "Dummy Decode";
        case 4: return "Execute";
        case 5: return "Dummy Execute";
        case 6: return "Memory";
        case 7: return "Write Back";
        default: return "Undefined Instruction Cycle"; 
    }
}
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
char* encode_opcode(char* plainOpcode){ 
    if (strcmp(plainOpcode, "ADD ") == 0) return "0000"; 
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
    return "Undefined Opcode Cant Encode";
}

char* encode_instruction(char* plainInstruction){
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
    int len_op = strlen(encodedOpCode);

    if(type==R){
        remaining=convertIntToBinary(atoi(rem),13);
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
    else if(type==I){
        remaining=convertIntToBinary(atoi(rem),18);
        int len_r1 = strlen(encodedR1);
        int len_r2 = strlen(encodedR2);
        int len_rem = strlen(remaining);
        int total_len = len_op + len_r1 + len_r2  + len_rem;
        char* merged = malloc(total_len + 1);
        strcpy(merged, encodedOpCode);
        strcat(merged, encodedR1);
        strcat(merged, encodedR2);
        strcat(merged, remaining);
        return merged;

    }

    else{
        remaining=convertIntToBinary(atoi(rem),28);
        int len_rem = strlen(remaining);
        int total_len = len_op  + len_rem;
        char* merged = malloc(total_len + 1);
        strcpy(merged, encodedOpCode);
        strcat(merged, remaining);
        return merged;

    }
    
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
        char str[33];
        strncpy(str, encode_instruction(line), 33);        
        strncpy(mainMemory[index],str , 33);
        mainMemory[index][32] = '\0'; 
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
    totalPipelined++;
    right=(right+1)%4; 
    Instruction instr;
    strcpy(instr.encodedInstruction, mainMemory[pc]);
    instr.instructionCycle=2;
    pc++;
    instr.instructionID=++totalFetched;
    printf("\nRunning Instruction Number : %d | Pipeline positon : %d | Clock cycle : %d ",instr.instructionID,right,1);
    printf("\nPhase: Fetch | Instruction Number : %d \n",instr.instructionID);
    pipelinedInstructions[right] =instr;
    instr.oldpc=pc;
}
void setInstructionVariables(Instruction* instr, char* opcode){
    instr->type = get_instr_type(opcode);
    instr->instrName = malloc(strlen(getInstructionName(instr->opcode)) + 1);
    strcpy(instr->instrName, getInstructionName(instr->opcode));
    switch(instr->opcode){
        case  0: case 1: case 2: case 3: case 5: case 6: case 8: case 9: case  10:
        instr->MEMflag=0;instr->WBflag=1;break;
        case 11:
        instr->MEMflag=1;instr->WBflag=0;break;
        case 4: case 7:
        instr->MEMflag=0;instr->WBflag=0;break;
    }
    instr->r1=-1;instr->r2=-1;instr->r3=-1;
    instr->shamt=-1;instr->address=-1;instr->imm=-1;
    instr->operationResult=-1;
}
void decode(Instruction* instr){
    char opcode[5];
    char er1[6];
    char er2[6];
    char er3[6];

    strncpy(opcode, instr->encodedInstruction, 4);
    opcode[4]='\0';
    instr->opcode = strtol(opcode, NULL, 2);
    setInstructionVariables(instr, opcode);
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
}

void execute(Instruction* inst, int cycle,int i){
    switch (inst->opcode) {
        case 0: //ADD
            inst->operationResult = registerFile[inst->r2] + registerFile[inst->r3];
            break;
        case 1: //SUB
            inst->operationResult = registerFile[inst->r2] - registerFile[inst->r3];
            break;
        case 2: //MUL
            inst->operationResult = registerFile[inst->r2] * registerFile[inst->r3];
            break;
        case 3: //MOVI
            inst->operationResult = inst->imm; 
            break;
        case 4: //JEQ
            right = i;
            if (registerFile[inst->r1] == registerFile[inst->r2]) 
                pc = inst->oldpc + inst->imm;
            totalPipelined=  totalPipelined - ((right-i)+MAX_PIPELINE_DEPTH)%MAX_PIPELINE_DEPTH;
            break;
        case 5: //AND
            inst->operationResult = registerFile[inst->r2] & registerFile[inst->r3];
            break;
        case 6: //XORI
            inst->operationResult = registerFile[inst->r2] ^ inst->imm;
            break;
        case 7: //JMP
            right = i;
            pc = (inst->oldpc & 0xF0000000) | (inst->address & 0x0FFFFFFF);
            totalPipelined =  totalPipelined - ((right-i)+MAX_PIPELINE_DEPTH)%MAX_PIPELINE_DEPTH;
            return;
        case 8: //LSL
            inst->operationResult = registerFile[inst->r2] << inst->shamt;
            break;
        case 9: //LSR
            inst->operationResult = registerFile[inst->r2] >> inst->shamt;
            break;
        case 10: //MOVR
            inst->operationResult = registerFile[inst->r2] + inst->imm;
            break;
        case 11: //MOVM
            inst->operationResult = registerFile[inst->r2] + inst->imm;
            break;
    }
}

void memory(Instruction* inst){
    // Only process if instruction needs memory access
    if (inst->MEMflag) {
        int address;
        switch(inst->opcode) {
            case 10:  // MOVR (LOAD)
                // Load from memory
                address= inst->operationResult;
                if (address < DATA_START || address > DATA_END) {
                    break;
                }

                char* result = mainMemory[address]; // accessing the result from the memory 
                inst->operationResult = (int32_t)strtol(result, NULL, 2); //converting the binary result t int
                break;
                
            case 11:  // MOVM (STORE)
                // Store to memory
                
                 address = inst->operationResult;
                if (address < DATA_START || address > DATA_END) {
                    break;
                }                
                char* binaryVal = convertIntToBinary(registerFile[inst->r1], 32);

                strcpy(mainMemory[address], binaryVal);
                free(binaryVal); // prevent memory leak
                break;
            
                default:
        }
    }
}

void write_back(Instruction* inst){
    // Only process if instruction needs writeback
    if (inst->WBflag) {
        
       if (inst->r1 != 0) { //Register 0 handling 
            registerFile[inst->r1] = inst->operationResult;
        } else {
            // Still print that a write was attempted to R0 (required for logging)
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
int dataHazard(int i){
    int j=left;
    while (!isEmpty()) {
        if (j == i) break;
        if(pipelinedInstructions[j].instructionCycle<=6&&pipelinedInstructions[j].MEMflag){
            // How to stop memory idk?

        }else if((pipelinedInstructions[j].instructionCycle<=7&&pipelinedInstructions[j].WBflag))
            if(pipelinedInstructions[j].r1 ==pipelinedInstructions[i].r2 ||pipelinedInstructions[j].r1 ==pipelinedInstructions[i].r3 )
                return 1;

        j = (j + 1) % MAX_PIPELINE_DEPTH;
    }
    return 0;
}
int fatalHazard=0;
void pipeline() {
    int i = left;int dataHazardOccured=0;
    printf("============================================[Starting Clock Cycle: %d]============================================ \n",programCycle);
    printf("Data about this clock cycle | left = %d | right = %d | totalPipelined = %d \n",left,right,totalPipelined);
    while (!isEmpty()) {
        int cycle = pipelinedInstructions[i].instructionCycle;
    printf("\nRunning Instruction Number : %d | Pipeline positon : %d | Clock cycle : %d ",pipelinedInstructions[i].instructionID,i,cycle);

        if(cycle>=8){
            fatalHazard=1;
            printf("FATAL HAZARD OCCURED BECAUSE OF INSTRUCTION NUMBER : %d \n",pipelinedInstructions[i].instructionID);
            return;
        }

        if(cycle >=4 && dataHazard(i)){
            dataHazardOccured=1;
            totalDataHazardDelay++;
            printf("\nData Hazard Detected in instruction : %d | Will delay all instructions by 1 clock cycle \n",pipelinedInstructions[i].instructionID);
            break;
        }
        switch(cycle){
            case 2:
            printf("\nPhase: Decode | Instruction Number : %d \n",pipelinedInstructions[i].instructionID);
            decode(&pipelinedInstructions[i]);break;
            case 3:printf("\nPhase: Dummy Decode | Instruction Number : %d \n",pipelinedInstructions[i].instructionID);break;
            case 4:
            printf("\nPhase: Execute | Instruction Number : %d \n",pipelinedInstructions[i].instructionID);
            execute(&pipelinedInstructions[i], cycle, i);break;
            case 5:printf("\nPhase: Dummy Execute | Instruction Number : %d \n",pipelinedInstructions[i].instructionID); break;
            case 6:
            printf("\nPhase: Memory | Instruction Number : %d \n",pipelinedInstructions[i].instructionID);
            memory(&pipelinedInstructions[i]); break;
            case 7:printf("\nPhase: Write Back |Instruction Number : %d \n",pipelinedInstructions[i].instructionID); 
            write_back(&pipelinedInstructions[i]);
        }
        pipelinedInstructions[i].instructionCycle++;
        if (i == right) break;

        i = (i + 1) % MAX_PIPELINE_DEPTH;
    }
    int removedInstruction=0;
    if (!isEmpty()&&pipelinedInstructions[left].instructionCycle == 8){
        printf("\nInstruction number: %d | Completed at cycle : %d | Removing From Pipeline \n" ,pipelinedInstructions[left].instructionID,programCycle);
        left = (left + 1) % MAX_PIPELINE_DEPTH;totalPipelined--;
        removedInstruction=1;
    }

    if (programCycle % 2 == 1 && !isFull()&&pc<maxInstructionIndex && !dataHazardOccured && !removedInstruction) 
        fetch();


    printf("\n============================================[Ending Clock Cycle: %d]==============================================\n \n",programCycle);
    programCycle++;
}

void run(){
    while(!fatalHazard&&(pc<maxInstructionIndex || !isEmpty()))
        pipeline();
}
int main(){
    initialize_program();
  
    run();
    print_registers();
   // print_memory();
    return 0;
}

