#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "SymbolTable.h"
#include "Array.h"

int GlobalScope = 0;

// When adding a function symbol
Symbol *addFunctionSymbol(SymbolTable *symbolTable, const char *name, const char *returnType)
{
    // Functions are not arrays, so isArray is false, arrayInfo is NULL
    insertSymbol(symbolTable, name, returnType, false, true, NULL);
    // Return the newly added symbol
    return findSymbol(symbolTable, name);
}

// When adding a variable symbol
Symbol *addVariableSymbol(SymbolTable *symbolTable, const char *name, const char *type, bool isArray, Array *arrayInfo)
{
    insertSymbol(symbolTable, name, type, isArray, false, arrayInfo);
    // Return the newly added symbol
    return findSymbol(symbolTable, name);
}

// Hash function based on ASCII values
unsigned int hashFunction(const char *name, int tableSize)
{
    if (name == NULL)
    {
        fprintf(stderr, "Error: Tried to hash a NULL name.\n");
        return 0;
    }

    unsigned long hash = 0;
    while (*name)
    {
        hash = abs((hash * 31) + (unsigned char)(*name));
        name++;
    }
    return hash % tableSize;
}

// Create a new symbol
Symbol *createSymbol(const char *name, const char *type, int index, bool isArray, bool isFunction, Array *arrayInfo)
{
    Symbol *newSymbol = (Symbol *)malloc(sizeof(Symbol));
    if (!newSymbol)
    {
        fprintf(stderr, "Memory allocation failed for symbol\n");
        exit(1);
    }
    newSymbol->name = strdup(name);
    newSymbol->type = strdup(type);
    newSymbol->index = index;
    newSymbol->value = NULL;
    newSymbol->isArray = isArray;
    newSymbol->isFunction = isFunction; // Set the isFunction flag
    newSymbol->paramList = NULL;
    newSymbol->arrayInfo = arrayInfo;
    newSymbol->next = NULL;
    return newSymbol;
}

// Insert a symbol into the current scope's symbol table
void insertSymbol(SymbolTable *symbolTable, const char *name, const char *type, bool isArray, bool isFunction, Array *arrayInfo)
{
    if (findSymbolInCurrentScope(symbolTable, name) != NULL)
    {
        fprintf(stderr, "Error: Symbol %s is already declared in the current scope.\n", name);
        return;
    }

    unsigned int index = hashFunction(name, symbolTable->size);
    Symbol *newSymbol = createSymbol(name, type, index, isArray, isFunction, arrayInfo);

    if (strcmp(type, "float") == 0)
    {
        newSymbol->value = strdup("0.0"); // Default float value
    }

    newSymbol->next = symbolTable->table[index];
    symbolTable->table[index] = newSymbol;

    printf("Inserted symbol: Name = %s, Type = %s, Index = %d, isArray = %s, isFunction = %s\n",
           name, type, index, isArray ? "true" : "false", isFunction ? "true" : "false");
}

// Find a symbol, starting from the current scope and moving to outer scopes
Symbol *findSymbol(SymbolTable *symbolTable, const char *name)
{
    while (symbolTable != NULL)
    {
        unsigned int index = hashFunction(name, symbolTable->size);
        Symbol *current = symbolTable->table[index];

        while (current != NULL)
        {
            if (strcmp(current->name, name) == 0)
            {
                return current;
            }
            current = current->next;
        }
        symbolTable = symbolTable->prev;
    }
    return NULL;
}

// Free the memory used by the symbol table
void freeSymbolTable(SymbolTable *symbolTable)
{
    if (symbolTable == NULL)
    {
        return;
    }

    for (int i = 0; i < symbolTable->size; i++)
    {
        Symbol *symbol = symbolTable->table[i];

        while (symbol != NULL)
        {
            Symbol *next = symbol->next;

            if (symbol->name != NULL)
            {
                free(symbol->name);
            }
            if (symbol->type != NULL)
            {
                free(symbol->type);
            }
            if (symbol->value != NULL)
            {
                free(symbol->value);
            }
            if (symbol->isArray && symbol->arrayInfo != NULL)
            {
                freeArray(symbol->arrayInfo);
            }

            free(symbol);
            symbol = next;
        }
    }

    free(symbolTable->table);
    free(symbolTable);
    printf("Successfully freed symbol table\n");
}

