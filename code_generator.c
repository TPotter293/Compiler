#include "code_generator.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h> // Include for debugging output

#define MAX_VARIABLES 100
#define MAX_TAC_INSTRUCTIONS 1000

struct {
    char* name;
    int offset;
    int is_float; // Track if the variable is a float
} variables[MAX_VARIABLES];
int variable_count = 0;

TACInstruction tac_instructions[MAX_TAC_INSTRUCTIONS];
int tac_instruction_count = 0;

void generateCode(const char* tac_filename, FILE* output_file) {
    fprintf(output_file, ".data\n");
    fprintf(output_file, "newline: .asciiz \"\\n\"\n");
    fprintf(output_file, ".text\n");
    fprintf(output_file, ".globl main\n");
    fprintf(output_file, "main:\n");

    // Add this line to initialize stack pointer
    fprintf(output_file, "addi $sp, $sp, -100\n");  // Allocate stack space

    printf("Generating code from TAC file: %s\n", tac_filename);
    readTACFile(tac_filename);
    generateTACCode(output_file);
    
    fprintf(output_file, "li $v0, 10\n");
    fprintf(output_file, "syscall\n");
    printf("Code generation completed.\n");
}

void readTACFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening TAC file\n");
        exit(1);
    }

    printf("Reading TAC file: %s\n", filename);
    char line[100];
    while (fgets(line, sizeof(line), file) && tac_instruction_count < MAX_TAC_INSTRUCTIONS) {
        TACInstruction* instr = &tac_instructions[tac_instruction_count];
        if (sscanf(line, "%s = %s %s %s", instr->result, instr->arg1, instr->op, instr->arg2) == 4) {
            printf("Parsed instruction: %s = %s %s %s\n", instr->result, instr->arg1, instr->op, instr->arg2);
            tac_instruction_count++;
        } else if (sscanf(line, "%s = %s", instr->result, instr->arg1) == 2) {
            instr->op[0] = '\0';
            instr->arg2[0] = '\0';
            printf("Parsed assignment: %s = %s\n", instr->result, instr->arg1);
            tac_instruction_count++;
        } else if (sscanf(line, "print %s", instr->arg1) == 1) {
            strcpy(instr->result, "print");
            instr->op[0] = '\0';
            instr->arg2[0] = '\0';
            printf("Parsed print instruction: %s\n", instr->arg1);
            tac_instruction_count++;
        } else if (sscanf(line, "ifFalse %s goto %s", instr->arg1, instr->arg2) == 2) {
            strcpy(instr->result, "ifFalse");
            instr->op[0] = '\0';
            printf("Parsed ifFalse instruction: %s goto %s\n", instr->arg1, instr->arg2);
            tac_instruction_count++;
        } else if (sscanf(line, "label %s::", instr->arg1) == 1) {
            // Label instruction
            strcpy(instr->result, "label");  // Set result to "label"
            instr->op[0] = '\0';             // Clear out op
            instr->arg2[0] = '\0';           // Clear out arg2
            printf("Parsed label: %s\n", instr->arg1);  // Show label name
            tac_instruction_count++;

        }
    }

    fclose(file);
    printf("Finished reading TAC file. Total instructions: %d\n", tac_instruction_count);
}

void generateTACCode(FILE* output_file) {
    printf("Generating TAC code...\n");
    for (int i = 0; i < tac_instruction_count; i++) {
        TACInstruction* instr = &tac_instructions[i];
        printf("####### Instruction %d: result='%s', arg1='%s', op='%s', arg2='%s'\n", 
                i, 
                instr->result, 
                instr->arg1, 
                instr->op, 
                instr->arg2);

        if (strcmp(instr->result, "print") == 0) {
            generateWriteCode(instr->arg1, output_file);
        } else if (strncmp(instr->result, "f", 1) == 0) {
            // Generate comparison code
            int offset1 = getVariableLocation(instr->arg1);
            int offset2 = getVariableLocation(instr->arg2);
            fprintf(output_file, "lw $t1, %d($sp)\n", offset1);    // Load x
            fprintf(output_file, "lw $t2, %d($sp)\n", offset2);    // Load y
            fprintf(output_file, "slt $t0, $t2, $t1\n");          // Set if y < x (same as x > y)
            fprintf(output_file, "sw $t0, %d($sp)\n", getVariableLocation(instr->result));
        } else if (strcmp(instr->result, "ifFalse") == 0) {
            // Load condition value into $t0
            int offset = getVariableLocation(instr->arg1);
            fprintf(output_file, "lw $t0, %d($sp)\n", offset);

            // Branch to the label if $t0 is zero
            fprintf(output_file, "beq $t0, $zero, %s\n", instr->arg2);
            printf("Generated ifFalse: branch to %s if %s is 0\n", instr->arg2, instr->arg1);
        } else if (strcmp(instr->result, "label") == 0) {
            fprintf(output_file, "%s:\n", instr->arg1);
            printf("Generated label: %s\n", instr->arg1);
        } else if (instr->op[0] != '\0') {
            generateBinaryOpCode(instr, output_file);
        } else {
            generateAssignmentCode(instr, output_file);
        }
    }
    printf("TAC code generation completed.\n");
}



