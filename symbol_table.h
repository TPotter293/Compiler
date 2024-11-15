#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define TABLE_SIZE 100

// Define FunctionInfo structure
typedef struct {
    char* name;
    char* returnType;
    char** paramTypes;
    int paramCount;
} FunctionInfo;

typedef struct {
    char *name;     // Identifier name
    char *type;     // Type (int, float, etc.)
    char *scope;
    int is_array;
    int array_size;
} Symbol;

// Declare symbol table functions
void init_symbol_table();
void insert_symbol(char *name, char *type, char* scope);
void insert_array_symbol(char *name, char *type, int size, char *scope);
int lookup_symbol(char *name);
void print_symbol_table();
void clean_up_symbol_table();

#endif