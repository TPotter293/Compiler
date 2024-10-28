#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"

const char* typeToString(NodeType type) {
    switch (type) {
        case NODE_TYPE_PROGRAM: return "program";
        case NODE_TYPE_STATEMENT: return "statement";
        case NODE_TYPE_DECLARATION: return "declaration";
        case NODE_TYPE_ASSIGNMENT: return "assignment";
        case NODE_TYPE_WRITE: return "write";
        case NODE_TYPE_IF: return "if";
        case NODE_TYPE_RETURN: return "return";
        case NODE_TYPE_INTEGER: return "int";
        case NODE_TYPE_FLOAT: return "float";
        case NODE_TYPE_IDENTIFIER: return "identifier";
        case NODE_TYPE_BOOLEAN: return "boolean";
        case NODE_TYPE_BINARY_OP: return "binary operation";
        case NODE_TYPE_UNARY_OP: return "unary operation";
        case NODE_TYPE_VARIABLE_DECLARATION: return "variable declaration";
        case NODE_TYPE_FUNCTION_DECLARATION: return "function declaration";
        case NODE_TYPE_PARAMETER: return "parameter";
        case NODE_TYPE_PARAMETERS: return "parameters";
        case NODE_TYPE_ARRAY_DECLARATION: return "array declaration";
        case NODE_TYPE_ARRAY_ACCESS: return "array access";
        case NODE_TYPE_ARRAY_ASSIGNMENT: return "array assignment";
        default: return "unknown";
    }
}

// Helper function to allocate a new node
ASTNode* createNode() {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    // Initialize all fields
    node->type = NODE_TYPE_UNDEFINED; // Set to a default or undefined value
    node->op = NULL;                   // Initialize operator to NULL
    node->value.intValue = 0; 
    node->value.floatValue = 0.0;                  // Set default value (could be another sentinel value)
    node->id = NULL;                   // Initialize identifier to NULL
    node->left = NULL;                 // Initialize left child
    node->right = NULL;                // Initialize right child
    node->temp_var = -1;               // Default for temp_var (indicate uninitialized)
    node->temp_var_name = NULL;        // Initialize temporary variable name to NULL
    node->boolean_val = NULL;          // Initialize boolean value to NULL
    node->arrayIndex = NULL;           // Initialize array index to NULL
    node->assignedValue = NULL;        // Initialize assigned value to NULL

    // Initialize union fields appropriately
    node->statements.count = 0;        // Initialize statement count
    node->varDecl.arraySize = 0;       // Default array size
    node->parameters.count = 0;        // Initialize parameter count
    node->funcDecl.identifier = NULL;  // Initialize funcDecl fields to NULL
    node->funcDecl.parameters = NULL;
    node->funcDecl.returnType = NULL;
    node->funcDecl.body = NULL;
    node->param.identifier = NULL;      // Initialize param fields to NULL
    node->param.paramType = NULL;

    return node;
}


// Create a node for statements
ASTNode* createStatementsNode(ASTNode** stmts, int count) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_STATEMENT;  // Set node type
    node->statements.stmts = (ASTNode**)malloc(count * sizeof(ASTNode*)); // Allocate memory for statement pointers
    for (int i = 0; i < count; ++i) {
        node->statements.stmts[i] = stmts[i];  // Copy pointers
    }
    node->statements.count = count;
    return node;
}

// Create a program node with statements
ASTNode* createProgramNode(ASTNode* stmtList) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_PROGRAM;
    node->statements = stmtList->statements;
    printf("DEBUG: Created program node with %d statements\n", node->statements.count);
    free(stmtList);  // Free the original stmtList node as we've moved its content
    return node;
}

// Create a declaration node with an identifier
ASTNode* createDeclarationNode(ASTNode* type, ASTNode* id) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_DECLARATION; // Set node type
    node->left = type;  // Type goes to left child 
    node->right = id;   // Identifier goes to right child
    return node;
}

// Create an assignment node linking the identifier and expression
ASTNode* createAssignmentNode(ASTNode* id, ASTNode* expr) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_ASSIGNMENT; // Set node type
    node->left = id;  // Identifier as left child
    node->right = expr; // Expression as right child
    return node;
}

// Create a write node
ASTNode* createWriteNode(ASTNode* expr) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_WRITE; // Set node type
    node->left = expr; // Assuming expression to write is stored in left
    return node;
}

