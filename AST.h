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
    NODE_TYPE_FUNCTION_PROTOTYPE,
    NODE_TYPE_PARAMETER,
    NODE_TYPE_PARAMETERS,
    NODE_TYPE_FUNCTION_CALL,      // Added for function calls
    NODE_TYPE_ARGUMENT_LIST       // Added for argument lists
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
            int count;               // Count of statements
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
        } funcProto;
        struct {
            struct ASTNode* identifier;  // Parameter identifier (e.g., variable name)
            struct ASTNode* paramType;   // Parameter type (e.g., int, float)
        } param;
        struct {
            struct ASTNode** params;  // Array of parameters
            int count;                // Count of parameters
        } parameters;
        struct {
            struct ASTNode* functionName;  // Function name for calls
            struct ASTNode* arguments;     // Arguments for the function call
        } funcCall;  // Added for function calls
        struct {
            struct ASTNode** args;  // Array of arguments
            int count;              // Count of arguments
        } argumentList;  // Added for argument lists
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
ASTNode* createFunctionPrototypeNode(ASTNode* identifier, ASTNode* parameters, ASTNode* returnType);
ASTNode* createParameterNode(ASTNode* identifier, ASTNode* type);
ASTNode* createParametersNode(ASTNode** params, int count);
ASTNode* createStatementsNode(ASTNode** stmts, int count);
ASTNode* createFunctionCallNode(char* identifier, ASTNode* arguments);  // Added for function calls
ASTNode* createArgumentListNode(ASTNode** args, int count);  // Added for argument lists
ASTNode* appendArgumentNode(ASTNode* list, ASTNode* arg);  // Added for appending arguments

// Function for printing the AST (ensure this is implemented in AST.c)
void printAST(ASTNode* node, int indentLevel);

// In AST.h or another appropriate header file
char** extractParamTypes(ASTNode** params, int count);


#endif  // AST_H
