%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "AST.h"
#include "optimizer.h"
#include "code_generator.h"


extern int yylex();
extern int yyparse();
extern int yylineno;
extern FILE* yyin;

ASTNode* root = NULL; // Root of the AST

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
        types[i] = strdup(params[i]->id);  // Assuming the type is stored in the 'id' field
    }
    return types;
}
%}

// Declare the union for storing various types, including ASTNode pointers
%union {
    int intval;        // For numbers
    char* strval;      // For identifiers, types, etc.
    struct ASTNode* node;     // For AST nodes
}

%token WRITE IF ELSE RETURN FUNCTION VAR
%token <intval> NUMBER
%token <strval> TYPE IDENTIFIER
%token <char> SEMICOLON EQ PLUS MINUS MULT DIVIDE
%token <char> NOT LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE COMMA

%type <node> program statements statement expression declaration assignment write_statement if_statement return_statement function_declaration variable_declaration parameter_list parameters

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
        insert_symbol($2, $1, NULL, 0, NULL);
        print_symbol_table();
        free($1);
        free($2);
    }
    ;

    variable_declaration:
    VAR IDENTIFIER TYPE SEMICOLON
    {
        $$ = createVariableDeclarationNode(createIdentifierNode($2), createIdentifierNode($3));
        printf("Variable declaration: %s of type %s\n", $2, $3);
        insert_symbol($2, $3, NULL, 0, NULL);
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
        ASTNode* idNode = createIdentifierNode($3);
        ASTNode* returnTypeNode = createIdentifierNode($2);

        // Create function declaration node
        $$ = createFunctionDeclarationNode(idNode, $5, returnTypeNode, $8);

        // Extract and store parameter types
        char** paramTypes = extractParamTypes($5->parameters.params, $5->parameters.count);
        insert_symbol($3, "function", paramTypes, $5->parameters.count, $2);
        freeParamTypes(paramTypes, $5->parameters.count);

        printf("Full function declaration parsed: %s\n", $3);
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
        $$->parameters.params[$$->parameters.count++] = createParameterNode(createIdentifierNode($3), createIdentifierNode($4));
        free($3);
        free($4);
    }
    | TYPE IDENTIFIER
    {
        $$ = createParametersNode(createParameterNode(createIdentifierNode($1), createIdentifierNode($2)), 1);
        free($1);
        free($2);
    }
    ;



assignment:
    IDENTIFIER EQ expression SEMICOLON
    {
        $$ = createAssignmentNode(createIdentifierNode($1), $3);
        printf("Assignment to variable '%s'.\n", $1);
        free($1);
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
    NUMBER
    {
        $$ = createNumberNode($1);
        printf("Number expression: %d\n", $1);
    }
    | IDENTIFIER
    {
        $$ = createIdentifierNode($1);
        printf("Identifier expression: %s\n", $1);
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
        $$ = $2;
        printf("Parenthesized expression parsed.\n");
    }
    ;

%% 

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
    generateCode("optimized.tac", output_file);

    fclose(output_file);

    // Print or traverse the AST here if needed
    clean_up_symbol_table();
    return 0;
}