void generateAssignmentCode(TACInstruction* instr, FILE* output_file) {
    printf("Generating assignment code for: %s = %s\n", instr->result, instr->arg1);
    if (is_int(instr->arg1)) {
        fprintf(output_file, "li $t0, %s\n", instr->arg1);
        printf("Loaded integer literal into $t0: %s\n", instr->arg1);
    } else if (is_float(instr->arg1)) {
        fprintf(output_file, "li.s $f0, %s\n", instr->arg1);
        printf("Loaded float literal into $f0: %s\n", instr->arg1);
    } else {
        // Load variable value
        int offset = getVariableLocation(instr->arg1);
        if (is_float(instr->arg1)) {
            fprintf(output_file, "l.s $f0, %d($sp)\n", offset);
            printf("Loaded float variable into $f0 from offset: %d\n", offset);
        } else {
            fprintf(output_file, "lw $t0, %d($sp)\n", offset);
            printf("Loaded integer variable into $t0 from offset: %d\n", offset);
        }
    }

    int result_offset = getVariableLocation(instr->result);
    if (is_float(instr->arg1)) {
        fprintf(output_file, "s.s $f0, %d($sp)\n", result_offset);
        printf("Stored float result into offset: %d\n", result_offset);
    } else {
        fprintf(output_file, "sw $t0, %d($sp)\n", result_offset);
        printf("Stored integer result into offset: %d\n", result_offset);
    }
}

void generateWriteCode(const char* arg, FILE* output_file) {
    printf("Generating write code for: %s\n", arg);
    if (is_float(arg)) {
        int offset = getVariableLocation(arg);
        fprintf(output_file, "l.s $f0, %d($sp)\n", offset);
        fprintf(output_file, "li $v0, 2\n"); // Print float
    } else {
        int offset = getVariableLocation(arg);
        fprintf(output_file, "lw $a0, %d($sp)\n", offset);
        fprintf(output_file, "li $v0, 1\n"); // Print integer
    }
    fprintf(output_file, "syscall\n");
    fprintf(output_file, "la $a0, newline\n");
    fprintf(output_file, "li $v0, 4\n");
    fprintf(output_file, "syscall\n");
}

