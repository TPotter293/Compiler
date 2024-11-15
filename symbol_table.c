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

// Insert a new symbol (non-array)
void insert_symbol(char *name, char *type, char *scope) {
    // Check for redeclaration
    if (lookup_symbol(name) != -1) {
        printf("Error: redeclaration of %s\n", name);
        return;
    }

    // Insert new symbol
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].type = strdup(type);
    symbol_table[symbol_count].scope = strdup(scope);
    symbol_table[symbol_count].is_array = 0;  // Not an array
    symbol_table[symbol_count].array_size = 0; // Not applicable

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


// Lookup a symbol by name
int lookup_symbol(char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return i;  // Found, return index
        }
    }
    return -1;  // Not found
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

// Clean up symbol table
void clean_up_symbol_table() {
    for (int i = 0; i < symbol_count; i++) {
        free(symbol_table[i].name);
        free(symbol_table[i].type);
        free(symbol_table[i].scope);
    }
}