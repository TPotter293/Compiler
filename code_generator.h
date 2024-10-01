#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "AST.h"
#include <stdio.h>

void generateCode(ASTNode* root, FILE* output_file);
void generateProgramCode(ASTNode* node, FILE* output_file);
void generateStatementsCode(ASTNode* node, FILE* output_file);
void generateStatementCode(ASTNode* node, FILE* output_file);
void generateDeclarationCode(ASTNode* node, FILE* output_file);
void generateAssignmentCode(ASTNode* node, FILE* output_file);
void generateWriteCode(ASTNode* node, FILE* output_file);
void generateIfCode(ASTNode* node, FILE* output_file);
void generateReturnCode(ASTNode* node, FILE* output_file);
void generateExpressionCode(ASTNode* node, FILE* output_file);
void generateBinaryOpCode(ASTNode* node, FILE* output_file);
void generateUnaryOpCode(ASTNode* node, FILE* output_file);

void initializeCodeGenSymbolTable();
void allocateVariable(const char* identifier);
int getVariableLocation(const char* identifier);
void freeCodeGenSymbolTable();

#endif // CODE_GENERATOR_H
