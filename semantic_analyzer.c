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

char* newFloat() {
    char* temp = (char*)malloc(10);
    sprintf(temp, "f%d", temp_var_count++);
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

            char* temp;

            // Check if both operands are constants
            if (node->left->type == NODE_TYPE_INTEGER && node->right->type == NODE_TYPE_INTEGER) {
                // Perform constant folding for integers
                temp = newTemp();
                int result = 0;
                if (strcmp(node->op, "+") == 0) {
                    result = node->left->value.intValue + node->right->value.intValue;
                } else if (strcmp(node->op, "-") == 0) {
                    result = node->left->value.intValue - node->right->value.intValue;
                } else if (strcmp(node->op, "*") == 0) {
                    result = node->left->value.intValue * node->right->value.intValue;
                } else if (strcmp(node->op, "/") == 0) {
                    result = node->left->value.intValue / node->right->value.intValue;
                }

                // Create a new temporary variable for the result
                sprintf(tac_line, "%s = %d", temp, result);
                printf("DEBUG: Constant Folding TAC -> %s\n", tac_line);
                generateTACLine(tac_line);
            } else if (node->left->type == NODE_TYPE_FLOAT && node->right->type == NODE_TYPE_FLOAT) {
                // Perform constant folding for floats
                temp = newFloat();
                float result = 0.0f;
                if (strcmp(node->op, "+") == 0) {
                    result = node->left->value.floatValue + node->right->value.floatValue;
                } else if (strcmp(node->op, "-") == 0) {
                    result = node->left->value.floatValue - node->right->value.floatValue;
                } else if (strcmp(node->op, "*") == 0) {
                    result = node->left->value.floatValue * node->right->value.floatValue;
                } else if (strcmp(node->op, "/") == 0) {
                    result = node->left->value.floatValue / node->right->value.floatValue;
                }

                // Create a new temporary variable for the result
                sprintf(tac_line, "%s = %f", temp, result);
                printf("DEBUG: Constant Folding TAC -> %s\n", tac_line);
                generateTACLine(tac_line);
            } else {
                temp = newFloat();
                // Create a new temporary variable for the result of the binary operation
                sprintf(tac_line, "%s = %s %s %s", temp, node->left->temp_var_name, node->op, node->right->temp_var_name);
                printf("DEBUG: Binary Operation TAC -> %s\n", tac_line);
                generateTACLine(tac_line);
            }
            node->temp_var_name = temp; // Store temp variable for further use
            break;
        case NODE_TYPE_IDENTIFIER:
            // Only store temp_var for identifiers in assignments or operations
            break;
        case NODE_TYPE_INTEGER:
            char* int_temp = newTemp();
            sprintf(tac_line, "%s = %d", int_temp, node->value.intValue);
            printf("DEBUG: Integer -> %s\n", tac_line);
            generateTACLine(tac_line);
            node->temp_var_name = int_temp; // Store the temp variable name for further use
            break;
        case NODE_TYPE_FLOAT:
            char* float_temp = newFloat();
            sprintf(tac_line, "%s = %f", float_temp, node->value.floatValue);
            printf("DEBUG: Float -> %s\n", tac_line);
            generateTACLine(tac_line);
            node->temp_var_name = float_temp;
            break;
        case NODE_TYPE_BOOLEAN:
            char* bool_temp = newTemp();
            int bool_val = 0;
            if (strcmp(node->boolean_val, "true") == 0) {
                bool_val = 1;
            }
            sprintf(tac_line, "%s = %s", bool_temp, bool_val);
            printf("DEBUG: Boolean -> %s\n", tac_line);
            generateTACLine(tac_line);
            node->temp_var_name = bool_temp; 
            break;
        case NODE_TYPE_ARRAY_ACCESS: // Handle array access
            // Assume node->array_id contains the array identifier and node->index contains the index expression
            printf("------------------- index : %d -----------------\n", node->value.intValue);
            generateTAC(node->value.intValue); // Generate TAC for the index
            if (node->value.intValue != NULL) {
                // Create a new temporary variable for accessing the array
                char* temp = newTemp();
                sprintf(tac_line, "%s = %s[%s]", temp, node->id, node->value.intValue);
                printf("DEBUG: Array Access TAC -> %s\n", tac_line);
                generateTACLine(tac_line);
                node->temp_var_name = temp; // Store the temp variable for further use
            } else {
                fprintf(stderr, "Error: Index for array access does not produce a temp variable.\n");
            }
            break;
        case NODE_TYPE_ARRAY_ASSIGNMENT: // Handle array assignments
            // Assume node->array_id contains the array identifier and node->index contains the index expression
            generateTAC(node->value.intValue); // Generate TAC for the index
            generateTAC(node->right); // Generate TAC for the right-hand side (value to assign)

            if (node->value.intValue != NULL && node->right->temp_var_name != NULL) {
                sprintf(tac_line, "%s[%s] = %s", node->id, node->value.intValue, node->right->temp_var_name);
                printf("DEBUG: Array Assignment TAC -> %s\n", tac_line);
                generateTACLine(tac_line);
            } else {
                fprintf(stderr, "Error: Index or value for array assignment does not produce a temp variable.\n");
            }
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
    if (node->left->temp_var_name != NULL) {
        sprintf(tac_line, "print %s", node->left->temp_var_name);
    } else {
        sprintf(tac_line, "print %s", node->left->id);
    }
    generateTACLine(tac_line);
}

