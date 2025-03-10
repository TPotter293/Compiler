
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_INSTRUCTIONS 100
#define MAX_ARRAY_SIZE 10


typedef struct {
    char op[4];
    char arg1[10];
    char arg2[10];
    char result[10];
    int is_dead;
    int is_optimized;
    int is_preserved; 
} TACInstruction;

void print_instructions(TACInstruction* instructions, int num_instructions);

// Function declaration for read_TAC
int read_TAC(const char* filename, TACInstruction* instructions);

int is_number(const char* str) {
    char* endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

// Make sure the read_TAC function is implemented in this file
int read_TAC(const char* filename, TACInstruction* instructions) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file");
        return -1;
    }

    int count = 0;
    char line[100];
    while (count < MAX_INSTRUCTIONS && fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (sscanf(line, "%s = %s %s %s", instructions[count].result, instructions[count].arg1, instructions[count].op, instructions[count].arg2) == 4) {
            instructions[count].is_dead = 0;
            instructions[count].is_optimized = 0;
            instructions[count].is_preserved = 0; // Initialize preservation flag
        } else if (sscanf(line, "%s = %s", instructions[count].result, instructions[count].arg1) == 2) {
            instructions[count].op[0] = '\0';
            instructions[count].arg2[0] = '\0';
            instructions[count].is_dead = 0;
            instructions[count].is_optimized = 0;
            instructions[count].is_preserved = 0; // Initialize preservation flag
        } else if (strncmp(line, "print", 5) == 0) {
            sscanf(line, "print %s", instructions[count].arg1);
            strcpy(instructions[count].result, "print");
            instructions[count].op[0] = '\0';
            instructions[count].arg2[0] = '\0';
            instructions[count].is_dead = 0;
            instructions[count].is_optimized = 0;
            instructions[count].is_preserved = 0; // Initialize preservation flag
            
        } else if (strncmp(line, "ifFalse", 7) == 0) {
            sscanf(line, "ifFalse %s goto %s", instructions[count].arg1, instructions[count].arg2);
            strcpy(instructions[count].result, "ifFalse");
            instructions[count].op[0] = '\0';
            instructions[count].is_dead = 0;
            instructions[count].is_optimized = 0;
            instructions[count].is_preserved = 1;  // Preserve conditional jumps
            count++;
        }
        else if (strncmp(line, "label", 5) == 0) {
            sscanf(line, "label %s", instructions[count].arg1);
            strcpy(instructions[count].result, "label");
            instructions[count].op[0] = '\0';
            instructions[count].arg2[0] = '\0';
            instructions[count].is_dead = 0;
            instructions[count].is_optimized = 0;
            instructions[count].is_preserved = 1;  // Preserve labels
            count++;
        }
        else if (strncmp(line, "j", 1) == 0) {
            sscanf(line, "j %s", instructions[count].arg1);
            strcpy(instructions[count].result, "j");
            instructions[count].op[0] = '\0';
            instructions[count].arg2[0] = '\0';
            instructions[count].is_dead = 0;
            instructions[count].is_optimized = 0;
            instructions[count].is_preserved = 1;  // Preserve jumps
            count++;
        }

        else {
            continue;
        }
        printf("Read instruction: %s = %s %s %s\n", instructions[count].result, instructions[count].arg1, instructions[count].op, instructions[count].arg2);
        count++;
    }

    fclose(file);
    return count;
}

int evaluate_constant_expression(int value1, int value2, const char* op) {
    if (strcmp(op, "+") == 0) return value1 + value2;
    if (strcmp(op, "-") == 0) return value1 - value2;
    if (strcmp(op, "*") == 0) return value1 * value2;
    if (strcmp(op, "/") == 0 && value2 != 0) return value1 / value2;
    return value1; // Default case or invalid operation
}

void constant_folding(TACInstruction* instructions, int* num_instructions) {
    for (int i = 0; i < *num_instructions; i++) {
        if (instructions[i].op[0] != '\0') {
            int arg1_val, arg2_val;
            if (is_number(instructions[i].arg1) && is_number(instructions[i].arg2)) {
                // Both operands are constants
                int result = evaluate_constant_expression(atoi(instructions[i].arg1), atoi(instructions[i].arg2), instructions[i].op);
                sprintf(instructions[i].arg1, "%d", result);
                instructions[i].op[0] = '\0';
                instructions[i].arg2[0] = '\0';
                instructions[i].is_optimized = 1;
            }
        }
    }
}


