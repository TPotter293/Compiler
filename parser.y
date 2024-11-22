%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "AST.h"
#include "optimizer.h"
#include "code_generator.h"
#include "parser.tab.h"
#define LT 300
#define GT 301



extern int yylex();
extern int yyparse();
extern int yylineno;
extern FILE* yyin;

struct symbol_table_entry {
    char* name;
    char* type;
    char* value;
    int is_array;
    int array_size;
    char* scope;
};


ASTNode* root = NULL; // Root of the AST
char* current_scope = "global"; // Global variable to track the current scope

void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
    exit(1);
}

void syntaxError(const char *message) {
    fprintf(stderr, "Syntax error: %s at line %d\n", message, yylineno);
}

void semanticError(const char *message) {
    fprintf(stderr, "Semantic error: %s at line %d\n", message, yylineno);
}

char** extractParamTypes(ASTNode** params, int count) {
    char** types = malloc(count * sizeof(char*));
    for (int i = 0; i < count; i++) {
        if (params[i]->id != NULL) {
            types[i] = strdup(params[i]->id);
        } else {
            fprintf(stderr, "Error: Parameter id is NULL at index %d\n", i);
            types[i] = NULL; // Handle the NULL case appropriately
        }
    }
    return types;
}

%}

// Declare the union for storing various types, including ASTNode pointers
%union {
    int intval;        // For int
    float floatval;    // For float
    char* strval;      // For identifiers, types, etc.
    struct ASTNode* node;     // For AST nodes
}

%token WRITE IF ELSE RETURN FUNCTION VAR
%token <intval> INT 
%token <floatval> FLOAT
%token <strval> TYPE IDENTIFIER BOOLVAL
%token <char> SEMICOLON EQ PLUS MINUS MULT DIVIDE
%token <char> NOT LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE COMMA
%token LT GT EQTO NEQTO AND OR NOT



%type <node> program statements statement expression declaration assignment write_statement if_statement return_statement function_declaration variable_declaration parameter_list parameters argument_list

%left OR
%left AND
%left EQ
%left PLUS MINUS
%left MULT DIVIDE
%right NOT

%%

// Grammar rules remain unchanged

program:
    statements
    {
        root = createProgramNode($1); // Assign root to the program node
        printf("Program parsed successfully!\n");
    }
    | error {syntaxError("Invalid program structure"); YYABORT; }
    ;

statements:
    statements statement
    {
        $$ = $1;
        $$->statements.stmts = realloc($$->statements.stmts, ($$->statements.count + 1) * sizeof(ASTNode*));
        $$->statements.stmts[$$->statements.count++] = $2;
    }
    | statement
    {
        $$ = createStatementsNode(&$1, 1);
    }
    | /* empty */ { $$ = createStatementsNode(NULL, 0); }
    ;



statement:
    declaration
    {
        $$ = $1;
        printf("Declaration statement parsed.\n");
    }
    | assignment
    {
        $$ = $1;
        printf("Assignment statement parsed.\n");
    }
    | write_statement
    {
        $$ = $1;
        printf("Write statement parsed.\n");
    }
    | if_statement
    {
        $$ = $1;
        printf("If statement parsed.\n");
    }
    | return_statement
    {
        $$ = $1;
        printf("Return statement parsed.\n");
    }
    | function_declaration
    {
        $$ = $1;
        printf("Function declaration parsed.\n");
    }
    | variable_declaration
    {
        $$ = $1;
        printf("Variable declaration parsed.\n");
    }
    ;

declaration:
    TYPE IDENTIFIER SEMICOLON
    {
        $$ = createDeclarationNode(createIdentifierNode($1), createIdentifierNode($2));
        printf("Declaration of variable '%s' of type '%s'.\n", $2, $1);
        insert_symbol($2, $1, "null", 0, "null");
        print_symbol_table();
    }
    | TYPE LBRACKET INT RBRACKET IDENTIFIER SEMICOLON
    {
        printf("Array declaration of '%s' with size %d of type '%s'.\n", $5, $3, $1);
        $$ = createArrayDeclarationNode(createIdentifierNode($5), $1, $3);
        insert_array_symbol($5, $1, $3, current_scope); // Insert into symbol table
        print_symbol_table();
    }
    ;

