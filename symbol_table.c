#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

#define TABLE_SIZE 100

Symbol symbol_table[TABLE_SIZE];
int symbol_count = 0;

// Initialize the symbol table
void init_symbol_table() {
    symbol_count = 0;
}

// Insert a new symbol
// Update insert_symbol to handle values
void insert_symbol(char *name, char *type, char** paramTypes, int paramCount, char* returnType) {
    if (name == NULL || type == NULL) {
        fprintf(stderr, "Error: NULL name or type passed to insert_symbol\n");
        return;
    }

    // Check for redeclaration
    if (lookup_symbol(name) != NULL) {
        printf("Error: redeclaration of %s\n", name);
        return;
    }

    // Insert new symbol
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].type = strdup(type);

    if (strcmp(type, "function") == 0 && paramTypes != NULL) {
        symbol_table[symbol_count].functionInfo = malloc(sizeof(FunctionInfo));
        symbol_table[symbol_count].functionInfo->name = strdup(name);
        symbol_table[symbol_count].functionInfo->returnType = returnType ? strdup(returnType) : NULL;
        symbol_table[symbol_count].functionInfo->paramTypes = malloc(sizeof(char*) * paramCount);
        symbol_table[symbol_count].functionInfo->paramCount = paramCount;

        for (int i = 0; i < paramCount; i++) {
            symbol_table[symbol_count].functionInfo->paramTypes[i] = 
                paramTypes[i] ? strdup(paramTypes[i]) : strdup("unknown");
        }
    } else {
        symbol_table[symbol_count].functionInfo = NULL;
    }

    symbol_count++;
}


// Insert a new array symbol
void insert_array_symbol(char *name, char *type, int size, char *scope) {
    // Check for redeclaration
    if (lookup_symbol(name) != NULL) {  // Compare to NULL instead of -1
        printf("Error: redeclaration of array %s\n", name);
        return;
    }

    // Insert new array symbol
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].type = strdup(type);
    symbol_table[symbol_count].scope = strdup(scope);
    symbol_table[symbol_count].is_array = 1;    // This is an array
    symbol_table[symbol_count].array_size = size; // Size of the array, 0 for dynamic arrays

    symbol_count++;
}


// Add this function implementation
void update_symbol_value(char* name, char* value) {
    Symbol* entry = lookup_symbol(name);
    if (entry) {
        entry->type = strdup(value);  // Using the 'type' field to store the value
    }
}



// Lookup a symbol by name
Symbol* lookup_symbol(char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            printf("DEBUG: Found symbol: %s, Type: %s\n", name, symbol_table[i].type);
            return &symbol_table[i];  // Found, return pointer to symbol
        }
    }
    return NULL;  // Not found
}

// Print symbol table
void print_symbol_table() {
    printf("Symbol Table:\n");
    printf("Index\tName\t\tType\t\tScope\t\tArray\t\tSize\n");
    printf("----------------------------------------------------------------------------\n");

    for (int i = 0; i < symbol_count; i++) {
        printf("%d\t%s\t\t%s\t\t%s\t\t%s\t\t%d\n", i, symbol_table[i].name, symbol_table[i].type,
               symbol_table[i].scope, symbol_table[i].is_array ? "Yes" : "No", symbol_table[i].array_size);
    }
}

void freeParamTypes(char** paramTypes, int count) {
    for (int i = 0; i < count; i++) {
        free(paramTypes[i]);
    }
    free(paramTypes);
}

// Clean up symbol table
void clean_up_symbol_table() {
    for (int i = 0; i < symbol_count; i++) {
        free(symbol_table[i].name);
        free(symbol_table[i].type);
        free(symbol_table[i].scope);
    }
}