// Create an if node
ASTNode* createIfNode(ASTNode* cond, ASTNode* thenStmt, ASTNode* elseStmt) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_IF; // Set node type
    node->left = cond; // Condition stored in left
    node->right = thenStmt; // Then statement stored in right
    // Assuming elseStmt would be handled differently or stored in another field if needed
    return node;
}

// Create a return node
ASTNode* createReturnNode(ASTNode* expr) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_RETURN; // Set node type
    node->left = expr; // Expression to return stored in left
    return node;
}

// Create a number node
ASTNode* createIntegerNode(int value) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_INTEGER; // Set node type
    node->value.intValue = value;
    return node;
}

ASTNode* createFloatNode(float value) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_FLOAT; // Set node type
    node->value.floatValue = value; // Store float value
    return node;
}

// Create an identifier node
ASTNode* createIdentifierNode(char* id) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_IDENTIFIER; // Set node type
    node->id = strdup(id);  // Duplicate string
    return node;
}

// Create boolean node
ASTNode* createBooleanNode(char* value) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_BOOLEAN;  // Set node type to boolean
    node->boolean_val = strdup(value);  // Store boolean value as a string
    return node;
}

// Create binary operation node with its left and right children
ASTNode* createBinaryOpNode(char* op, ASTNode* left, ASTNode* right) {
    printf("DEBUG: Creating binary op node with op '%s'\n", op ? op : "NULL");
    ASTNode* node = createNode();
    node->type = NODE_TYPE_BINARY_OP; // Set node type
    if (op) {
        node->op = strdup(op);
    }
    node->left = left;  // Set left operand
    node->right = right; // Set right operand
    return node;
}

// Create a unary operation node
ASTNode* createUnaryOpNode(ASTNode* expr) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_UNARY_OP; // Set node type
    node->left = expr; // Assuming unary operation uses a single expression
    return node;
}

ASTNode* createVariableDeclarationNode(ASTNode* identifier, ASTNode* type) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_VARIABLE_DECLARATION;
    node->varDecl.identifier = identifier;
    node->varDecl.varType = type;
    return node;
}

ASTNode* createFunctionDeclarationNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType, ASTNode* body) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_FUNCTION_DECLARATION;
    node->funcDecl.identifier = identifier;
    node->funcDecl.parameters = parameters;
    node->funcDecl.returnType = returnType;
    node->funcDecl.body = body;
    return node;
}

ASTNode* createParameterNode(ASTNode* identifier, ASTNode* type) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_PARAMETER;
    node->param.identifier = identifier;
    node->param.paramType = type;
    return node;
}

ASTNode* createParametersNode(ASTNode* parameter, int count) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_PARAMETERS;
    node->parameters.params = malloc(sizeof(ASTNode*) * count);
    node->parameters.params[0] = parameter;
    node->parameters.count = count;
    return node;
}

ASTNode* createArrayDeclarationNode(ASTNode* identifier, char* typeNode, int arraySize) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_ARRAY_DECLARATION;
    node->id = identifier;        // Identifier (variable name)
    node->varDecl.varType = typeNode;     // Type (int, float, etc.)
    node->varDecl.arraySize = arraySize;  // Array size

    return node;
}

ASTNode* createArrayAccessNode(ASTNode* identifier, ASTNode* indexNode) {
    ASTNode* node = createNode(); 
    node->type = NODE_TYPE_ARRAY_ACCESS;
    node->id  = identifier;        // Identifier (array name)
    node->value.intValue = indexNode;    // Index to access

    return node;
}

ASTNode* createArrayAssignmentNode(char* id, ASTNode* index, ASTNode* value) {
    ASTNode* node = createNode();  // Create a new node
    node->type = NODE_TYPE_ARRAY_ASSIGNMENT;  // Set the node type
    node->id = strdup(id);  // Store the identifier for the array
    node->arrayIndex = index;  // Store the index expression
    node->assignedValue = value;  // Store the value to assign
    return node;  // Return the created node
}

