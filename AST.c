#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"

// Helper function to allocate a new node
ASTNode* createNode() {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
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

ASTNode* createVariableDeclarationNode(ASTNode* identifier, ASTNode* type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_TYPE_VARIABLE_DECLARATION;
    node->varDecl.identifier = identifier;
    node->varDecl.varType = type;
    return node;
}

ASTNode* createFunctionDeclarationNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_TYPE_FUNCTION_DECLARATION;
    node->funcDecl.identifier = identifier;
    node->funcDecl.parameters = parameters;
    node->funcDecl.returnType = returnType;
    node->funcDecl.body = body;
    return node;
}

ASTNode* createParameterNode(ASTNode* identifier, ASTNode* type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_TYPE_PARAMETER;
    node->param.identifier = identifier;
    node->param.paramType = type;
    return node;
}

ASTNode* createParametersNode(ASTNode* parameter, int count) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_TYPE_PARAMETERS;
    node->parameters.params = malloc(sizeof(ASTNode*) * count);
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
        case NODE_TYPE_NUMBER:
            printf("DEBUG: Node type: Number\n");
            printf("DEBUG: Number value: %d\n", node->value);
            break;
        case NODE_TYPE_IDENTIFIER:
            printf("DEBUG: Node type: Identifier\n");
            printf("DEBUG: Identifier value: %s\n", node->id);
            break;
        case NODE_TYPE_BINARY_OP:
            printf("DEBUG: Node type: Binary Operator\n");
            printf("DEBUG: Operator value: %s\n", node->op);
            break;
        case NODE_TYPE_UNARY_OP:
            printf("DEBUG: Node type: Unary Operator\n");
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
