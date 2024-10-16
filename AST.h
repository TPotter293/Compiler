#ifndef AST_H
#define AST_H

// Enum to define node types in the AST
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
    NODE_TYPE_FUNCTION_PROTOTYPE,  // Add this for function prototypes
    NODE_TYPE_PARAMETER,
    NODE_TYPE_PARAMETERS,
} NodeType;

// ASTNode structure for representing AST nodes
typedef struct ASTNode {
    NodeType type;  // Node type (e.g., program, statement, etc.)
    char* op;       // For operators (e.g., +, -, *)
    int value;      // For numeric values (numbers)
    char* id;       // For identifiers (variable names, function names)
    struct ASTNode* left;    // Left child (if applicable)
    struct ASTNode* right;   // Right child (if applicable)
    int temp_var;         // Temporary variable number (for temporary variables)
    char* temp_var_name;  // Name of the temporary variable
    union {
        struct {
            struct ASTNode** stmts;  // Array of statements
            int count;                // Count of statements
        } statements;
        struct {
            struct ASTNode* identifier;  // Identifier (for declarations, assignments)
            struct ASTNode* varType;     // Variable type (for declarations)
        } varDecl;
        struct {
            struct ASTNode* identifier;  // Function identifier (function name)
            struct ASTNode* parameters;  // Function parameters
            struct ASTNode* returnType;  // Function return type
            struct ASTNode* body;        // Function body (statements)
        } funcDecl;
        struct {
            struct ASTNode* identifier;  // Function identifier (for prototype)
            struct ASTNode* parameters;  // Function parameters
            struct ASTNode* returnType;  // Function return type
        } funcProto;  // Function prototype
        struct {
            struct ASTNode* identifier;  // Parameter identifier (e.g., variable name)
            struct ASTNode* paramType;   // Parameter type (e.g., int, float)
        } param;
        struct {
            struct ASTNode** params;  // Array of parameters
            int count;                // Count of parameters
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
ASTNode* createFunctionPrototypeNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType);  // Adjusted for function prototypes
ASTNode* createParameterNode(ASTNode* identifier, ASTNode* type);
ASTNode* createParametersNode(ASTNode* parameter, int count);

// Function for creating a statement node
ASTNode* createStatementsNode(ASTNode** stmts, int count);

// Function for printing the AST
void printAST(ASTNode* node, int indentLevel);

#endif  // AST_H
