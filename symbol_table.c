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
void insert_symbol(char *name, char *type, char** paramTypes, int paramCount, char* returnType) {
    if (name == NULL || type == NULL) {
        fprintf(stderr, "Error: NULL name or type passed to insert_symbol\n");
        return;
    }

    // Check for redeclaration
    if (lookup_symbol(name) != -1) {
        printf("Error: redeclaration of %s\n", name);
        return;
    }

    // Insert new symbol
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].type = strdup(type);

    if (strcmp(type, "function") == 0) {
        symbol_table[symbol_count].functionInfo = malloc(sizeof(FunctionInfo));
        symbol_table[symbol_count].functionInfo->name = strdup(name);
        symbol_table[symbol_count].functionInfo->returnType = returnType ? strdup(returnType) : NULL;
        symbol_table[symbol_count].functionInfo->paramTypes = malloc(sizeof(char*) * paramCount);
        symbol_table[symbol_count].functionInfo->paramCount = paramCount;

        for (int i = 0; i < paramCount; i++) {
            if (paramTypes[i] != NULL) {
                symbol_table[symbol_count].functionInfo->paramTypes[i] = strdup(paramTypes[i]);
            } else {
                fprintf(stderr, "Error: NULL parameter type at index %d\n", i);
                symbol_table[symbol_count].functionInfo->paramTypes[i] = NULL;
            }
        }
    } else {
        symbol_table[symbol_count].functionInfo = NULL;
    }

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
    printf("Index\tName\t\tType\t\tFunction Info\n");
    printf("------------------------------------------------\n");
    
    for (int i = 0; i < symbol_count; i++) {
        printf("%d\t%s\t\t%s", i, symbol_table[i].name, symbol_table[i].type);
        if (symbol_table[i].functionInfo != NULL) {
            printf("\t\tReturn: %s, Params: ", symbol_table[i].functionInfo->returnType);
            for (int j = 0; j < symbol_table[i].functionInfo->paramCount; j++) {
                printf("%s ", symbol_table[i].functionInfo->paramTypes[j]);
            }
        }
        printf("\n");
    }
}

void freeParamTypes(char** paramTypes, int count) {
    for (int i = 0; i < count; i++) {
        free(paramTypes[i]);
    }
    free(paramTypes);
}


void clean_up_symbol_table() {
    for (int i = 0; i < symbol_count; i++) {
        free(symbol_table[i].name);
        free(symbol_table[i].type);
        if (symbol_table[i].functionInfo != NULL) {
            free(symbol_table[i].functionInfo->name);
            free(symbol_table[i].functionInfo->returnType);
            for (int j = 0; j < symbol_table[i].functionInfo->paramCount; j++) {
                free(symbol_table[i].functionInfo->paramTypes[j]);
            }
            free(symbol_table[i].functionInfo->paramTypes);
            free(symbol_table[i].functionInfo);
        }
    }
}