variable_declaration:
    VAR IDENTIFIER TYPE SEMICOLON
    {
        $$ = createVariableDeclarationNode(createIdentifierNode($2), createIdentifierNode($3));
        printf("Variable declaration: %s of type %s in scope '%s'\n", $2, $3, current_scope);
        insert_symbol($2, $3, "null", 0, "null"); // Use current_scope
        print_symbol_table();
        free($2);
        free($3);
    }
    ;

function_declaration:
    FUNCTION TYPE IDENTIFIER LPAREN parameter_list RPAREN LBRACE statements RBRACE
    {
        printf("DEBUG: Processing full function declaration for %s\n", $3);

        // Create the nodes for function declaration
        printf("DEBUG: Creating function declaration for %s\n", $3);
        ASTNode* idNode = createIdentifierNode($3);
        ASTNode* returnTypeNode = createIdentifierNode($2);
        printf("DEBUG: Created identifier and return type nodes\n");
        printf("DEBUG: Parameter list node: %p\n", (void*)$5);
        printf("DEBUG: Statements node: %p\n", (void*)$8);

        // Create function declaration node
        $$ = createFunctionDeclarationNode(idNode, $5, returnTypeNode, $8);
        printf("DEBUG: createFunctionDeclarationNode returned: %p\n", (void*)$$);
         if ($$ == NULL) {
            yyerror("Failed to create function declaration node");
            YYABORT;
        }

          printf("DEBUG: Function declaration node created successfully\n");
        printf("DEBUG: Node type: %d\n", $$->type);
        printf("DEBUG: Node id: %s\n", $$->id);
        printf("DEBUG: Node left (parameters): %p\n", (void*)$$->left);
        printf("DEBUG: Node right (return type): %p\n", (void*)$$->right);
        printf("DEBUG: Node statements count: %d\n", $$->statements.count);
        printf("Full function declaration parsed: %s\n", $3);

        // Extract and store parameter types
        char** paramTypes = extractParamTypes($5->parameters.params, $5->parameters.count);
        insert_symbol($3, "function", paramTypes, $5->parameters.count, $2);
        freeParamTypes(paramTypes, $5->parameters.count);

        printf("DEBUG: Function declaration node created successfully\n");
        printf("Full function declaration parsed: %s\n", $3);


        // Add the function declaration to the program's AST
        printf("DEBUG: About to add function declaration to AST\n");
        printf("DEBUG: Root node address: %p\n", (void*)root);
        if (root == NULL) {
            printf("DEBUG: Creating new program node as root\n");
            ASTNode* stmtNode = createStatementsNode(&$$, 1);
            root = createProgramNode(stmtNode);
            printf("DEBUG: New program node created as root\n");
        } else {
            printf("DEBUG: Adding function to existing program node\n");
            ASTNode* newStatements = createStatementsNode(&$$, 1);
            root->statements.stmts = realloc(root->statements.stmts, 
                                             (root->statements.count + 1) * sizeof(ASTNode*));
            if (root->statements.stmts == NULL) {
                yyerror("Memory allocation failed when adding function to program");
                YYABORT;
            }
            root->statements.stmts[root->statements.count++] = newStatements->statements.stmts[0];
            free(newStatements);
            printf("DEBUG: Function added to existing program node\n");
        }
        printf("DEBUG: Function declaration added to AST\n");

    }
    | FUNCTION TYPE IDENTIFIER LPAREN parameter_list RPAREN SEMICOLON
    {
        printf("DEBUG: Processing function prototype for %s\n", $3);

        // Create the nodes for function prototype
        ASTNode* idNode = createIdentifierNode($3);
        ASTNode* returnTypeNode = createIdentifierNode($2);

        // Create function prototype node
        $$ = createFunctionPrototypeNode(idNode, $5, returnTypeNode);

        // Extract and store parameter types
        char** paramTypes = extractParamTypes($5->parameters.params, $5->parameters.count);
        insert_symbol($3, "function", paramTypes, $5->parameters.count, $2);
        freeParamTypes(paramTypes, $5->parameters.count);

        printf("Function prototype parsed: %s\n", $3);
    }
    ;



parameter_list:
    parameters
    | /* empty */
    {
        $$ = createParametersNode(NULL, 0); // Empty parameter list
    }
    ;

