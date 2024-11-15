#ifndef AST_H
#define AST_H

typedef enum {
    NODE_TYPE_UNDEFINED,
    NODE_TYPE_PROGRAM,
    NODE_TYPE_STATEMENT,
    NODE_TYPE_DECLARATION,
    NODE_TYPE_ASSIGNMENT,
    NODE_TYPE_WRITE,
    NODE_TYPE_IF,
    NODE_TYPE_RETURN,
    NODE_TYPE_INTEGER,
    NODE_TYPE_FLOAT,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_BOOLEAN,
    NODE_TYPE_BINARY_OP,
    NODE_TYPE_UNARY_OP,
    NODE_TYPE_VARIABLE_DECLARATION,
    NODE_TYPE_FUNCTION_DECLARATION,
    NODE_TYPE_PARAMETER,
    NODE_TYPE_PARAMETERS,
    NODE_TYPE_ARRAY_DECLARATION,
    NODE_TYPE_ARRAY_ACCESS,
    NODE_TYPE_ARRAY_ASSIGNMENT
} NodeType;


typedef struct ASTNode {
    NodeType type;  // Added node type field
    char* op;       // For operators (e.g., +, -, *)
    union {
        int intValue; // For integer values
        float floatValue; // For float values
    } value;
    char* id;       // For identifiers
    struct ASTNode* left;    // Left child
    struct ASTNode* right;   // Right child
    int temp_var;         // <-- Add this line to store the temporary variable number.
    char* temp_var_name;  // New field to store the temporary variable name
    char* boolean_val;   // true or false
    struct ASTNode* arrayIndex; // Index for array access
    struct ASTNode* assignedValue; // Value to assign to the array
    union {
        struct {
            struct ASTNode** stmts;
            int count;
        } statements;
        struct {
            struct ASTNode* identifier;
            struct ASTNode* varType;
            int arraySize;
        } varDecl;
        struct {
            struct ASTNode* identifier;
            struct ASTNode* parameters;
            struct ASTNode* returnType;
            struct ASTNode* body;
        } funcDecl;
        struct {
            struct ASTNode* identifier;
            struct ASTNode* paramType;
        } param;
        struct {
            struct ASTNode** params;
            int count;
        } parameters;
    };
} ASTNode;

// Function declarations for creating AST nodes
const char* typeToString(NodeType type);
ASTNode* createProgramNode(ASTNode* stmtList);
ASTNode* createDeclarationNode(ASTNode* type, ASTNode* id);
ASTNode* createAssignmentNode(ASTNode* id, ASTNode* expr);
ASTNode* createWriteNode(ASTNode* expr);
ASTNode* createIfNode(ASTNode* cond, ASTNode* thenStmt, ASTNode* elseStmt);
ASTNode* createReturnNode(ASTNode* expr);
ASTNode* createIntegerNode(int value);
ASTNode* createFloatNode(float value);
ASTNode* createIdentifierNode(char* id);
ASTNode* createBooleanNode(char* value);
ASTNode* createBinaryOpNode(char* op, ASTNode* left, ASTNode* right);
ASTNode* createUnaryOpNode(ASTNode* expr);
ASTNode* createVariableDeclarationNode(ASTNode* identifier, ASTNode* type);
ASTNode* createFunctionDeclarationNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType, ASTNode* body);
ASTNode* createParameterNode(ASTNode* identifier, ASTNode* type);
<<<<<<< Updated upstream
ASTNode* createParametersNode(ASTNode* parameter, int count);
=======
ASTNode* createParametersNode(ASTNode** params, int count);
>>>>>>> Stashed changes
ASTNode* createArrayDeclarationNode(ASTNode* identifier, char* typeNode, int arraySize);
ASTNode* createArrayAccessNode(ASTNode* identifier, ASTNode* indexNode);
ASTNode* createArrayAssignmentNode(char* id, ASTNode* index, ASTNode* value);
ASTNode* createStatementsNode(ASTNode** stmts, int count);

// Function for printing the AST
void printAST(ASTNode* node, int indentLevel);
void freeASTNode(ASTNode* node);

#endif