void algebraic_simplification(TACInstruction* instructions, int* num_instructions) {
    for (int i = 0; i < *num_instructions; i++) {
        if (strcmp(instructions[i].op, "+") == 0 || strcmp(instructions[i].op, "-") == 0) {
            if (strcmp(instructions[i].arg2, "0") == 0) {
                // Simplification: x + 0 -> x or x - 0 -> x
                instructions[i].op[0] = '\0';
                instructions[i].arg2[0] = '\0';
                instructions[i].is_optimized = 1;
                printf("Simplified: %s = %s\n", instructions[i].result, instructions[i].arg1);
            }
        } else if (strcmp(instructions[i].op, "*") == 0) {
            if (strcmp(instructions[i].arg1, "1") == 0) {
                // Simplification: 1 * x -> x
                strcpy(instructions[i].arg1, instructions[i].arg2);
                instructions[i].op[0] = '\0';
                instructions[i].arg2[0] = '\0';
                instructions[i].is_optimized = 1;
                printf("Simplified: %s = %s\n", instructions[i].result, instructions[i].arg1);
            } else if (strcmp(instructions[i].arg2, "1") == 0) {
                // Simplification: x * 1 -> x
                instructions[i].op[0] = '\0';
                instructions[i].arg2[0] = '\0';
                instructions[i].is_optimized = 1;
                printf("Simplified: %s = %s\n", instructions[i].result, instructions[i].arg1);
            }
        }
    }
}

void copy_propagation(TACInstruction* instructions, int* num_instructions) {
    for (int i = 0; i < *num_instructions; i++) {

        // Skip propagation for variables used in conditions
        if (strncmp(instructions[i].result, "f", 1) == 0) {
            continue;
        }
        
        if (instructions[i].op[0] == '\0' && !is_number(instructions[i].arg1)) {
            for (int j = i + 1; j < *num_instructions; j++) {
                // Don't propagate into conditional statements
                if (strncmp(instructions[j].result, "f", 1) == 0) {
                    continue;
                }
                
                if (strcmp(instructions[j].arg1, instructions[i].result) == 0) {
                    strcpy(instructions[j].arg1, instructions[i].arg1);
                    instructions[j].is_optimized = 1;
                }
                if (strcmp(instructions[j].arg2, instructions[i].result) == 0) {
                    strcpy(instructions[j].arg2, instructions[i].arg1);
                    instructions[j].is_optimized = 1;
                }
                if (strcmp(instructions[j].result, instructions[i].result) == 0) {
                    break;
                }
            }
        }

        if (strchr(instructions[i].result, '[')) {
            // Handle array assignments like arr[t0] = t1
            for (int j = i + 1; j < *num_instructions; j++) {
                if (strcmp(instructions[j].result, "print") == 0 &&
                    strstr(instructions[j].arg1, instructions[i].result) != NULL) {
                    // If an array element is being printed, check if it can be optimized
                    if (is_number(instructions[i].arg2)) {
                        // Directly replace array access with constant value
                        strcpy(instructions[j].arg1, instructions[i].arg2);
                        instructions[j].is_optimized = 1;
                    } else if (is_number(instructions[i].arg1)) {
                        // If the array index is constant, propagate the value directly
                        sprintf(instructions[j].arg1, "%s", instructions[i].arg2);
                        instructions[j].is_optimized = 1;
                    }
                }
            }
        } else if (strchr(instructions[i].arg1, '[')) {
            // Handle array loads (e.g., arr[0]) and propagate known constant values
            for (int j = i + 1; j < *num_instructions; j++) {
                if (strcmp(instructions[j].result, instructions[i].arg1) == 0) {
                    // If the array element value is known (from a previous assignment), replace it
                    if (is_number(instructions[i].arg2)) {
                        strcpy(instructions[j].arg1, instructions[i].arg2);
                        instructions[j].is_optimized = 1;
                    }
                }
            }
        }
    }
}


void dead_code_elimination(TACInstruction* instructions, int* num_instructions) {
    int used_instructions[MAX_INSTRUCTIONS] = {0};

    // Track if an instruction is used later
    for (int i = 0; i < *num_instructions; i++) {
        // Mark all instructions used in control flow as used
        if (strcmp(instructions[i].result, "ifFalse") == 0 ||
            strcmp(instructions[i].result, "label") == 0 ||
            strncmp(instructions[i].result, "f", 1) == 0) {
            used_instructions[i] = 1;
            
            // Mark all variables used in conditions as used
            for (int j = 0; j < i; j++) {
                if (strcmp(instructions[j].result, instructions[i].arg1) == 0 ||
                    strcmp(instructions[j].result, instructions[i].arg2) == 0) {
                    used_instructions[j] = 1;
                    // Also preserve the original variable assignments
                    for (int k = 0; k < j; k++) {
                        if (strcmp(instructions[k].result, instructions[j].arg1) == 0 ||
                            strcmp(instructions[k].result, instructions[j].arg2) == 0) {
                            used_instructions[k] = 1;
                        }
                    }
                }
            }
        }
        
        // Original dependency tracking
        if (!instructions[i].is_dead && !instructions[i].is_preserved) {

            for (int j = i + 1; j < *num_instructions; j++) {
                if (strcmp(instructions[j].arg1, instructions[i].result) == 0 ||
                    strcmp(instructions[j].arg2, instructions[i].result) == 0 ||
                    (strcmp(instructions[j].result, "print") == 0 &&
                     strcmp(instructions[j].arg1, instructions[i].result) == 0)) {
                    used_instructions[i] = 1;
                    break;
                }
            }
        }
    }

    // Mark instructions as dead

    for (int i = 0; i < *num_instructions; i++) {
        if (!used_instructions[i] && !instructions[i].is_preserved) {
            instructions[i].is_dead = 1;
            instructions[i].is_optimized = 1;
        }
    }
}


