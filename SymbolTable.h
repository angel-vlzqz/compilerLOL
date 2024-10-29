#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdlib.h>
#include <stdbool.h>
#include "Array.h"

typedef struct Symbol
{
    char *name;
    char *type;
    char *value;
    int index;
    bool isArray;
    bool isFunction;
    Array *arrayInfo;
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable
{
    int size;
    Symbol **table;             // Array of symbol pointers (linked list heads)
    int scope;
    struct SymbolTable *prev;   // Pointer to the outer scope
    struct SymbolTable *next;   // Pointer to the inner scope
} SymbolTable;

extern int GlobalScope;

// Function Declarations
unsigned int hashFunction(const char *name, int tableSize);
Symbol *addFunctionSymbol(SymbolTable *symbolTable, const char *name, const char *returnType);
Symbol *addVariableSymbol(SymbolTable *symbolTable, const char *name, const char *type, bool isArray, Array *arrayInfo);
Symbol *createSymbol(const char *name, const char *type, int index, bool isArray, bool isFunction, Array *arrayInfo);
void insertSymbol(SymbolTable *symbolTable, const char *name, const char *type, bool isArray, bool isFunction, Array *arrayInfo);
Symbol *findSymbol(SymbolTable *symbolTable, const char *name);
void freeSymbolTable(SymbolTable *symbolTable);
void freeAllSymbolTables(SymbolTable *symbolTable); // Added function prototype
SymbolTable *createSymbolTable(int size, SymbolTable *prev);
void updateSymbolValue(SymbolTable *symbolTable, const char *name, const char *value);
const char *getSymbolValue(SymbolTable *symbolTable, const char *name);
SymbolTable *getSymbolTableAtDepth(SymbolTable *symTab, int scope);
void collectAllSymbols(SymbolTable *symTab, Symbol ***symbolList, int *symbolCount, int *symbolCapacity);
/*
Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition
that all men are created equal.

Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a
great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that
nation might live. It is altogether fitting and proper that we should do this.

But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled
here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never
forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly
advanced. It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that
cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation,
under God, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not perish from the earth.
*/
#endif // SYMBOL_TABLE_H