#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_TAC 100
#define MAX_VARS 20

typedef struct {
    char op[4];    // Operation: +, -, *, /
    char arg1[10]; // First argument
    char arg2[10]; // Second argument
    char result[10]; // Result
} TAC;

// Function to check if a string represents a constant
bool is_constant(const char *str) {
    if (*str == '\0') return false;
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}

// Function to evaluate a simple arithmetic expression if both arguments are constants
int evaluate_expression(const char *op, const char *arg1, const char *arg2) {
    int value1 = atoi(arg1);
    int value2 = atoi(arg2);
    
    if (strcmp(op, "+") == 0) return value1 + value2;
    if (strcmp(op, "-") == 0) return value1 - value2;
    if (strcmp(op, "*") == 0) return value1 * value2;
    if (strcmp(op, "/") == 0) return value1 / value2;

    return 0; // In case of unknown operation
}

// Function to perform constant propagation optimization
void constant_propagation(TAC tac[], int count) {
    for (int i = 0; i < count; i++) {
        // Print the original TAC for debugging
        printf("Original TAC[%d]: %s = %s %s %s\n", i, tac[i].result, tac[i].arg1, tac[i].op, tac[i].arg2);

        // Replace assignments of constants
        if (strcmp(tac[i].op, "=") == 0 && is_constant(tac[i].arg1)) {
            // Update the TAC with constant value
            sprintf(tac[i].arg1, "%s", tac[i].arg1);
            printf("Updated TAC[%d]: %s = %s (constant propagation)\n", i, tac[i].result, tac[i].arg1);
        }

        // Try to evaluate expressions with constants
        if (strcmp(tac[i].op, "+") == 0 && is_constant(tac[i].arg1) && is_constant(tac[i].arg2)) {
            int result = evaluate_expression(tac[i].op, tac[i].arg1, tac[i].arg2);
            strcpy(tac[i].op, "=");
            sprintf(tac[i].arg1, "%d", result);
            strcpy(tac[i].arg2, ""); // Clear arg2
            printf("Evaluated TAC[%d]: %s = %s (result)\n", i, tac[i].result, tac[i].arg1);
        }
    }
}

// Function to optimize TAC by reading from input file, performing optimization, and writing to output file
void optimize_TAC(const char* input_file, const char* output_file) {
    TAC tac[MAX_TAC];
    int count = 0;

    FILE *in = fopen(input_file, "r");
    if (in == NULL) {
        printf("Error: Could not open input file.\n");
        return;
    }

    // Read the TAC from the input file
    while (count < MAX_TAC && fscanf(in, "%s = %s %s %s", tac[count].result, tac[count].arg1, tac[count].op, tac[count].arg2) == 4) {
        count++;
    }

    while (count < MAX_TAC && fscanf(in, "%s = %s", tac[count].result, tac[count].arg1) == 2) {
        strcpy(tac[count].op, "=");
        strcpy(tac[count].arg2, "");
        count++;
    }

    fclose(in);
    
    // Perform constant propagation optimization
    constant_propagation(tac, count);

    // Write the optimized TAC to the output file
    FILE *out = fopen(output_file, "w");
    if (out == NULL) {
        printf("Error: Could not open output file.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (strlen(tac[i].arg2) == 0) {
            fprintf(out, "%s = %s\n", tac[i].result, tac[i].arg1);
        } else {
            fprintf(out, "%s = %s %s %s\n", tac[i].result, tac[i].arg1, tac[i].op, tac[i].arg2);
        }
    }

    fclose(out);
}
