#ifndef AST_H
#define AST_H

typedef enum {
    NODE_TYPE_PROGRAM,
    NODE_TYPE_STATEMENT,
    NODE_TYPE_DECLARATION,
    NODE_TYPE_ASSIGNMENT,
    NODE_TYPE_WRITE,
    NODE_TYPE_IF,
    NODE_TYPE_RETURN,
    NODE_TYPE_NUMBER,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_BINARY_OP,
    NODE_TYPE_UNARY_OP,
    NODE_TYPE_VARIABLE_DECLARATION,
    NODE_TYPE_FUNCTION_DECLARATION,
    NODE_TYPE_PARAMETER,
    NODE_TYPE_PARAMETERS
} NodeType;


typedef struct ASTNode {
    NodeType type;  // Added node type field
    char* op;       // For operators (e.g., +, -, *)
    int value;      // For numbers
    char* id;       // For identifiers
    struct ASTNode* left;    // Left child
    struct ASTNode* right;   // Right child
    int temp_var;         // <-- Add this line to store the temporary variable number.
     char* temp_var_name;  // New field to store the temporary variable name
    union {
        struct {
            struct ASTNode** stmts;
            int count;
        } statements;
        struct {
            struct ASTNode* identifier;
            struct ASTNode* varType;
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
ASTNode* createProgramNode(ASTNode* stmtList);
ASTNode* createDeclarationNode(ASTNode* type, ASTNode* id);
ASTNode* createAssignmentNode(ASTNode* id, ASTNode* expr);
ASTNode* createWriteNode(ASTNode* expr);
ASTNode* createIfNode(ASTNode* cond, ASTNode* thenStmt, ASTNode* elseStmt);
ASTNode* createReturnNode(ASTNode* expr);
ASTNode* createNumberNode(int value);
ASTNode* createIdentifierNode(char* id);
ASTNode* createBinaryOpNode(char* op, ASTNode* left, ASTNode* right);
ASTNode* createUnaryOpNode(ASTNode* expr);
ASTNode* createVariableDeclarationNode(ASTNode* identifier, ASTNode* type);
ASTNode* createFunctionDeclarationNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType, ASTNode* body);
ASTNode* createParameterNode(ASTNode* identifier, ASTNode* type);
ASTNode* createParametersNode(ASTNode* parameter, int count);


// Function for creating a statement node
ASTNode* createStatementsNode(ASTNode** stmts, int count);

// Function for printing the AST
void printAST(ASTNode* node, int indentLevel);

#endif