// Create a new symbol table with a link to the outer scope
SymbolTable *createSymbolTable(int size, SymbolTable *prev)
{
    SymbolTable *newTable = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (!newTable)
    {
        fprintf(stderr, "Memory allocation failed for symbol table\n");
        return NULL;
    }

    newTable->size = size;
    newTable->table = (Symbol **)malloc(sizeof(Symbol *) * size);
    if (!newTable->table)
    {
        free(newTable);
        fprintf(stderr, "Error: Memory allocation failed for symbol table entries\n");
        return NULL;
    }

    // Initialize table entries to NULL
    for (int i = 0; i < size; i++)
    {
        newTable->table[i] = NULL;
    }

    newTable->scope = GlobalScope++;
    newTable->prev = prev;
    newTable->next = NULL;

    // If the previous symbol table exists, update its next pointer
    if (prev)
    {
        // printf("prev is valid and located at address %p\n", (void *)prev);
        // Check if prev->next is already set
        if (prev->next == NULL)
        {
            prev->next = newTable;
        }
        else
        {
            // If prev->next is not NULL, traverse to the end and append
            SymbolTable *temp = prev->next;
            while (temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = newTable;
        }
    }

    printf("Created new symbol table at address %p\n", (void *)newTable);
    printf("newTable->prev = %p\n", (void *)newTable->prev);
    printf("newTable->next = %p\n", (void *)newTable->next);

    return newTable;
}

// Update the symbol value in the current or nearest outer scope
void updateSymbolValue(SymbolTable *symbolTable, const char *name, const char *value)
{
    Symbol *symbol = findSymbol(symbolTable, name);

    if (symbol == NULL)
    {
        fprintf(stderr, "Error: Symbol %s not found in the current or outer scopes, cannot update value.\n", name);
        return;
    }
    if (symbol->isArray)
    {
        fprintf(stderr, "Error: Cannot update value of array symbol %s directly.\n", name);
        return;
    }

    if (symbol->value != NULL)
    {
        free(symbol->value);
    }
    symbol->value = strdup(value);
    printf("Updated symbol %s with new value: %s\n", name, value);
}

// Retrieve a symbol's value from the current or nearest outer scope
const char *getSymbolValue(SymbolTable *symbolTable, const char *name)
{
    Symbol *symbol = findSymbol(symbolTable, name);

    if (symbol == NULL)
    {
        fprintf(stderr, "Error: Symbol %s not found in the current or outer scopes.\n", name);
        return NULL;
    }
    if (symbol->isArray)
    {
        fprintf(stderr, "Error: Cannot get value of array symbol %s directly.\n", name);
        return NULL;
    }

    return symbol->value;
}

// Retrieve a symbol table at a specific depth (scope level)
SymbolTable *getSymbolTableAtDepth(SymbolTable *symTab, int scope)
{
    int currentDepth = 0;

    // Traverse back through previous scopes until reaching the desired depth
    while (symTab != NULL && currentDepth < scope)
    {
        symTab = symTab->prev;
        currentDepth++;
    }

    // Check if the desired depth was reached
    if (currentDepth == scope)
    {
        return symTab; // Return the symbol table at the specified depth
    }
    else
    {
        fprintf(stderr, "Error: Scope level %d exceeds available scope levels.\n", scope);
        return NULL; // Scope level is out of bounds
    }
}

void collectAllSymbols(SymbolTable *symTab, Symbol ***symbolList, int *symbolCount, int *symbolCapacity)
{
    if (symTab == NULL)
        return;

    // Traverse the symbol table's buckets
    for (int i = 0; i < symTab->size; i++)
    {
        Symbol *symbol = symTab->table[i];
        while (symbol != NULL)
        {
            // Check if we need to expand the symbol list array
            if (*symbolCount >= *symbolCapacity)
            {
                *symbolCapacity *= 2;
                *symbolList = realloc(*symbolList, *symbolCapacity * sizeof(Symbol *));
                if (*symbolList == NULL)
                {
                    fprintf(stderr, "Error: Memory allocation failed while expanding symbol list.\n");
                    exit(1);
                }
            }

            // Add symbol to the list
            (*symbolList)[(*symbolCount)++] = symbol;

            symbol = symbol->next;
        }
    }

    // Recursively collect symbols from inner scopes
    if (symTab->next != NULL)
    {
        collectAllSymbols(symTab->next, symbolList, symbolCount, symbolCapacity);
    }
}

// Function to free all symbol tables starting from the given symbol table
void freeAllSymbolTables(SymbolTable *symbolTable)
{
    if (symbolTable == NULL)
        return;

    // Recursively free inner scopes first
    if (symbolTable->next != NULL)
    {
        freeAllSymbolTables(symbolTable->next);
        symbolTable->next = NULL; // Avoid double-free
    }

    // Free the current symbol table
    freeSymbolTable(symbolTable);
}

void setSymbolParamList(Symbol *symbol, ASTNode *paramList)
{
    if (symbol == NULL)
    {
        fprintf(stderr, "Error: Attempted to set paramList on a NULL symbol.\n");
        return;
    }

    if (!symbol->isFunction)
    {
        fprintf(stderr, "Error: Symbol %s is not a function and cannot have parameters.\n", symbol->name);
        return;
    }

    symbol->paramList = paramList;
}

Symbol *findSymbolInCurrentScope(SymbolTable *symbolTable, const char *name)
{
    if (symbolTable == NULL)
    {
        return NULL;
    }

    unsigned int index = hashFunction(name, symbolTable->size);
    Symbol *current = symbolTable->table[index];

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL; // Return NULL if not found in the current scope
}