void write_TAC(const char* filename, TACInstruction* instructions, int num_instructions) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Could not open file");
        return;
    }

    for (int i = 0; i < num_instructions; i++) {
        if (!instructions[i].is_dead) {
            if (strcmp(instructions[i].result, "print") == 0) {
                fprintf(file, "print %s\n", instructions[i].arg1);
            } else if (strcmp(instructions[i].result, "ifFalse") == 0) {
                fprintf(file, "ifFalse %s goto %s\n", instructions[i].arg1, instructions[i].arg2);
            } else if (strcmp(instructions[i].result, "label") == 0) {
                fprintf(file, "label %s\n", instructions[i].arg1);
            } else if (strcmp(instructions[i].result, "j") == 0) {
                fprintf(file, "j %s\n", instructions[i].arg1);
            } else if (instructions[i].op[0] != '\0') {
                fprintf(file, "%s = %s %s %s\n", instructions[i].result, instructions[i].arg1, instructions[i].op, instructions[i].arg2);
            } else {
                fprintf(file, "%s = %s\n", instructions[i].result, instructions[i].arg1);
            }
        }
    }

    fclose(file);
}


void optimize_TAC(const char* input_filename, const char* output_filename) {
    TACInstruction instructions[MAX_INSTRUCTIONS];
    int num_instructions = read_TAC(input_filename, instructions);
    if (num_instructions == -1) {
        return;
    }

    constant_folding(instructions, &num_instructions);
    algebraic_simplification(instructions, &num_instructions);
    copy_propagation(instructions, &num_instructions);

    // Mark print instructions as preserved
    for (int i = 0; i < num_instructions; i++) {
        if (strcmp(instructions[i].result, "print") == 0) {
            instructions[i].is_preserved = 1;
        }
    }

        // Mark control flow instructions and their dependencies as preserved
    for (int i = 0; i < num_instructions; i++) {
        // Preserve labels
        if (strncmp(instructions[i].result, "label", 5) == 0) {
            instructions[i].is_preserved = 1;
        }
        // Preserve conditional jumps and their conditions
        if (strncmp(instructions[i].result, "f", 1) == 0) {
            instructions[i].is_preserved = 1;
            // Preserve variables used in condition
            for (int j = 0; j < i; j++) {
                if (strcmp(instructions[j].result, instructions[i].arg1) == 0 ||
                    strcmp(instructions[j].result, instructions[i].arg2) == 0) {
                    instructions[j].is_preserved = 1;
                }
            }
        }
    }


    for (int i = 0; i < num_instructions; i++) {
        if (strcmp(instructions[i].result, "ifFalse") == 0) {
            for (int j = 0; j < i; j++) {
                if (strcmp(instructions[j].result, instructions[i].arg1) == 0) {
                    instructions[j].is_preserved = 1;
                    for (int k = 0; k < j; k++) {
                        if (strcmp(instructions[k].result, instructions[j].arg1) == 0 ||
                            strcmp(instructions[k].result, instructions[j].arg2) == 0) {
                            instructions[k].is_preserved = 1;
                        }
                    }
                }
            }
            instructions[i].is_preserved = 1;
        }
    }

    dead_code_elimination(instructions, &num_instructions);

    write_TAC(output_filename, instructions, num_instructions);
}


void print_instructions(TACInstruction* instructions, int num_instructions) {
    for (int i = 0; i < num_instructions; i++) {
        if (!instructions[i].is_dead) {
            if (strcmp(instructions[i].result, "print") == 0) {
                printf("print %s\n", instructions[i].arg1);
            } else if (instructions[i].op[0] != '\0') {
                printf("%s = %s %s %s\n", instructions[i].result, instructions[i].arg1, instructions[i].op, instructions[i].arg2);
            } else {
                printf("%s = %s\n", instructions[i].result, instructions[i].arg1);
            }
        }
    }
    printf("\n");
}
