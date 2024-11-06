#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AST.h"
#define DEBUG_MEMORY(msg, ptr) printf("DEBUG MEMORY: %s %p\n", msg, (void*)ptr)

// Add this line
void freeNode(ASTNode* node);
void debugPrintFunctionNode(ASTNode* node);
char* generateTempVariable();

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
    DEBUG_MEMORY("Allocated node", node);

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
// Function to create a binary operation node

// Function to generate a unique temporary variable name
char* generateTempVariable() {
    static int tempVarCounter = 0;
    char* tempVarName = (char*)malloc(20); // Allocate space for the name
    snprintf(tempVarName, 20, "t%d", tempVarCounter++); // Generate unique name
    return tempVarName;
}


ASTNode* createBinaryOpNode(char* op, ASTNode* left, ASTNode* right) {
    printf("DEBUG: Creating binary op node with op '%s'\n", op ? op : "NULL");
    ASTNode* node = createNode();
    node->type = NODE_TYPE_BINARY_OP; // Set node type
    if (op) {
        node->op = strdup(op);
    }
    node->left = left;  // Set left operand
    node->right = right; // Set right operand

    // Generate a temporary variable for the result
    node->temp_var_name = generateTempVariable();

    // Log the temporary variable creation
    printf("DEBUG: Temporary variable created for binary op: %s\n", node->temp_var_name);

    return node;
}

// Example usage in processing expressions
void processExpression(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_TYPE_BINARY_OP:
            processExpression(node->left);
            processExpression(node->right);
            // Generate TAC for binary operation
            printf("TAC: %s = %s %s %s\n", 
                   node->temp_var_name, 
                   node->left->temp_var_name, 
                   node->op, 
                   node->right->temp_var_name);
            break;

        case NODE_TYPE_UNARY_OP:
            processExpression(node->left); // Process the expression
            // Generate TAC for unary operation
            printf("TAC: %s = %s %s\n", 
                   node->temp_var_name, 
                   node->left->temp_var_name, 
                   node->op); // Ensure op is set correctly
            break;

       case NODE_TYPE_INTEGER:
            // Directly use the number value
            node->temp_var_name = generateTempVariable();
            printf("TAC: %s = %d\n", 
                   node->temp_var_name, 
                   node->value);
            break;
        case NODE_TYPE_FLOAT:
            // Directly use the number value
            node->temp_var_name = generateTempVariable();
            printf("TAC: %s = %d\n", 
                   node->temp_var_name, 
                   node->value);
            break;
            

        case NODE_TYPE_IDENTIFIER:
            // Use the identifier directly
            node->temp_var_name = strdup(node->id); // No need for a temporary variable
            break;

        case NODE_TYPE_ASSIGNMENT:
            processExpression(node->right); // Process the expression
            printf("TAC: %s = %s\n", 
                   node->left->id, // The identifier name
                   node->right->temp_var_name);
            break;

      case NODE_TYPE_FUNCTION_CALL:
    if (strcmp(node->id, "add") == 0) {
        // Process arguments first
        processExpression(node->funcCall.arguments->argumentList.args[0]);
        processExpression(node->funcCall.arguments->argumentList.args[1]);
        
        // Generate TAC for function call with proper argument handling
        printf("TAC: %s = %s(%s, %s)\n", 
            node->temp_var_name,
            node->id,
            node->funcCall.arguments->argumentList.args[0]->temp_var_name,
            node->funcCall.arguments->argumentList.args[1]->temp_var_name);
        
        // Generate addition operation
        printf("TAC: %s = %s + %s\n",
            node->temp_var_name,
            node->funcCall.arguments->argumentList.args[0]->temp_var_name,
            node->funcCall.arguments->argumentList.args[1]->temp_var_name);
    }
    break;


        default:
            printf("DEBUG: Unknown expression type\n");
            break;
    }
}

void processFunctionCall(ASTNode* node) {
    if (node->funcCall.arguments) {
        processExpression(node->funcCall.arguments);
    }
    // Generate TAC for the function call
    printf("TAC: CALL %s\n", node->id);
}

void processStatement(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_TYPE_WRITE:
            processExpression(node->left);
            printf("TAC: WRITE %s\n", node->left->temp_var_name);
            break;

        case NODE_TYPE_RETURN:
            processExpression(node->left);
            printf("TAC: RETURN %s\n", node->left->temp_var_name);
            break;

        default:
            printf("DEBUG: Unknown statement type\n");
            break;
    }
}

