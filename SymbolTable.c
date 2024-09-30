#include <string.h>
#include "SymbolTable.h"
#include <stdio.h>

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
        hash = (hash * 31) + (unsigned char)(*name);
        name++;
    }

    return hash % tableSize;
}

// Create a new symbol with the given name, type, and index
Symbol *createSymbol(const char *name, const char *type, int index)
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
    newSymbol->next = NULL;
    return newSymbol;
}

// Insert a symbol into the symbol table
void insertSymbol(SymbolTable *symbolTable, const char *name, const char *type)
{
    unsigned int index = hashFunction(name, symbolTable->size);
    Symbol *newSymbol = createSymbol(name, type, index);

    // Insert the symbol at the head of the linked list for this index
    if (symbolTable->table[index] == NULL)
    {
        symbolTable->table[index] = newSymbol;
        printf("Inserted symbol at new index: Name = %s, Type = %s, Index = %d\n", name, type, index);
    }
    else
    {
        newSymbol->next = symbolTable->table[index];
        symbolTable->table[index] = newSymbol;
        printf("Inserted symbol at existing index: Name = %s, Type = %s, Index = %d\n", name, type, index);
    }
}

// Find a symbol in the symbol table by name
Symbol *findSymbol(SymbolTable *symbolTable, const char *name)
{
    unsigned int index = hashFunction(name, symbolTable->size);
    printf("Finding symbol: Name = %s, Index = %u\n", name, index);
    Symbol *current = symbolTable->table[index];

    // Traverse the linked list to find the symbol
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            printf("Symbol found: Name = %s, Type = %s, Index = %d\n", current->name, current->type, index);
            return current;
        }
        current = current->next;
    }
    printf("Symbol not found: Name = %s\n", name);
    return NULL; // Return NULL if the symbol is not found
}

// Free the memory used by the symbol table
void freeSymbolTable(SymbolTable *symbolTable)
{
    printf("Freeing symbol table...\n");
    for (int i = 0; i < symbolTable->size; i++)
    {
        Symbol *current = symbolTable->table[i];
        while (current != NULL)
        {
            Symbol *temp = current;
            printf("Freeing symbol: Name = %s, Type = %s, Index = %d\n", temp->name, temp->type, temp->index);
            current = current->next;
            free(temp->name);
            free(temp->type);
            free(temp);
        }
    }
    free(symbolTable->table);
    free(symbolTable);
    printf("Symbol table freed successfully.\n");
}

// Function to create a new symbol table
SymbolTable *createSymbolTable(int size)
{
    printf("Creating symbol table with size: %d\n", size);
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
        return 0;
    }

    // Initialize all table entries to NULL
    for (int i = 0; i < size; i++)
    {
        newTable->table[i] = 0;
    }

    printf("Symbol table created successfully.\n");
    return newTable;
}