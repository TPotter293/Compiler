#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#include "symbol_table.h"

#define MAX_IDENTIFIERS 100  // Define MAX_IDENTIFIERS

FILE* tac_file;
int temp_var_count = 0;
int free_temp_regs[MAX_IDENTIFIERS]; // Track free temp registers
int free_temp_reg_count = 0;

struct {
    char name[20];
    int temp_var;
    int is_var;  // 1 if this temp holds a variable, 0 if it's temporary
} id_to_temp[MAX_IDENTIFIERS];
int id_to_temp_count = 0;

// Function declarations
void updateIdToTemp(const char* id, int temp_var);
char* newTemp(int is_var);
void freeTemp(int temp_var);
void generateTACLine(const char* tac_line);
void generateTAC(ASTNode* node);
void analyzeNode(ASTNode* node);
void analyzeProgram(ASTNode* node);
void analyzeDeclaration(ASTNode* node);
void analyzeAssignment(ASTNode* node);
void analyzeWrite(ASTNode* node);
void analyzeBinaryOp(ASTNode* node);
void analyzeIdentifier(ASTNode* node);
void performSemanticAnalysis(ASTNode* root);

char* newTemp(int is_var) {
    char* temp = (char*)malloc(10);
    int temp_var;

    if (free_temp_reg_count > 0) {
        // Reuse a free temporary register
        temp_var = free_temp_regs[--free_temp_reg_count];
    } else {
        temp_var = temp_var_count++;
    }
    
    sprintf(temp, "t%d", temp_var);

    // Store the new temp as either variable or temporary
    if (is_var) {
        updateIdToTemp(temp, temp_var);
    }

    return temp;
}

void freeTemp(int temp_var) {
    free_temp_regs[free_temp_reg_count++] = temp_var; // Mark temp as free
}

void generateTACLine(const char* tac_line) {
    fprintf(tac_file, "%s\n", tac_line);
}

void generateTAC(ASTNode* node) {
    if (node == NULL) {
        return;
    }

    char tac_line[100];
    switch (node->type) {
        case NODE_TYPE_PROGRAM:
            for (int i = 0; i < node->statements.count; i++) {
                generateTAC(node->statements.stmts[i]);
            }
            break;
        case NODE_TYPE_DECLARATION:
            // Declarations are not output to TAC
            break;
        case NODE_TYPE_ASSIGNMENT:
            generateTAC(node->right);
            sprintf(tac_line, "%s = t%d", node->left->id, temp_var_count - 1);
            generateTACLine(tac_line);
            updateIdToTemp(node->left->id, temp_var_count - 1);
            break;
        case NODE_TYPE_WRITE:
            generateTAC(node->left);
            sprintf(tac_line, "print t%d", temp_var_count - 1);
            generateTACLine(tac_line);
            break;
        case NODE_TYPE_BINARY_OP:
            generateTAC(node->left);
            int left_temp = temp_var_count - 1;
            generateTAC(node->right);
            int right_temp = temp_var_count - 1;
            sprintf(tac_line, "t%d = t%d %s t%d", temp_var_count, left_temp, node->op, right_temp);
            generateTACLine(tac_line);
            temp_var_count++;
            freeTemp(left_temp);
            freeTemp(right_temp);
            break;
        case NODE_TYPE_IDENTIFIER:
            sprintf(tac_line, "t%d = %s", temp_var_count, node->id);
            generateTACLine(tac_line);
            temp_var_count++;
            break;
        case NODE_TYPE_NUMBER:
            sprintf(tac_line, "t%d = %d", temp_var_count, node->value);
            generateTACLine(tac_line);
            temp_var_count++;
            break;
        default:
            fprintf(stderr, "Error: Unknown node type %d in TAC generation\n", node->type);
            exit(1);
    }
}

void updateIdToTemp(const char* id, int temp_var) {
    for (int i = 0; i < id_to_temp_count; i++) {
        if (strcmp(id_to_temp[i].name, id) == 0) {
            id_to_temp[i].temp_var = temp_var;
            id_to_temp[i].is_var = 1;  // Mark it as a variable
            return;
        }
    }
    if (id_to_temp_count < MAX_IDENTIFIERS) {
        strcpy(id_to_temp[id_to_temp_count].name, id);
        id_to_temp[id_to_temp_count].temp_var = temp_var;
        id_to_temp[id_to_temp_count].is_var = 1;  // Mark it as a variable
        id_to_temp_count++;
    }
}

