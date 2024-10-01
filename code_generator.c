#include "code_generator.h"
#include <stdlib.h>
#include <string.h>

#define MAX_VARIABLES 100
#define MAX_TAC_INSTRUCTIONS 1000

struct {
    char* name;
    int offset;
} variables[MAX_VARIABLES];
int variable_count = 0;

TACInstruction tac_instructions[MAX_TAC_INSTRUCTIONS];
int tac_instruction_count = 0;

void generateCode(const char* tac_filename, FILE* output_file) {
    printf("DEBUG: Entering generateCode\n");
    fprintf(output_file, ".data\n");
    fprintf(output_file, "newline: .asciiz \"\\n\"\n");
    fprintf(output_file, ".text\n");
    fprintf(output_file, ".globl main\n");
    fprintf(output_file, "main:\n");

    readTACFile(tac_filename);
    generateTACCode(output_file);

    fprintf(output_file, "li $v0, 10\n");
    fprintf(output_file, "syscall\n");
    printf("DEBUG: Exiting generateCode\n");
}

void readTACFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening TAC file\n");
        exit(1);
    }

    char line[100];
    while (fgets(line, sizeof(line), file) && tac_instruction_count < MAX_TAC_INSTRUCTIONS) {
        TACInstruction* instr = &tac_instructions[tac_instruction_count];
        if (sscanf(line, "%s = %s %s %s", instr->result, instr->arg1, instr->op, instr->arg2) == 4) {
            tac_instruction_count++;
        } else if (sscanf(line, "%s = %s", instr->result, instr->arg1) == 2) {
            instr->op[0] = '\0';
            instr->arg2[0] = '\0';
            tac_instruction_count++;
        } else if (sscanf(line, "print %s", instr->arg1) == 1) {
            strcpy(instr->result, "print");
            instr->op[0] = '\0';
            instr->arg2[0] = '\0';
            tac_instruction_count++;
        }
    }

    fclose(file);
}

void generateTACCode(FILE* output_file) {
    for (int i = 0; i < tac_instruction_count; i++) {
        TACInstruction* instr = &tac_instructions[i];
        if (strcmp(instr->result, "print") == 0) {
            generateWriteCode(instr->arg1, output_file);
        } else if (instr->op[0] != '\0') {
            generateBinaryOpCode(instr, output_file);
        } else {
            generateAssignmentCode(instr, output_file);
        }
    }
}

void generateAssignmentCode(TACInstruction* instr, FILE* output_file) {
    if (is_number_cg(instr->arg1)) {
        fprintf(output_file, "li $t0, %s\n", instr->arg1);
    } else {
        int offset = getVariableLocation(instr->arg1);
        fprintf(output_file, "lw $t0, %d($sp)\n", offset);
    }
    int offset = getVariableLocation(instr->result);
    fprintf(output_file, "sw $t0, %d($sp)\n", offset);
}

void generateWriteCode(const char* arg, FILE* output_file) {
    if (is_number_cg(arg)) {
        fprintf(output_file, "li $a0, %s\n", arg);
    } else {
        int offset = getVariableLocation(arg);
        fprintf(output_file, "lw $a0, %d($sp)\n", offset);
    }
    fprintf(output_file, "li $v0, 1\n");
    fprintf(output_file, "syscall\n");
    fprintf(output_file, "la $a0, newline\n");
    fprintf(output_file, "li $v0, 4\n");
    fprintf(output_file, "syscall\n");
}

void generateBinaryOpCode(TACInstruction* instr, FILE* output_file) {
    if (is_number_cg(instr->arg1)) {
        fprintf(output_file, "li $t1, %s\n", instr->arg1);
    } else {
        int offset = getVariableLocation(instr->arg1);
        fprintf(output_file, "lw $t1, %d($sp)\n", offset);
    }

    if (is_number_cg(instr->arg2)) {
        fprintf(output_file, "li $t2, %s\n", instr->arg2);
    } else {
        int offset = getVariableLocation(instr->arg2);
        fprintf(output_file, "lw $t2, %d($sp)\n", offset);
    }

    if (strcmp(instr->op, "+") == 0) {
        fprintf(output_file, "add $t0, $t1, $t2\n");
    } else if (strcmp(instr->op, "-") == 0) {
        fprintf(output_file, "sub $t0, $t1, $t2\n");
    } else if (strcmp(instr->op, "*") == 0) {
        fprintf(output_file, "mul $t0, $t1, $t2\n");
    } else if (strcmp(instr->op, "/") == 0) {
        fprintf(output_file, "div $t0, $t1, $t2\n");
    }

    int offset = getVariableLocation(instr->result);
    fprintf(output_file, "sw $t0, %d($sp)\n", offset);
}

int is_number_cg(const char* str) {
    char* endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

void allocateVariable(const char* identifier) {
    if (variable_count >= MAX_VARIABLES) {
        fprintf(stderr, "Too many variables\n");
        exit(1);
    }
    variables[variable_count].name = strdup(identifier);
    variables[variable_count].offset = -4 * (variable_count + 1);
    variable_count++;
}

int getVariableLocation(const char* identifier) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, identifier) == 0) {
            return variables[i].offset;
        }
    }
    allocateVariable(identifier);
    return variables[variable_count - 1].offset;
}

void freeCodeGenSymbolTable() {
    for (int i = 0; i < variable_count; i++) {
        free(variables[i].name);
    }
    variable_count = 0;
}
