#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <stdio.h>

typedef struct {
    char result[10];
    char arg1[10];
    char op[4];
    char arg2[10];
} TACInstruction;

void generateCode(const char* tac_filename, FILE* output_file);
void readTACFile(const char* filename);
void generateTACCode(FILE* output_file);
void generateAssignmentCode(TACInstruction* instr, FILE* output_file);
void generateWriteCode(const char* arg, FILE* output_file);
void generateBinaryOpCode(TACInstruction* instr, FILE* output_file);

int is_number_cg(const char* str);
int is_float(const char* str);
void allocateVariable(const char* identifier, int is_float);
int getVariableLocation(const char* identifier);
void freeCodeGenSymbolTable();

#endif // CODE_GENERATOR_H