void analyzeBinaryOp(ASTNode* node) {
    analyzeNode(node->left);
    analyzeNode(node->right);

    char* temp = newFloat();
    char tac_line[100];

    if (node->left->temp_var_name == NULL || node->right->temp_var_name == NULL) {
        fprintf(stderr, "Error: Uninitialized variable in binary operation\n");
        exit(1);
    }

<<<<<<< Updated upstream
=======
    printf("DEBUG: Left operand value: %s\n", node->left->temp_var_name);
    printf("DEBUG: Right operand value: %s\n", node->right->temp_var_name);

>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
=======

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

    // Check the function identifier
    if (node->id == NULL) {
        fprintf(stderr, "Error: Function declaration missing identifier\n");
        return;
    }

    // Check parameters
    if (node->left != NULL) {
        // Analyze parameters (node->left should be the parameters node)
        analyzeNode(node->left);
    } else {
        fprintf(stderr, "Error: Function declaration missing parameters\n");
        return;
    }

    // Check return type
    if (node->right != NULL && node->right->id != NULL) {
        // Analyze return type (node->right should be the return type node)
        analyzeNode(node->right);
    } else {
        fprintf(stderr, "Error: Function declaration missing return type\n");
        return;
    }

    // Check function body
    if (node->statements.count > 0) {
        for (int i = 0; i < node->statements.count; i++) {
            analyzeNode(node->statements.stmts[i]);
        }
    }

    // Extract parameter types
    char** paramTypes = extractParamTypes(node->left->parameters.params, node->left->parameters.count);
    if (paramTypes == NULL) {
        fprintf(stderr, "Error: Failed to extract parameter types\n");
        return;
    }

    // Insert function into symbol table
    insert_symbol(node->id, "function", paramTypes, node->left->parameters.count, node->right->id);
    freeParamTypes(paramTypes, node->left->parameters.count);

    // Debugging: Print the function details
    printf("DEBUG: Function '%s' with return type '%s' and %d parameters inserted into symbol table.\n",
           node->id, node->right->id, node->left->parameters.count);
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

    if (node->type != NODE_TYPE_RETURN) {
        fprintf(stderr, "Error: Node is not a return node\n");
        return;
    }

    if (node->left != NULL) {
        analyzeNode(node->left);
        if (node->left->temp_var_name != NULL) {
            char tac_line[100];
            sprintf(tac_line, "return %s", node->left->temp_var_name);
            generateTACLine(tac_line);
            printf("DEBUG: Return statement with temp variable %s\n", node->left->temp_var_name);
        } else {
            fprintf(stderr, "Error: Return expression does not produce a temp variable.\n");
        }
    } else {
        fprintf(stderr, "Error: Return statement missing expression\n");
    }
}


void analyzeArgumentList(ASTNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: NULL node in argument list analysis\n");
        return;
    }

    for (int i = 0; i < node->argumentList.count; i++) {
        ASTNode* arg = node->argumentList.args[i];
        analyzeNode(arg);  // Analyze each argument
        if (arg->temp_var_name == NULL) {
            fprintf(stderr, "Error: Argument %d does not produce a temp variable.\n", i);
        }
    }
}

