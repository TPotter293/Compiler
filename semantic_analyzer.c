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
        char* temp = newTemp();
        sprintf(tac_line, "%s = %d", temp, result);
        printf("DEBUG: Constant Folding TAC -> %s\n", tac_line);
        generateTACLine(tac_line);
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
    analyzeNode(node->right);  // Analyze the right-hand side to get temp_var_name

    char tac_line[100];
    sprintf(tac_line, "%s = %s", node->left->id, node->right->temp_var_name);
    generateTACLine(tac_line);

    node->left->temp_var_name = node->right->temp_var_name;  // Assign temp_var_name to the left-hand side
    updateIdToTemp(node->left->id, getIdIndex(node->right->temp_var_name));

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

    if (node->left->temp_var_name == NULL || node->right->temp_var_name == NULL) {
        fprintf(stderr, "Error: Uninitialized variable in binary operation\n");
        exit(1);
    }

    sprintf(tac_line, "%s = %s %s %s", temp, node->left->temp_var_name, node->op, node->right->temp_var_name);
    printf("DEBUG: Binary Operation TAC -> %s\n", tac_line);
    generateTACLine(tac_line);

    node->temp_var_name = temp;

    int temp_var_index = getIdIndex(temp);
    updateIdToTemp(temp, temp_var_index);
}


void analyzeIdentifier(ASTNode* node) {
    int index = getIdIndex(node->id);
    if (index != -1) {
        node->temp_var_name = id_to_temp[index].name;
    } else {
        // Initialize uninitialized variables with a default value (e.g., 0)
        char* temp = newTemp();
        char tac_line[100];
        sprintf(tac_line, "%s = 0", temp);
        generateTACLine(tac_line);
        node->temp_var_name = temp;
        updateIdToTemp(node->id, getIdIndex(temp));
        fprintf(stderr, "Warning: Initializing uninitialized variable %s to 0\n", node->id);
    }
}

// Function to analyze function declarations
void analyzeFunctionDeclaration(ASTNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: NULL node in function declaration analysis\n");
        return;
    }

    // Check that the node is indeed a function declaration
    if (node->type != NODE_TYPE_FUNCTION_DECLARATION) {
        fprintf(stderr, "Error: Node is not a function declaration\n");
        return;
    }

    // Example checks (you can expand these based on your language's semantics)
    // Check the function identifier
    if (node->id == NULL) {
        fprintf(stderr, "Error: Function declaration missing identifier\n");
        return;
    }

    // Check parameters
    if (node->left != NULL) {
        // Analyze parameters (node->left should be the parameters node)
        analyzeNode(node->left);
    }

    // Check return type
    if (node->right != NULL) {
        // Analyze return type (node->right should be the return type node)
        analyzeNode(node->right);
    }

    // Check function body
    if (node->statements.count > 0) {
        for (int i = 0; i < node->statements.count; i++) {
            analyzeNode(node->statements.stmts[i]);
        }
    }
}

void analyzeParameters(ASTNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: NULL node in parameters analysis\n");
        return;
    }

    // Check that the node is indeed a parameters node
    if (node->type != NODE_TYPE_PARAMETERS) {
        fprintf(stderr, "Error: Node is not a parameters node\n");
        return;
    }

    // Iterate over each parameter and analyze it
    for (int i = 0; i < node->parameters.count; i++) {
        ASTNode* param = node->parameters.params[i];
        if (param != NULL) {
            analyzeNode(param);
        } else {
            fprintf(stderr, "Error: NULL parameter at index %d\n", i);
        }
    }
}

void analyzeParameter(ASTNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: NULL node in parameter analysis\n");
        return;
    }

    // Check that the node is indeed a parameter node
    if (node->type != NODE_TYPE_PARAMETER) {
        fprintf(stderr, "Error: Node is not a parameter node\n");
        return;
    }

    // Check the parameter identifier
    if (node->param.identifier == NULL || node->param.identifier->id == NULL) {
        fprintf(stderr, "Error: Parameter missing identifier\n");
        return;
    }

    // Check the parameter type
    if (node->param.paramType == NULL || node->param.paramType->id == NULL) {
        fprintf(stderr, "Error: Parameter missing type\n");
        return;
    }

    // Additional checks can be added here as needed
}

void analyzeReturn(ASTNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: NULL node in return analysis\n");
        return;
    }

    // Check that the node is indeed a return node
    if (node->type != NODE_TYPE_RETURN) {
        fprintf(stderr, "Error: Node is not a return node\n");
        return;
    }

    // Analyze the expression being returned
    if (node->left != NULL) {
        analyzeNode(node->left);
    } else {
        fprintf(stderr, "Error: Return statement missing expression\n");
    }

    // Additional checks can be added here as needed
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
            // Existing logic for numbers
            break;
        case NODE_TYPE_FUNCTION_DECLARATION:
            analyzeFunctionDeclaration(node);
            break;
        case NODE_TYPE_PARAMETERS:
            analyzeParameters(node);
            break;
        case NODE_TYPE_PARAMETER:
            analyzeParameter(node);
            break;
        case NODE_TYPE_RETURN:
            analyzeReturn(node);
            break;
        // Add cases for other node types as needed
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
