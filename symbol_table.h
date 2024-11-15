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
    int is_initialized; // New field to track initialization status
    FunctionInfo* functionInfo;
    char *scope;
    int is_array;
    int array_size;
} Symbol;

// Declare symbol table functions
void init_symbol_table();
void insert_symbol(char *name, char *type, char** paramTypes, int paramCount, char* returnType);
Symbol* lookup_symbol(char *name);  // Ensure this matches the definition in symbol_table.c
void insert_array_symbol(char *name, char *type, int size, char *scope);
void print_symbol_table();
void freeParamTypes(char** paramTypes, int count);
void clean_up_symbol_table();

#endif