#include <string.h>
#include <math.h>
#include <stdio.h>
#include "SymbolTable.h"

// Hash function: Hash based on the ASCII values of the string's characters
unsigned int hashFunction(const char *name, int tableSize)
{
    unsigned long hash = 0;
    while (*name)
    {
        hash = abs((hash * 31) + (unsigned char)(*name)); // 31 is used as a multiplier for hashing
        name++;
    }
    return hash % tableSize; // Use the prime number size for better distribution
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
    newSymbol->value = NULL;
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
<<<<<<< HEAD
            printf("Symbol found: Name = %s, Type = %s, Index = %d, Value = %s\n",
                   current->name,
                   current->type,
                   index,
                   current->value);
=======
            printf("Symbol found: Name = %s, Type = %s, Index = %d, Value = %s\n", current->name, current->type, index, current->value ? current->value : "NULL");
>>>>>>> 8975319 (did some things, including absolute value in hash method)
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
    if (symbolTable == NULL)
    {
        printf("Symbol table is NULL. Nothing to free.\n");
        return;
    }

    printf("Starting to free symbol table of size %d\n", symbolTable->size);

    for (int i = 0; i < symbolTable->size; i++)
    {
        Symbol *symbol = symbolTable->table[i];

        // Print current bucket being processed
        printf("Processing bucket %d\n", i);

        while (symbol != NULL)
        {
            printf("Freeing symbol: %s\n", symbol->name ? symbol->name : "NULL");

            Symbol *next = symbol->next;

            // Safely free the symbol's name if it exists
            if (symbol->name != NULL)
            {
                printf("Freeing symbol name: %s\n", symbol->name);
                free(symbol->name);
                symbol->name = NULL; // Avoid double free
            }

            // Safely free the symbol's value if it exists
            if (symbol->value != NULL) {
                printf("Freeing symbol value: %s\n", symbol->value);
                free(symbol->value);
                symbol->value = NULL; // Avoid double free
            }

            // Free the symbol itself
            printf("Freeing symbol structure\n");
            free(symbol);
            symbol = next;

            // Print status of next symbol to process
            if (symbol != NULL)
            {
                printf("Next symbol in bucket: %s\n", symbol->name ? symbol->name : "NULL");
            }
            else
            {
                printf("No more symbols in this bucket.\n");
            }
        }
    }

    // Free the table array and the symbol table itself
    printf("Freeing the symbol table array.\n");
    free(symbolTable->table);
    symbolTable->table = NULL; // Avoid double free

    printf("Freeing the symbol table structure itself.\n");
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

    // Return the value of the symbol
    return symbol->value;
}