parameters:
    parameters COMMA TYPE IDENTIFIER
    {
        $$ = $1;
        $$->parameters.params = realloc($$->parameters.params, ($$->parameters.count + 1) * sizeof(ASTNode*));
        $$->parameters.params[$$->parameters.count++] = createParameterNode(createIdentifierNode($4), createIdentifierNode($3));
        free($3);
        free($4);
    }
    | TYPE IDENTIFIER
    {
        ASTNode* paramNode = createParameterNode(createIdentifierNode($2), createIdentifierNode($1));
        ASTNode** params = malloc(sizeof(ASTNode*));
        params[0] = paramNode;
        $$ = createParametersNode(params, 1);
        free($1);
        free($2);
    }

    ;



assignment:
    IDENTIFIER EQ expression SEMICOLON
    {
        $$ = createAssignmentNode(createIdentifierNode($1), $3);
        printf("Assignment to variable '%s' in scope '%s'.\n", $1, current_scope);
        free($1);
    }
    | IDENTIFIER LBRACKET expression RBRACKET EQ expression SEMICOLON
    {
        $$ = createArrayAssignmentNode($1, $3, $6);
        printf("Array Assignment: %s[%s] = %s.\n", $1, $3, $6);
    }
    ;


write_statement:
    WRITE expression SEMICOLON
    {
        $$ = createWriteNode($2);
        printf("Write statement encountered.\n");
    }
    ;

if_statement:
    IF LPAREN expression RPAREN LBRACE statements RBRACE
    {
        $$ = createIfNode($3, $6, NULL);
        printf("If statement with no else branch parsed.\n");
    }
    | IF LPAREN expression RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
    {
        $$ = createIfNode($3, $6, $10);
        printf("If-else statement parsed.\n");
    }
    ;

return_statement:
    RETURN expression SEMICOLON
    {
        $$ = createReturnNode($2);
        printf("Return statement parsed.\n");
    }
    ;

expression:
    INT 
    {
        $$ = createIntegerNode($1);
        printf("Integer expression: %d\n", $1);
    }
    | FLOAT 
    {
        $$ = createFloatNode($1);
        printf("Float expression: %f\n", $1);
    }
    | expression LT expression
    {
        $$ = createBinaryOpNode("<", $1, $3);
        printf("Less than expression parsed.\n");
    }
    | expression GT expression
    {
        $$ = createBinaryOpNode(">", $1, $3);
        printf("Greater than expression parsed.\n");
    }
    | expression EQTO expression
    {
        $$ = createBinaryOpNode("==", $1, $3);
        printf("Equal to expression parsed.\n");
    }
    | expression NEQTO expression
    {
        $$ = createBinaryOpNode("!=", $1, $3);
        printf("Not Equal ot expression parsed.\n");
    }
    | IDENTIFIER
    {
        $$ = createIdentifierNode($1);
        printf("Identifier expression: %s\n", $1);
        free($1);
    }

    | IDENTIFIER LPAREN argument_list RPAREN
    {
        $$ = createFunctionCallNode($1, $3);
        printf("Function call expression: %s\n", $1);
    }

    | BOOLVAL
    {
        $$ = createBooleanNode($1);
        printf("Boolean expression: %s\n", $1);
        free($1);
    }
    | expression PLUS expression
    {
        $$ = createBinaryOpNode("+", $1, $3);
        printf("Addition expression parsed.\n");
    }
    | expression MINUS expression
    {
        $$ = createBinaryOpNode("-", $1, $3);
        printf("Subtraction expression parsed.\n");
    }
    | expression MULT expression
    {
        $$ = createBinaryOpNode("*", $1, $3);
        printf("Multiplication expression parsed.\n");
    }
    | expression DIVIDE expression
    {
        $$ = createBinaryOpNode("/", $1, $3);
        printf("Division expression parsed.\n");
    }
    | expression AND expression
    {
        $$ = createBinaryOpNode("AND", $1, $3);
        printf("Logical AND expression parsed.\n");
    }
    | expression OR expression
    {
        $$ = createBinaryOpNode("OR", $1, $3);
        printf("Logical OR expression parsed.\n");
    }
    | NOT expression
    {
        $$ = createUnaryOpNode($2);
        printf("Logical NOT expression parsed.\n");
    }
    | LPAREN expression RPAREN
    {
        // $$ = $2;
        printf("Parenthesized expression parsed.\n");
    }
    | IDENTIFIER LBRACKET INT RBRACKET
    {
        $$ = createArrayAccessNode($1, $3);
        printf("Array access: %s[%s].\n", $1, $3);
    }
    ;

    argument_list:
    /* empty */
    {
        $$ = createArgumentListNode(NULL, 0);
    }
    | expression
    {
        $$ = createArgumentListNode(&$1, 1);
    }
    | argument_list COMMA expression
    {
        $$ = appendArgumentNode($1, $3);
    }
    ;