int getIdIndex(const char* id) {
    for (int i = 0; i < id_to_temp_count; i++) {
        if (strcmp(id_to_temp[i].name, id) == 0) {
            return i;
        }
    }
    return -1;
}

void analyzeNode(ASTNode* node);

void analyzeProgram(ASTNode* node) {
    for (int i = 0; i < node->statements.count; i++) {
        analyzeNode(node->statements.stmts[i]);
    }
}

void analyzeDeclaration(ASTNode* node) {
    // Declarations are not output to TAC
}

void analyzeAssignment(ASTNode* node) {
    analyzeNode(node->right);

    char* id = node->left->id;
    char tac_line[100];
    sprintf(tac_line, "%s = t%d", id, temp_var_count - 1);
    generateTACLine(tac_line);

    updateIdToTemp(id, temp_var_count - 1);
}

void analyzeWrite(ASTNode* node) {
    analyzeNode(node->left);

    char tac_line[100];
    if (node->left->type == NODE_TYPE_IDENTIFIER) {
        int index = getIdIndex(node->left->id);
        if (index != -1) {
            sprintf(tac_line, "print t%d", id_to_temp[index].temp_var);
        } else {
            sprintf(tac_line, "print %s", node->left->id);
        }
    } else {
        sprintf(tac_line, "print t%d", temp_var_count - 1);
    }
    generateTACLine(tac_line);
}

void analyzeBinaryOp(ASTNode* node) {
    analyzeNode(node->left);
    analyzeNode(node->right);

    char* temp = newTemp(0);  // Temporary value, not a variable
    char tac_line[100];
    char left_operand[10], right_operand[10];

    if (node->left->type == NODE_TYPE_IDENTIFIER) {
        int index = getIdIndex(node->left->id);
        if (index != -1) {
            sprintf(left_operand, "t%d", id_to_temp[index].temp_var);
        } else {
            strcpy(left_operand, node->left->id);
        }
    } else {
        sprintf(left_operand, "t%d", temp_var_count - 2);
    }

    if (node->right->type == NODE_TYPE_IDENTIFIER) {
        int index = getIdIndex(node->right->id);
        if (index != -1) {
            sprintf(right_operand, "t%d", id_to_temp[index].temp_var);
        } else {
            strcpy(right_operand, node->right->id);
        }
    } else {
        sprintf(right_operand, "t%d", temp_var_count - 1);
    }

    sprintf(tac_line, "%s = %s %s %s", temp, left_operand, node->op, right_operand);
    generateTACLine(tac_line);
}

void analyzeIdentifier(ASTNode* node) {
    // Skip TAC generation for identifiers
}

void analyzeNode(ASTNode* node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE_TYPE_PROGRAM:
            analyzeProgram(node);
            break;
        case NODE_TYPE_DECLARATION:
            analyzeDeclaration(node);
            break;
        case NODE_TYPE_ASSIGNMENT:
            analyzeAssignment(node);
            break;
        case NODE_TYPE_WRITE:
            analyzeWrite(node);
            break;
        case NODE_TYPE_BINARY_OP:
            analyzeBinaryOp(node);
            break;
        case NODE_TYPE_IDENTIFIER:
            analyzeIdentifier(node);
            break;
        case NODE_TYPE_NUMBER:
            {
                char* temp = newTemp(0);  // Temporary value
                char tac_line[100];
                sprintf(tac_line, "%s = %d", temp, node->value);
                generateTACLine(tac_line);
            }
            break;
        default:
            fprintf(stderr, "Error: Unknown node type %d in semantic analysis\n", node->type);
            exit(1);
    }
}

void performSemanticAnalysis(ASTNode* root) {
    printf("DEBUG: Starting semantic analysis\n");
    init_symbol_table();

    tac_file = fopen("output.tac", "w");
    if (tac_file == NULL) {
        fprintf(stderr, "Error opening output.tac file\n");
        exit(1);
    }

    analyzeNode(root);

    fclose(tac_file);
}