void generateBinaryOpCode(TACInstruction* instr, FILE* output_file) {
    printf("Generating binary operation code for: %s = %s %s %s\n", instr->result, instr->arg1, instr->op, instr->arg2);
    int offset1 = getVariableLocation(instr->arg1);
    int offset2 = getVariableLocation(instr->arg2);
    
    if (is_float(instr->arg1)) {
        fprintf(output_file, "l.s $f1, %d($sp)\n", offset1);
        printf("Loaded float variable into $f1 from offset: %d\n", offset1);
    } else {
        fprintf(output_file, "lw $t1, %d($sp)\n", offset1);
        printf("Loaded integer variable into $t1 from offset: %d\n", offset1);
    }

    if (is_float(instr->arg2)) {
        fprintf(output_file, "l.s $f2, %d($sp)\n", offset2);
        printf("Loaded float variable into $f2 from offset: %d\n", offset2);
    } else {
        fprintf(output_file, "lw $t2, %d($sp)\n", offset2);
        printf("Loaded integer variable into $t2 from offset: %d\n", offset2);
    }

    if (strcmp(instr->op, "+") == 0) {
        if (is_float(instr->arg1) || is_float(instr->arg2)) {
            fprintf(output_file, "add.s $f0, $f1, $f2\n");
            printf("Performed float addition: $f0 = $f1 + $f2\n");
        } else {
            fprintf(output_file, "add $t0, $t1, $t2\n");
            printf("Performed integer addition: $t0 = $t1 + $t2\n");
        }
    } else if (strcmp(instr->op, "-") == 0) {
        if (is_float(instr->arg1) || is_float(instr->arg2)) {
            fprintf(output_file, "sub.s $f0, $f1, $f2\n");
            printf("Performed float subtraction: $f0 = $f1 - $f2\n");
        } else {
            fprintf(output_file, "sub $t0, $t1, $t2\n");
            printf("Performed integer subtraction: $t0 = $t1 - $t2\n");
        }
    } else if (strcmp(instr->op, "*") == 0) {
        if (is_float(instr->arg1) || is_float(instr->arg2)) {
            fprintf(output_file, "mul.s $f0, $f1, $f2\n");
            printf("Performed float multiplication: $f0 = $f1 * $f2\n");
        } else {
            fprintf(output_file, "mul $t0, $t1, $t2\n");
            printf("Performed integer multiplication: $t0 = $t1 * $t2\n");
        }
    } else if (strcmp(instr->op, "/") == 0) {
        if (is_float(instr->arg1) || is_float(instr->arg2)) {
            fprintf(output_file, "div.s $f0, $f1, $f2\n");
            printf("Performed float division: $f0 = $f1 / $f2\n");
        } else {
            fprintf(output_file, "div $t0, $t1, $t2\n");
            printf("Performed integer division: $t0 = $t1 / $t2\n");
        }
    }

    int result_offset = getVariableLocation(instr->result);
    if (is_float(instr->arg1) || is_float(instr->arg2)) {
        fprintf(output_file, "s.s $f0, %d($sp)\n", result_offset);
        printf("Stored float result into offset: %d\n", result_offset);
    } else {
        fprintf(output_file, "sw $t0, %d($sp)\n", result_offset);
        printf("Stored integer result into offset: %d\n", result_offset);
    }
}

int is_int(const char* str) {
    char* endptr;
    strtol(str, &endptr, 10);
    int result = *endptr == '\0';
    printf("is_int(%s) = %d\n", str, result);
    return result;
}

int is_float(const char* str) {
    char* endptr;

    // Use strtof to convert the string to a float
    strtof(str, &endptr);

    // Check if the conversion consumed the entire string
    // Check for the presence of a decimal point and ensure there are digits after it
    int result = (*endptr == '\0') && (strchr(str, '.') != NULL);

    // Additional check to ensure there are digits after the decimal point
    if (result) {
        char* decimal_point = strchr(str, '.');
        if (decimal_point != NULL) {
            if (strlen(decimal_point) == 1 || !isdigit(*(decimal_point + 1))) {
                result = 0; // No digits after the decimal point
            }
        }
    }

    printf("is_float(%s) = %d\n", str, result);
    return result;
}

void allocateVariable(const char* identifier, int is_float) {
    if (variable_count >= MAX_VARIABLES) {
        fprintf(stderr, "Too many variables\n");
        exit(1);
    }
    variables[variable_count].name = strdup(identifier);
    variables[variable_count].offset = -4 * (variable_count + 1);
    variables[variable_count].is_float = is_float;
    printf("Allocated variable: %s, offset: %d, is_float: %d\n", identifier, variables[variable_count].offset, is_float);
    variable_count++;
}

int getVariableLocation(const char* identifier) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, identifier) == 0) {
            printf("Found variable %s at offset: %d\n", identifier, variables[i].offset);
            return variables[i].offset;
        }
    }
    allocateVariable(identifier, is_float(identifier));
    printf("Allocated new variable %s at offset: %d\n", identifier, variables[variable_count - 1].offset);
    return variables[variable_count - 1].offset;
}

void freeCodeGenSymbolTable() {
    for (int i = 0; i < variable_count; i++) {
        free(variables[i].name);
    }
    variable_count = 0;
    printf("Freed symbol table.\n");
}
