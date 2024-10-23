#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#define DEBUG_MEMORY(msg, ptr) printf("DEBUG MEMORY: %s %p\n", msg, (void*)ptr)

// Add this line
void freeNode(ASTNode* node);
void debugPrintFunctionNode(ASTNode* node);


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
    if (!node) return NULL;
    
    node->type = NODE_TYPE_STATEMENT;
    if (count > 0) {
        node->statements.stmts = malloc(count * sizeof(ASTNode*));
        if (!node->statements.stmts) {
            free(node);
            return NULL;
        }
        
        for (int i = 0; i < count; i++) {
            node->statements.stmts[i] = stmts[i];
        }
        node->statements.count = count;
    } else {
        node->statements.stmts = NULL;
        node->statements.count = 0;
    }
    
    return node;
}


// Create a program node with statements
ASTNode* createProgramNode(ASTNode* stmtList) {
    printf("DEBUG: Entering createProgramNode\n");
    ASTNode* node = createNode();
    node->type = NODE_TYPE_PROGRAM;
    node->statements = stmtList->statements;

    // Do not free stmtList here if you're still using it
    free(stmtList);  // Free stmtList after transferring ownership
    stmtList = NULL; // Prevent accidental use after freeing

    printf("DEBUG: Exiting createProgramNode\n");
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

// Function declaration node creation with body management
ASTNode* createFunctionDeclarationNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType, ASTNode* body) {
    printf("DEBUG: Entering createFunctionDeclarationNode\n");
    
    ASTNode* node = createNode();
    node->type = NODE_TYPE_FUNCTION_DECLARATION;
    node->id = strdup(identifier->id);
    node->left = parameters;
    node->right = returnType;
    
    if (body && body->statements.count > 0) {
        node->statements.count = body->statements.count;
        node->statements.stmts = malloc(sizeof(ASTNode*) * node->statements.count);
        
        // Deep copy each statement pointer
        for (int i = 0; i < body->statements.count; i++) {
            node->statements.stmts[i] = body->statements.stmts[i];
            printf("DEBUG: Copied statement %d from %p to new location\n", 
                   i, (void*)body->statements.stmts[i]);
        }
        
        // Don't free the body node here - it will be freed later
        body->statements.stmts = NULL;
        body->statements.count = 0;
    }
    
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

// Add the new debug function here
void debugPrintFunctionNode(ASTNode* node) {
    printf("DEBUG: Function Node Details:\n");
    printf("Address: %p\n", (void*)node);
    printf("Type: %d\n", node->type);
    printf("ID: %s\n", node->id ? node->id : "NULL");
    printf("Statements count: %d\n", node->statements.count);
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
ASTNode* createParametersNode(ASTNode** params, int count) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_PARAMETERS;
    node->statements.count = count;
    node->statements.stmts = malloc(sizeof(ASTNode*) * count);
    if (node->statements.stmts == NULL) {
        fprintf(stderr, "Memory allocation failed for parameters array\n");
        free(node);
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        node->statements.stmts[i] = params[i];
    }
    return node;
}

void printAST(ASTNode* node, int indentLevel) {
    if (node == NULL) return;

    for (int i = 0; i < indentLevel; i++) {
        printf("  ");  // Indentation for readability
    }

    switch (node->type) {
        case NODE_TYPE_PROGRAM:
            printf("Program Node\n");
            break;
        // Add cases for each node type
        default:
            printf("Unknown Node Type\n");
            break;
    }

    if (node->left) printAST(node->left, indentLevel + 1);
    if (node->right) printAST(node->right, indentLevel + 1);
}

void freeNode(ASTNode* node) {
    if (!node) return;

    printf("DEBUG: Starting to free node of type %d at %p\n", node->type, (void*)node);

    // Free child nodes first
    if (node->left) {
        printf("DEBUG: Freeing left child at %p\n", (void*)node->left);
        freeNode(node->left);
        node->left = NULL;
    }
    
    if (node->right) {
        printf("DEBUG: Freeing right child at %p\n", (void*)node->right);
        freeNode(node->right);
        node->right = NULL;
    }

    // Free statements array if it exists
    if (node->statements.stmts) {
        printf("DEBUG: Freeing statements of node\n");
        for (int i = 0; i < node->statements.count; i++) {
            if (node->statements.stmts[i]) {
                printf("DEBUG: Freeing statement %d at %p\n", i, (void*)node->statements.stmts[i]);
                freeNode(node->statements.stmts[i]);
                node->statements.stmts[i] = NULL;
            }
        }
        free(node->statements.stmts);
        node->statements.stmts = NULL;
    }

    // Free string members
    if (node->id) {
        printf("DEBUG: Freeing node id: %s\n", node->id);
        free(node->id);
        node->id = NULL;
    }
    
    if (node->op) {
        printf("DEBUG: Freeing node op: %s\n", node->op);
        free(node->op);
        node->op = NULL;
    }

    free(node);
    printf("DEBUG: Freed node at %p\n", (void*)node);
}


void freeAST(ASTNode* root) {
    if (!root) return;

    printf("DEBUG: Starting AST cleanup for node type %d at %p\n", root->type, (void*)root);
    
    // Track statement ownership before cleanup
    if (root->statements.count > 0) {
        printf("DEBUG: Node %p has %d statements\n", (void*)root, root->statements.count);
        for (int i = 0; i < root->statements.count; i++) {
            printf("DEBUG: Statement %d at %p with type %d\n", 
                   i, (void*)root->statements.stmts[i], 
                   root->statements.stmts[i]->type);
        }
    }

    // Track child nodes
    if (root->left) {
        printf("DEBUG: Processing left child at %p\n", (void*)root->left);
        freeAST(root->left);
    }
    if (root->right) {
        printf("DEBUG: Processing right child at %p\n", (void*)root->right);
        freeAST(root->right);
    }
    
    printf("DEBUG: Freeing node %p with type %d\n", (void*)root, root->type);
    freeNode(root);
}