%% 

int lookupSymbolValue(char* id) {
    struct symbol_table_entry* entry = lookup_symbol(id);
    if (entry != NULL) {
        return atoi(entry->value);
    }
    return 0;
}



int evaluateExpression(ASTNode* expr) {
    if (!expr) return 0;
    
    switch(expr->type) {
        case NODE_TYPE_INTEGER:
            return expr->value.intValue;
            
        case NODE_TYPE_FUNCTION_CALL:
            if (strcmp(expr->id, "add") == 0) {
                int arg1 = evaluateExpression(expr->funcCall.arguments->argumentList.args[0]);
                int arg2 = evaluateExpression(expr->funcCall.arguments->argumentList.args[1]);
                return arg1 + arg2;
            }
            break;
    }
    return 0;
}


int executeFunctionCall(ASTNode* node) {
    if (strcmp(node->id, "add") == 0) {
        // Get the argument values
        ASTNode* args = node->funcCall.arguments;
        int arg1 = evaluateExpression(args->argumentList.args[0]);
        int arg2 = evaluateExpression(args->argumentList.args[1]);
        return arg1 + arg2;
    }
    return 0;
}



void executeStatement(ASTNode* stmt) {
    if (!stmt) return;
    
    switch(stmt->type) {
        case NODE_TYPE_DECLARATION:
            printf("Executing declaration\n");
            break;
            
        case NODE_TYPE_ASSIGNMENT: {
            printf("Executing assignment\n");
            if (stmt->left && stmt->left->id) {
                int value = evaluateExpression(stmt->right);
                char value_str[32];
                snprintf(value_str, 32, "%d", value);
                // Update symbol table with the new value
                update_symbol_value(stmt->left->id, value_str);
            }
            break;
        }
        
        case NODE_TYPE_WRITE:
            printf("Executing write statement\n");
            if (stmt->left && stmt->left->id) {
                struct symbol_table_entry* entry = lookup_symbol(stmt->left->id);
                if (entry && entry->value) {
                    int value = atoi(entry->value);
                    printf("Output: %d\n", value);
                }
            }
            break;
    }
}


void executeMain(ASTNode* root) {
    if (!root) return;
    
    for (int i = 0; i < root->statements.count; i++) {
        ASTNode* stmt = root->statements.stmts[i];
        if (stmt->type == NODE_TYPE_FUNCTION_DECLARATION && 
            stmt->id != NULL && 
            strcmp(stmt->id, "main") == 0) {
            printf("Executing main function...\n");
            for (int j = 0; j < stmt->statements.count; j++) {
                executeStatement(stmt->statements.stmts[j]);
            }
            return;
        }
    }
    printf("Error: No main function found\n");
}



int main(int argc, char** argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }

    printf("Starting parser...\n");
    int parse_result = yyparse();
    if (parse_result == 0) {
        printf("Parsing completed successfully.\n");
    } else {
        printf("Parsing failed.\n");
        return 1;
    }

    // Print the AST
    printf("Abstract Syntax Tree (AST):\n");
    printAST(root, 0);  // Start printing from the root node with indentation level 0

    // Perform semantic analysis
    performSemanticAnalysis(root);

    // Execute main function
    executeMain(root);

    // Optimize TAC
    printf("Optimizing TAC...\n");
    optimize_TAC("output.tac", "optimized.tac");
    printf("TAC optimization completed.\n");

    // Generate MIPS code

    FILE* output_file = fopen("output.asm", "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error opening output file\n");
        return 1;
    }
    generateCode("output.tac", output_file);

    fclose(output_file);

    // Print or traverse the AST here if needed
    clean_up_symbol_table();
    freeASTNode(root);
    return 0;
}
