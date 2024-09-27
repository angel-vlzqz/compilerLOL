#include <string.h>
#include "SymbolTable.h"
#include <stdio.h>

// Hash function: Hash based on the ASCII values of the string's characters
unsigned int hashFunction(const char *name, int tableSize) {
    unsigned long hash = 0;
    while (*name) {
        hash = (hash * 31) + (unsigned char)(*name); // 31 is used as a multiplier for hashing
        name++;
    }
    return hash % tableSize;  // Use the prime number size for better distribution
}


// Create a new symbol with the given name, type, and index
Symbol* createSymbol(const char *name, const char *type, int index) {
    Symbol *newSymbol = (Symbol*)malloc(sizeof(Symbol));
    if (!newSymbol) {
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
void insertSymbol(SymbolTable *symbolTable, const char *name, const char *type) {
    unsigned int index = hashFunction(name, symbolTable->size);
    Symbol *newSymbol = createSymbol(name, type, index);
    
    // Insert the symbol at the head of the linked list for this index
    if (symbolTable->table[index] == NULL) {
        symbolTable->table[index] = newSymbol;
    } else {
        newSymbol->next = symbolTable->table[index];
        symbolTable->table[index] = newSymbol;
    }
    printf("Inserted symbol: Name = %s, Type = %s, Index = %d\n", name, type, index);
}

// Find a symbol in the symbol table by name
Symbol* findSymbol(SymbolTable *symbolTable, const char *name) {
    unsigned int index = hashFunction(name, symbolTable->size);
    Symbol *current = symbolTable->table[index];
    
    // Traverse the linked list to find the symbol
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;  // Return NULL if the symbol is not found
}

// Free the memory used by the symbol table
void freeSymbolTable(SymbolTable *symbolTable) {
    for (int i = 0; i < symbolTable->size; i++) {
        Symbol *current = symbolTable->table[i];
        while (current != NULL) {
            Symbol *temp = current;
            current = current->next;
            free(temp->name);
            free(temp->type);
            free(temp);
        }
    }
    free(symbolTable->table);
    free(symbolTable);
}

// Function to create a new symbol table
SymbolTable* createSymbolTable(int size) {
    SymbolTable* newTable = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!newTable) return 0;

    newTable->size = size;
    newTable->table = (Symbol**)malloc(sizeof(Symbol*) * size);
    if (!newTable->table) {
        free(newTable);
        return 0;
    }

    // Initialize all table entries to NULL
    for (int i = 0; i < size; i++) {
        newTable->table[i] = 0;
    }

    return newTable;
}