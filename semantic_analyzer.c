#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#include "symbol_table.h"

FILE* tac_file;
int temp_var_count = 0;

#define MAX_IDENTIFIERS 100
struct {
    char name[20];
    int temp_var;
} id_to_temp[MAX_IDENTIFIERS];
int id_to_temp_count = 0;

char* newTemp() {
    char* temp = (char*)malloc(10);
    sprintf(temp, "t%d", temp_var_count++);
    return temp;
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
            // Generate TAC for the right-hand side first
            generateTAC(node->right);  
            generateTAC(node->left);  // Make sure to evaluate the left identifier
            
            // Ensure the right side produces a temp variable
            if (node->right->temp_var_name != NULL) {
                sprintf(tac_line, "%s = %s", node->left->id, node->right->temp_var_name);
                printf("DEBUG: Assignment -> %s\n", tac_line);
                generateTACLine(tac_line);
            } else {
                fprintf(stderr, "Error: Right-hand side of assignment does not produce a temp variable.\n");
            }
            break;
        case NODE_TYPE_WRITE:
            // Ensure that we're writing the identifier directly
            sprintf(tac_line, "print %s", node->left->id);
            printf("DEBUG: Write -> %s\n", tac_line);
            generateTACLine(tac_line);
            break;
        case NODE_TYPE_BINARY_OP:
            // Generate TAC for left and right operands first
            generateTAC(node->left);
            generateTAC(node->right);

            // Check if both operands are constants
            if (node->left->type == NODE_TYPE_NUMBER && node->right->type == NODE_TYPE_NUMBER) {
                // Perform constant folding
                int result = 0;
                if (strcmp(node->op, "+") == 0) {
                    result = node->left->value + node->right->value;
                } else if (strcmp(node->op, "-") == 0) {
                    result = node->left->value - node->right->value;
                } else if (strcmp(node->op, "*") == 0) {
                    result = node->left->value * node->right->value;
                } else if (strcmp(node->op, "/") == 0) {
                    result = node->left->value / node->right->value;
                }

                // Create a new temporary variable for the result
                char* temp = newTemp(); // Function to generate new temporary variable names
                sprintf(tac_line, "%s = %d", temp, result);
                printf("DEBUG: Constant Folding TAC -> %s\n", tac_line);
                generateTACLine(tac_line);
                node->temp_var_name = temp; // Store temp variable for further use
            } else {
                // Create a new temporary variable for the result of the binary operation
                char* temp = newTemp();
                sprintf(tac_line, "%s = %s %s %s", temp, node->left->temp_var_name, node->op, node->right->temp_var_name);
                printf("DEBUG: Binary Operation TAC -> %s\n", tac_line);
                generateTACLine(tac_line);
                node->temp_var_name = temp; // Store temp variable for further use
            }
            break;
        case NODE_TYPE_IDENTIFIER:
            // Only store temp_var for identifiers in assignments or operations
            break;
        case NODE_TYPE_NUMBER:
            char* num_temp = newTemp();
            sprintf(tac_line, "%s = %d", num_temp, node->value);
            printf("DEBUG: Number -> %s\n", tac_line);
            generateTACLine(tac_line);
            node->temp_var_name = num_temp; // Store the temp variable name for further use
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
            return;
        }
    }
    if (id_to_temp_count < MAX_IDENTIFIERS) {
        strcpy(id_to_temp[id_to_temp_count].name, id);
        id_to_temp[id_to_temp_count].temp_var = temp_var;
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
    
    char tac_line[100];
    sprintf(tac_line, "%s = %s", node->left->id, node->right->temp_var_name);
    generateTACLine(tac_line);
}

void analyzeWrite(ASTNode* node) {
    analyzeNode(node->left);

    char tac_line[100];
    sprintf(tac_line, "print %s", node->left->id);
    generateTACLine(tac_line);
}

void analyzeBinaryOp(ASTNode* node) {
    analyzeNode(node->left);
    analyzeNode(node->right);

    char* temp = newTemp();
    char tac_line[100];

    sprintf(tac_line, "%s = %s %s %s", temp, node->left->temp_var_name, node->op, node->right->temp_var_name);
    printf("DEBUG: Binary Operation TAC -> %s\n", tac_line);
    generateTACLine(tac_line);

    node->temp_var_name = temp;
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
                char* temp = newTemp();
                char tac_line[100];
                sprintf(tac_line, "%s = %d", temp, node->value);
                generateTACLine(tac_line);
                node->temp_var_name = temp;
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
        perror("Error opening TAC output file");
        exit(EXIT_FAILURE);
    }

    analyzeNode(root);

    if (fclose(tac_file) != 0) {
        perror("Error closing TAC output file");
        exit(EXIT_FAILURE);
    }

    printf("TAC generation and semantic analysis completed successfully.\n");
}
