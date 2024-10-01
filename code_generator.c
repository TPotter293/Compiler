#include "code_generator.h"
#include "AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARIABLES 100

struct {
    char* name;
    int offset;
} variables[MAX_VARIABLES];
int variable_count = 0;

void generateCode(ASTNode* root, FILE* output_file) {
    printf("DEBUG: Entering generateCode\n");
    fprintf(output_file, ".data\n");
    fprintf(output_file, "newline: .asciiz \"\\n\"\n");
    fprintf(output_file, ".text\n");
    fprintf(output_file, ".globl main\n");
    fprintf(output_file, "main:\n");
    generateProgramCode(root, output_file);
    fprintf(output_file, "li $v0, 10\n");
    fprintf(output_file, "syscall\n");
    printf("DEBUG: Exiting generateCode\n");
}

void generateProgramCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateProgramCode\n");
    generateStatementsCode(node, output_file);
    printf("DEBUG: Exiting generateProgramCode\n");
}

void generateStatementsCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateStatementsCode\n");
    for (int i = 0; i < node->statements.count; i++) {
        printf("DEBUG: Processing statement %d of %d\n", i+1, node->statements.count);
        generateStatementCode(node->statements.stmts[i], output_file);
    }
    printf("DEBUG: Exiting generateStatementsCode\n");
}

void generateStatementCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateStatementCode for node type %d\n", node->type);
    switch (node->type) {
        case NODE_TYPE_DECLARATION:
            generateDeclarationCode(node, output_file);
            break;
        case NODE_TYPE_ASSIGNMENT:
            generateAssignmentCode(node, output_file);
            break;
        case NODE_TYPE_WRITE:
            generateWriteCode(node, output_file);
            break;
        case NODE_TYPE_IF:
            generateIfCode(node, output_file);
            break;
        case NODE_TYPE_RETURN:
            generateReturnCode(node, output_file);
            break;
        default:
            fprintf(stderr, "Unknown statement type in code generation\n");
            exit(1);
    }
    printf("DEBUG: Exiting generateStatementCode\n");
}

void generateDeclarationCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateDeclarationCode\n");
    if (node->right && node->right->id) {
        allocateVariable(node->right->id);
    } else {
        fprintf(stderr, "Error: Invalid declaration node structure\n");
        exit(1);
    }
    printf("DEBUG: Exiting generateDeclarationCode\n");
}

void generateAssignmentCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateAssignmentCode\n");
    generateExpressionCode(node->right, output_file);
    int offset = getVariableLocation(node->left->id);
    fprintf(output_file, "sw $t0, %d($sp)\n", offset);
    printf("DEBUG: Exiting generateAssignmentCode\n");
}

void generateWriteCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateWriteCode\n");
    generateExpressionCode(node->left, output_file);
    fprintf(output_file, "move $a0, $t0\n");
    fprintf(output_file, "li $v0, 1\n");
    fprintf(output_file, "syscall\n");
    fprintf(output_file, "la $a0, newline\n");
    fprintf(output_file, "li $v0, 4\n");
    fprintf(output_file, "syscall\n");
    printf("DEBUG: Exiting generateWriteCode\n");
}

void generateIfCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateIfCode\n");
    static int label_count = 0;
    int current_label = label_count++;
    
    generateExpressionCode(node->left, output_file);
    fprintf(output_file, "beqz $t0, else_%d\n", current_label);
    generateStatementsCode(node->right, output_file);
    fprintf(output_file, "j endif_%d\n", current_label);
    fprintf(output_file, "else_%d:\n", current_label);
    if (node->statements.count > 0) {
        generateStatementsCode(node->statements.stmts[0], output_file);
    }
    fprintf(output_file, "endif_%d:\n", current_label);
    printf("DEBUG: Exiting generateIfCode\n");
}

void generateReturnCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateReturnCode\n");
    generateExpressionCode(node->left, output_file);
    fprintf(output_file, "move $v0, $t0\n");
    fprintf(output_file, "jr $ra\n");
    printf("DEBUG: Exiting generateReturnCode\n");
}

void generateExpressionCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateExpressionCode for node type %d\n", node->type);
    switch (node->type) {
        case NODE_TYPE_NUMBER:
            fprintf(output_file, "li $t0, %d\n", node->value);
            break;
        case NODE_TYPE_IDENTIFIER:
            {
                int offset = getVariableLocation(node->id);
                fprintf(output_file, "lw $t0, %d($sp)\n", offset);
            }
            break;
        case NODE_TYPE_BINARY_OP:
            generateBinaryOpCode(node, output_file);
            break;
        case NODE_TYPE_UNARY_OP:
            generateUnaryOpCode(node, output_file);
            break;
        default:
            fprintf(stderr, "Unknown expression type in code generation\n");
            exit(1);
    }
    printf("DEBUG: Exiting generateExpressionCode\n");
}

void generateBinaryOpCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateBinaryOpCode\n");
    generateExpressionCode(node->left, output_file);
    fprintf(output_file, "move $t1, $t0\n");
    generateExpressionCode(node->right, output_file);
    fprintf(output_file, "move $t2, $t0\n");

    switch (node->op[0]) {
        case '+':
            fprintf(output_file, "add $t0, $t1, $t2\n");
            break;
        case '-':
            fprintf(output_file, "sub $t0, $t1, $t2\n");
            break;
        case '*':
            fprintf(output_file, "mul $t0, $t1, $t2\n");
            break;
        case '/':
            fprintf(output_file, "div $t0, $t1, $t2\n");
            break;
        default:
            fprintf(stderr, "Unknown binary operator in code generation\n");
            exit(1);
    }
    printf("DEBUG: Exiting generateBinaryOpCode\n");
}

void generateUnaryOpCode(ASTNode* node, FILE* output_file) {
    printf("DEBUG: Entering generateUnaryOpCode\n");
    generateExpressionCode(node->left, output_file);
    fprintf(output_file, "not $t0, $t0\n");
    printf("DEBUG: Exiting generateUnaryOpCode\n");
}

void initializeCodeGenSymbolTable() {
    printf("DEBUG: Entering initializeCodeGenSymbolTable\n");
    variable_count = 0;
    printf("DEBUG: Exiting initializeCodeGenSymbolTable\n");
}

void allocateVariable(const char* identifier) {
    printf("DEBUG: Entering allocateVariable for %s\n", identifier);
    if (variable_count >= MAX_VARIABLES) {
        fprintf(stderr, "Too many variables\n");
        exit(1);
    }
    variables[variable_count].name = strdup(identifier);
    variables[variable_count].offset = -4 * (variable_count + 1);
    variable_count++;
    printf("DEBUG: Exiting allocateVariable\n");
}

int getVariableLocation(const char* identifier) {
    printf("DEBUG: Entering getVariableLocation for %s\n", identifier);
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, identifier) == 0) {
            printf("DEBUG: Exiting getVariableLocation, found at offset %d\n", variables[i].offset);
            return variables[i].offset;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", identifier);
    exit(1);
}

void freeCodeGenSymbolTable() {
    printf("DEBUG: Entering freeCodeGenSymbolTable\n");
    for (int i = 0; i < variable_count; i++) {
        free(variables[i].name);
    }
    variable_count = 0;
    printf("DEBUG: Exiting freeCodeGenSymbolTable\n");
}