void processAST(ASTNode* root) {
    if (!root) return;

    // Example for processing a program node
    if (root->type == NODE_TYPE_PROGRAM) {
        for (int i = 0; i < root->statements.count; i++) {
            processStatement(root->statements.stmts[i]);
        }
    } else {
        processExpression(root);
    }
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

    printf("DEBUG: Creating function declaration node with body at %p\n", (void*)body);

    ASTNode* node = createNode();
    node->type = NODE_TYPE_FUNCTION_DECLARATION;
    node->id = strdup(identifier->id);
    node->left = parameters;
    node->right = returnType;
    
    if (body && body->statements.count > 0) {
        printf("DEBUG: Body has %d statements\n", body->statements.count);
        node->statements.count = body->statements.count;
        node->statements.stmts = malloc(sizeof(ASTNode*) * body->statements.count);
        
        for (int i = 0; i < body->statements.count; i++) {
            printf("DEBUG: Copying statement %d from %p\n", i, (void*)body->statements.stmts[i]);
            node->statements.stmts[i] = body->statements.stmts[i];
            printf("DEBUG: Statement %d copied to %p\n", i, (void*)node->statements.stmts[i]);
        }
    }
    
    // Don't free the body node here - let the caller handle it
    printf("DEBUG: Function declaration node creation complete\n");
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
     if (identifier == NULL || type == NULL) {
        fprintf(stderr, "Error: NULL identifier or type in parameter creation\n");
        return NULL;
    }

    ASTNode* node = createNode();
    node->type = NODE_TYPE_PARAMETER;
    node->param.identifier = identifier;
    node->param.paramType = type;
        printf("DEBUG: Created parameter node with identifier: %s, type: %s\n",
           identifier->id, type->id);
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

// Create a function call node
ASTNode* createFunctionCallNode(char* identifier, ASTNode* arguments) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_FUNCTION_CALL;
    node->id = strdup(identifier);
    node->funcCall.arguments = arguments;
    node->temp_var_name = generateTempVariable();
    return node;
}

// Create an argument list node
ASTNode* createArgumentListNode(ASTNode** args, int count) {
    ASTNode* node = createNode();
    node->type = NODE_TYPE_ARGUMENT_LIST;
    node->argumentList.count = count;
    node->argumentList.args = malloc(sizeof(ASTNode*) * count);
    if (node->argumentList.args == NULL) {
        fprintf(stderr, "Memory allocation failed for argument list array\n");
        free(node);
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        node->argumentList.args[i] = args[i];
    }
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


// Append an argument node to an existing list
ASTNode* appendArgumentNode(ASTNode* list, ASTNode* arg) {
    if (list->type != NODE_TYPE_ARGUMENT_LIST) {
        fprintf(stderr, "Error: Node is not an argument list\n");
        return NULL;
    }
    list->argumentList.args = realloc(list->argumentList.args, (list->argumentList.count + 1) * sizeof(ASTNode*));
    if (list->argumentList.args == NULL) {
        fprintf(stderr, "Memory allocation failed when appending argument\n");
        return NULL;
    }
    list->argumentList.args[list->argumentList.count++] = arg;
    return list;
}

void printAST(ASTNode* node, int indentLevel) {
    if (node == NULL) return;

    for (int i = 0; i < indentLevel; i++) {
        printf("  ");  // Indentation for readability
    }

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
            printf("Unknown Node Type\n");
            break;
    }

    if (node->left) printAST(node->left, indentLevel + 1);
    if (node->right) printAST(node->right, indentLevel + 1);
}

// Function to free the identifier string in an AST node
void freeIdentifier(ASTNode* node) {
    if (node->type == NODE_TYPE_IDENTIFIER && node->id != NULL) {
        free(node->id);
        node->id = NULL;
    }
}

void freeNode(ASTNode* node) {
    if (!node) return;

    // Free child nodes
    if (node->left) {
        freeNode(node->left);
        node->left = NULL;
    }
    
    if (node->right) {
        freeNode(node->right);
        node->right = NULL;
    }

    // Free statements
    if (node->statements.stmts) {
        for (int i = 0; i < node->statements.count; i++) {
            if (node->statements.stmts[i]) {
                freeNode(node->statements.stmts[i]);
            }
        }
        free(node->statements.stmts);
        node->statements.stmts = NULL;
    }

    // Free the identifier string if applicable
    freeIdentifier(node);

    // Free other string members
    if (node->op) free(node->op);
    
    free(node);
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
