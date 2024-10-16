#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#define DEBUG_MEMORY(msg, ptr) printf("DEBUG MEMORY: %s %p\n", msg, (void*)ptr)

// Helper function to allocate a new node
ASTNode* createNode() {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    DEBUG_MEMORY("Allocated node", node);
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed in createNode\n");
        return NULL;
    }
    node->left = NULL;
    node->right = NULL;
    node->op = NULL;
    node->id = NULL;
    node->value = 0;
    node->statements.count = 0;
    node->statements.stmts = NULL; // Initialize statements array
    return node;
}

// Create a node for statements
ASTNode* createStatementsNode(ASTNode** stmts, int count) {
    ASTNode* node = createNode();
    if (node == NULL) return NULL;
    node->type = NODE_TYPE_STATEMENT;
    node->statements.stmts = (ASTNode**)malloc(count * sizeof(ASTNode*));
    DEBUG_MEMORY("Allocated statements array", node->statements.stmts);
    if (node->statements.stmts == NULL) {
        fprintf(stderr, "Memory allocation failed for statements array\n");
        free(node);
        return NULL;
    }
    for (int i = 0; i < count; ++i) {
        node->statements.stmts[i] = stmts[i];
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

// Create a function prototype node
ASTNode* createFunctionPrototypeNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType) {
    ASTNode* node = createNode();
    if (!node) {
        printf("Memory allocation failed for function prototype node\n");
        exit(1);
    }
    node->type = NODE_TYPE_FUNCTION_PROTOTYPE;  // Set the correct node type
    node->funcProto.identifier = identifier;
    node->funcProto.parameters = parameters;
    node->funcProto.returnType = returnType;

    printf("DEBUG: Function prototype node created with identifier: %s\n", identifier->id);

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
ASTNode* createNumberNode(int value) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_NUMBER; // Set node type
    node->value = value;
    return node;
}

// Create an identifier node
ASTNode* createIdentifierNode(char* id) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_IDENTIFIER; // Set node type
    node->id = strdup(id);  // Duplicate string
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

// Create a variable declaration node
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
    node->id = strdup(identifier->id);  // Make a copy of the function name
    node->left = parameters;
    node->right = returnType;
    
    // Deep copy the statements from the body
    node->statements.count = body->statements.count;
    node->statements.stmts = malloc(sizeof(ASTNode*) * node->statements.count);
    for (int i = 0; i < node->statements.count; i++) {
        node->statements.stmts[i] = body->statements.stmts[i];
    }
    
    // Free the temporary body node
    free(body);
    
    return node;
}



// Add a new function for debugging AST nodes
void debugPrintNode(ASTNode* node, const char* message) {
    if (node == NULL) {
        printf("DEBUG: %s is NULL\n", message);
        return;
    }
    printf("DEBUG: %s - Type: %d, Address: %p\n", message, node->type, (void*)node);
}

// Create a parameter node
ASTNode* createParameterNode(ASTNode* identifier, ASTNode* type) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_PARAMETER;
    node->param.identifier = identifier;
    node->param.paramType = type;
    return node;
}

// Create a parameters node
ASTNode* createParametersNode(ASTNode* parameter, int count) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_PARAMETERS;
    node->parameters.params = (ASTNode**)malloc(sizeof(ASTNode*) * count);
    if (node->parameters.params == NULL) {
        fprintf(stderr, "Memory allocation failed for parameters array\n");
        return NULL;
    }
    node->parameters.params[0] = parameter;
    node->parameters.count = count;
    return node;
}

// Recursive function to print the AST
void printAST(ASTNode* node, int indentLevel) {
    if (node == NULL) {
        printf("DEBUG: Encountered NULL node at indent level %d\n", indentLevel);
        return;
    }

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
            return;
        case NODE_TYPE_STATEMENT:
            printf("Statement node\n");
            for (int i = 0; i < node->statements.count; i++) {
                printAST(node->statements.stmts[i], indentLevel + 1);
            }
            break;
        case NODE_TYPE_DECLARATION:
            printf("Declaration node\n");
            break;
        case NODE_TYPE_ASSIGNMENT:
            printf("Assignment node\n");
            break;
        case NODE_TYPE_WRITE:
            printf("Write node\n");
            break;
        case NODE_TYPE_IF:
            printf("If node\n");
            break;
        case NODE_TYPE_RETURN:
            printf("Return node\n");
            break;
        case NODE_TYPE_NUMBER:
            printf("Number node: %d\n", node->value);
            break;
        case NODE_TYPE_IDENTIFIER:
            printf("Identifier node: %s\n", node->id);
            break;
        case NODE_TYPE_BINARY_OP:
            printf("Binary Op node: %s\n", node->op);
            break;
        case NODE_TYPE_UNARY_OP:
            printf("Unary Op node\n");
            break;
        default:
            printf("Unknown or uninitialized node\n");
    }

    // Process left and right children
    if (node->left) {
        printAST(node->left, indentLevel + 1);
    }
    if (node->right) {
        printAST(node->right, indentLevel + 1);
    }
}

// Free the AST
void freeASTNode(ASTNode* node) {
    if (node == NULL) return;
    DEBUG_MEMORY("Freeing node", node);

    if (node->op) free(node->op);
    if (node->id) free(node->id);
    if (node->statements.stmts) {
        for (int i = 0; i < node->statements.count; i++) {
            freeASTNode(node->statements.stmts[i]);
        }
        free(node->statements.stmts);
    }
    freeASTNode(node->left);
    freeASTNode(node->right);
    free(node);
}