// Recursive function to print the AST
void printAST(ASTNode* node, int indentLevel) {
    if (node == NULL) {
        printf("DEBUG: Encountered NULL node at indent level %d\n", indentLevel);
        return;
    }

    printf("DEBUG: Processing node at indent level %d\n", indentLevel);
    printf("DEBUG: Node address: %p\n", (void*)node);

    // Print indentation for current level
    for (int i = 0; i < indentLevel; i++) {
        printf("  ");
    }

    // Identify the type of node and print relevant information
    switch (node->type) {
        case NODE_TYPE_PROGRAM:
            printf("Program:\n");
            for (int i = 0; i < node->statements.count; i++) {
                printAST(node->statements.stmts[i], indentLevel + 1);
            }
            return; // Return here to avoid processing left and right children        case NODE_TYPE_STATEMENT:
            printf("DEBUG: Node type: Statement\n");
            printf("DEBUG: Processing %d statements\n", node->statements.count);
            for (int i = 0; i < node->statements.count; i++) {
                printAST(node->statements.stmts[i], indentLevel + 1);
            }
            break;
        case NODE_TYPE_DECLARATION:
            printf("DEBUG: Node type: Declaration\n");
            break;
        case NODE_TYPE_ASSIGNMENT:
            printf("DEBUG: Node type: Assignment\n");
            break;
        case NODE_TYPE_WRITE:
            printf("DEBUG: Node type: Write\n");
            break;
        case NODE_TYPE_IF:
            printf("DEBUG: Node type: If\n");
            break;
        case NODE_TYPE_RETURN:
            printf("DEBUG: Node type: Return\n");
            break;
        case NODE_TYPE_INTEGER:
            printf("DEBUG: Node type: integer\n");
            printf("DEBUG: Integer value: %d\n", node->value.intValue);
            break;
        case NODE_TYPE_FLOAT:
            printf("DEBUG: Node type: Float\n");
            printf("DEBUG: Float value: %f\n", node->value.floatValue);
            break;
        case NODE_TYPE_IDENTIFIER:
            printf("DEBUG: Node type: Identifier\n");
            printf("DEBUG: Identifier value: %s\n", node->id);
            break;
        case NODE_TYPE_BOOLEAN:  // Handle boolean nodes
            printf("DEBUG: Node type: Boolean\n");
            printf("DEBUG: Boolean value: %s\n", node->value.intValue ? "true" : "false");
            break;
        case NODE_TYPE_BINARY_OP:
            printf("DEBUG: Node type: Binary Operator\n");
            printf("DEBUG: Operator value: %s\n", node->op);
            break;
        case NODE_TYPE_UNARY_OP:
            printf("DEBUG: Node type: Unary Operator\n");
            break;
        case NODE_TYPE_ARRAY_DECLARATION:
            printf("DEBUG: Node type: Array Declaration\n");
            printf("DEBUG: Identifier value: %s\n", node->id);
            printf("DEBUG: Type: %s\n", node->varDecl.varType);
            printf("DEBUG: Array size: %d\n", node->varDecl.arraySize);
            break;
        case NODE_TYPE_ARRAY_ASSIGNMENT:
            printf("DEBUG: Node type: Array Assignment\n");
            printf("DEBUG: Array identifier: %s\n", node->id);
            printf("DEBUG: Index:\n");
            printAST(node->arrayIndex, indentLevel + 1);
            printf("DEBUG: Value to assign:\n");
            printAST(node->assignedValue, indentLevel + 1);
            break;
        case NODE_TYPE_ARRAY_ACCESS:
            printf("DEBUG: Node type: Array Access\n");
            printf("DEBUG: Array identifier: %s\n", node->id);
            printf("DEBUG: Index:\n");
            printAST(node->arrayIndex, indentLevel + 1);
            break;
        default:
            printf("DEBUG: Node type is unknown or uninitialized\n");
    }

    // Print left child
    printf("DEBUG: Processing left child\n");
    if (node->left) {
        printAST(node->left, indentLevel + 1);
    } else {
        printf("DEBUG: Left child is NULL\n");
    }

    // Print right child
    printf("DEBUG: Processing right child\n");
    if (node->right) {
        printAST(node->right, indentLevel + 1);
    } else {
        printf("DEBUG: Right child is NULL\n");
    }

    printf("DEBUG: Finished processing node at indent level %d\n", indentLevel);
}


void freeASTNode(ASTNode* node) {
    if (node == NULL) return;  // Base case: nothing to free

    // Recursively free children
    freeASTNode(node->left);
    freeASTNode(node->right);
    
    // Free dynamically allocated fields
    free(node->op);                 // Free operator string
    free(node->id);                 // Free identifier string
    free(node->temp_var_name);      // Free temporary variable name string
    free(node->boolean_val);        // Free boolean value string
    freeASTNode(node->arrayIndex);  // Free array index node
    freeASTNode(node->assignedValue); // Free assigned value node

    // Free union members if they point to dynamic memory
    // (Since the union does not require explicit freeing if they are NULL, skip this)

    // Free the node itself
    free(node);
}
