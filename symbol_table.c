#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

Symbol symbol_table[TABLE_SIZE];
int symbol_count = 0;

// Initialize the symbol table
void init_symbol_table() {
    symbol_count = 0;
}

// Insert a new symbol
void insert_symbol(char *name, char *type) {
    // Check for redeclaration
    if (lookup_symbol(name) != -1) {
        printf("Error: redeclaration of %s\n", name);
        return;
    }
    
    // Insert new symbol
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].type = strdup(type);
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
    printf("Index\tName\t\tType\n");
    printf("--------------------------------\n");
    
    for (int i = 0; i < symbol_count; i++) {
        printf("%d\t%s\t\t%s\n", i, symbol_table[i].name, symbol_table[i].type);
    }
}

void clean_up_symbol_table() {
    for (int i = 0; i < symbol_count; i++) {
        free(symbol_table[i].name);  // Free allocated name memory
        free(symbol_table[i].type);  // Free allocated type memory
    }
}
