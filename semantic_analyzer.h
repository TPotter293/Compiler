#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "AST.h"

// Main function to perform semantic analysis
void performSemanticAnalysis(ASTNode* root);

char* newTemp();
char* newFloat();

// Function to write TAC to the file
void generateTAC(const char* tac_line);

#endif // SEMANTIC_ANALYZER_H
