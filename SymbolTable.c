#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "SymbolTable.h"
#include "Array.h"

// Hash function: Hash based on the ASCII values of the string's characters
unsigned int hashFunction(const char *name, int tableSize)
{
    // Check if the name is NULL
    if (name == NULL)
    {
        fprintf(stderr, "Error: Tried to hash a NULL name.\n");
        return 0; // Or handle it appropriately
    }

    unsigned long hash = 0;
    while (*name) // Proceed only if name is non-NULL
    {
        hash = abs((hash * 31) + (unsigned char)(*name));
        name++;
    }

    return hash % tableSize;
}

// Create a new symbol with the given name, type, and index
Symbol *createSymbol(const char *name, const char *type, int index, bool isArray, Array *arrayInfo)
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
    newSymbol->arrayInfo = arrayInfo;
    newSymbol->next = NULL;
    return newSymbol;
}

// Insert a symbol into the symbol table
void insertSymbol(SymbolTable *symbolTable, const char *name, const char *type, bool isArray, Array *arrayInfo)
{
    // Check if the symbol already exists
    Symbol *existingSymbol = findSymbol(symbolTable, name);
    if (existingSymbol != NULL)
    {
        fprintf(stderr, "Error: Symbol %s is already declared.\n", name);
        return;
    }

    unsigned int index = hashFunction(name, symbolTable->size);
    Symbol *newSymbol = createSymbol(name, type, index, isArray, arrayInfo);

    // Insert the symbol at the head of the linked list for this index
    newSymbol->next = symbolTable->table[index];
    symbolTable->table[index] = newSymbol;

    printf("Inserted symbol: Name = %s, Type = %s, Index = %d, isArray = %s\n",
           name, type, index, isArray ? "true" : "false");
}

// Find a symbol in the symbol table by name
Symbol *findSymbol(SymbolTable *symbolTable, const char *name)
{
    // Ensure the symbolTable and table are properly initialized
    if (symbolTable == NULL || symbolTable->table == NULL)
    {
        fprintf(stderr, "Error: Symbol table is NULL or uninitialized.\n");
        return NULL;
    }

    unsigned int index = hashFunction(name, symbolTable->size);
    if (index >= symbolTable->size) {
        fprintf(stderr, "Error: Hash index out of bounds.\n");
        return NULL;
    }

    Symbol *current = symbolTable->table[index];

    // Traverse the linked list to find the symbol
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL; // Return NULL if the symbol is not found
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

            // Free the symbol's name
            if (symbol->name != NULL)
            {
                free(symbol->name);
                symbol->name = NULL;
            }

            // Free the symbol's type
            if (symbol->type != NULL)
            {
                free(symbol->type);
                symbol->type = NULL;
            }

            // Free the symbol's value
            if (symbol->value != NULL)
            {
                free(symbol->value);
                symbol->value = NULL;
            }

            // If the symbol is an array, free the arrayInfo
            if (symbol->isArray && symbol->arrayInfo != NULL)
            {
                freeArray(symbol->arrayInfo);
                symbol->arrayInfo = NULL;
            }

            // Free the symbol itself
            free(symbol);
            symbol = next;
        }
    }

    // Free the table array and the symbol table itself
    free(symbolTable->table);
    symbolTable->table = NULL;
    free(symbolTable);
    printf("Successfully freed symbol table\n");
}

// Function to create a new symbol table
SymbolTable *createSymbolTable(int size)
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

    // Initialize all table entries to NULL
    for (int i = 0; i < size; i++)
    {
        newTable->table[i] = NULL;
    }

    return newTable;
}

void updateSymbolValue(SymbolTable *symbolTable, const char *name, const char *value)
{
    // Find the symbol in the table
    Symbol *symbol = findSymbol(symbolTable, name);

    if (symbol == NULL)
    {
        // If the symbol is not found, print an error or handle it accordingly
        fprintf(stderr, "Error: Symbol %s not found in the table, cannot update value.\n", name);
        return;
    }

    // If the symbol is an array, you should not update its value directly
    if (symbol->isArray)
    {
        fprintf(stderr, "Error: Cannot update value of array symbol %s directly.\n", name);
        return;
    }

    // Free the old value if it exists to avoid memory leaks
    if (symbol->value != NULL)
    {
        free(symbol->value);
    }

    // Allocate memory for the new value and copy it
    symbol->value = strdup(value);
    printf("Updated symbol %s with new value: %s\n", name, value);
}

const char *getSymbolValue(SymbolTable *symbolTable, const char *name)
{
    // Find the symbol in the table
    Symbol *symbol = findSymbol(symbolTable, name);

    if (symbol == NULL)
    {
        // If the symbol is not found, print an error or return NULL
        fprintf(stderr, "Error: Symbol %s not found in the table.\n", name);
        return NULL;
    }

    // If the symbol is an array, you should not get its value directly
    if (symbol->isArray)
    {
        fprintf(stderr, "Error: Cannot get value of array symbol %s directly.\n", name);
        return NULL;
    }

    // Return the value of the symbol
    return symbol->value;
}