>>>>>>> Stashed changes
void analyzeArrayDeclaration(ASTNode* node) {
    // Array declarations don't output TAC but could be tracked in the symbol table
    printf("DEBUG: Array declaration of %s with type %s and size %d\n", node->id, node->varDecl.varType, node->varDecl.arraySize);
}

void analyzeArrayAccess(ASTNode* node) {
    // First, analyze the index expression to get the temp variable for the index
    analyzeNode(node->value.intValue);

    printf("----------------- index : %d ----------------------\n", node->value.intValue);

    // Create a new temp variable to hold the accessed array value
    char* temp = newTemp();
    char tac_line[100];

    // Generate TAC for array access: temp = array[index]
    sprintf(tac_line, "%s = %s[%d]", temp, node->id, node->value.intValue);
    generateTACLine(tac_line);

    // Store the temp variable name for future use
    node->temp_var_name = temp;

    int temp_var_index = getIdIndex(temp);
    updateIdToTemp(temp, temp_var_index);

    printf("DEBUG: Array Access TAC -> %s\n", tac_line);
}

void analyzeArrayAssignment(ASTNode* node) {
    // First, analyze the index and value expressions
    analyzeNode(node->arrayIndex);
    analyzeNode(node->assignedValue);

    char tac_line[100];

    // Generate TAC for array assignment: array[index] = value
    sprintf(tac_line, "%s[%d] = %s", node->id, node->value.intValue, node->assignedValue->temp_var_name);
    generateTACLine(tac_line);

    printf("DEBUG: Array Assignment TAC -> %s\n", tac_line);
}


void analyzeNode(ASTNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: NULL node passed to analyzeNode\n");
        return; // Handle the error
    }

    printf("DEBUG: analyzeNode node->type = %s\n", typeToString(node->type));

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
<<<<<<< Updated upstream
=======
            printf("DEBUG: Identifier node %s has temp variable %s\n", node->id, node->temp_var_name);
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
        case NODE_TYPE_FUNCTION_CALL:
            analyzeFunctionCall(node);
            break;
        case NODE_TYPE_ARGUMENT_LIST:
            analyzeArgumentList(node);
>>>>>>> Stashed changes
            break;
        case NODE_TYPE_INTEGER:
            char* temp = newTemp();
            char tac_line[100];
            sprintf(tac_line, "%s = %d", temp, node->value.intValue);
            generateTACLine(tac_line);
            node->temp_var_name = temp;

            int temp_var_index = getIdIndex(temp);
            updateIdToTemp(temp, temp_var_index);
            break;
        case NODE_TYPE_FLOAT:
            char* temp2 = newFloat();
            char tac_line2[100];
            sprintf(tac_line2, "%s = %f", temp2, node->value.floatValue);
            generateTACLine(tac_line2);
            node->temp_var_name = temp2;

            int temp_var_index2 = getIdIndex(temp2);
            updateIdToTemp(temp2, temp_var_index2);
            break;
        case NODE_TYPE_BOOLEAN:
            {
                char* temp = newTemp();
                char tac_line[100];

                int bool_val = 0;
                if (strcmp(node->boolean_val, "true") == 0) {
                    bool_val = 1;
                }


                sprintf(tac_line, "%s = %d", temp, bool_val);
                generateTACLine(tac_line);
                node->temp_var_name = temp;

                int temp_var_index = getIdIndex(temp);
                updateIdToTemp(temp, temp_var_index);
            }
            break;
        case NODE_TYPE_ARRAY_DECLARATION:
            analyzeArrayDeclaration(node);
            break;
        case NODE_TYPE_ARRAY_ACCESS:
            analyzeArrayAccess(node);
            break;
        case NODE_TYPE_ARRAY_ASSIGNMENT:
            analyzeArrayAssignment(node);
            break;
        default:
            fprintf(stderr, "Error: Unknown node type %d in semantic analysis\n", node->type);
            exit(1);
    }
}

void performSemanticAnalysis(ASTNode* root) {
    printf("-------------Starting semantic analysis--------------------------\n");
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
