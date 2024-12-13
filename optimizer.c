// optimizer.c
#include "optimizer.h"
#include "utils.h" // Ensure this is included to access isVariable
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Define DEBUG flag (Set to 1 to enable debug prints, 0 to disable)
#define DEBUG 1 // Enable debugging for detailed output

#if DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

// ==================== Hash Table Implementation ====================

// Define the size of the hash table
#define HASH_TABLE_SIZE 256

// Structure for hash table entries
typedef struct HashEntry {
    char *key;               // Variable name or array element (e.g., "arr[3]")
    char *value;             // Constant value
    struct HashEntry *next;  // Next entry in the chain (for collision handling)
} HashEntry;

// Structure for the hash table
typedef struct HashTable {
    HashEntry *buckets[HASH_TABLE_SIZE];
} HashTable;

// Function Prototypes for Helper Functions
static HashTable* createHashTable();
static void freeHashTableStruct(HashTable *table);
static unsigned int hashFunction(const char *key);
static bool setConstant(HashTable *table, const char *key, const char *value);
static const char* getConstant(HashTable *table, const char *key);
static bool invalidateConstant(HashTable *table, const char *key);
static bool replaceArrayIndexInField(char **field, HashTable *knownVars);

// ==================== Hash Table Functions ====================

// Create a new hash table
static HashTable* createHashTable() {
    HashTable *table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) {
        perror("Failed to allocate memory for hash table");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
        table->buckets[i] = NULL;
    return table;
}

// Free the hash table
static void freeHashTableStruct(HashTable *table) {
    if (!table) return;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *entry = table->buckets[i];
        while (entry) {
            HashEntry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(table);
}

// Simple hash function (djb2)
static unsigned int hashFunction(const char *key) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*key++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % HASH_TABLE_SIZE;
}

// Set or update a constant value for a variable
static bool setConstant(HashTable *table, const char *key, const char *value) {
    if (!table || !key || !value)
        return false;
    unsigned int index = hashFunction(key);
    HashEntry *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Update existing entry
            free(entry->value);
            entry->value = strdup(value);
            if (!entry->value) {
                perror("Failed to duplicate value string");
                exit(EXIT_FAILURE);
            }
            return true;
        }
        entry = entry->next;
    }
    // Create a new entry
    HashEntry *newEntry = (HashEntry*)malloc(sizeof(HashEntry));
    if (!newEntry) {
        perror("Failed to allocate memory for hash entry");
        exit(EXIT_FAILURE);
    }
    newEntry->key = strdup(key);
    if (!newEntry->key) {
        perror("Failed to duplicate key string");
        free(newEntry);
        exit(EXIT_FAILURE);
    }
    newEntry->value = strdup(value);
    if (!newEntry->value) {
        perror("Failed to duplicate value string");
        free(newEntry->key);
        free(newEntry);
        exit(EXIT_FAILURE);
    }
    newEntry->next = table->buckets[index];
    table->buckets[index] = newEntry;
    return true;
}

// Get the constant value for a variable
static const char* getConstant(HashTable *table, const char *key) {
    if (!table || !key)
        return NULL;
    unsigned int index = hashFunction(key);
    HashEntry *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0)
            return entry->value;
        entry = entry->next;
    }
    return NULL;
}

// Invalidate a variable's constant value
static bool invalidateConstant(HashTable *table, const char *key) {
    if (!table || !key)
        return false;
    unsigned int index = hashFunction(key);
    HashEntry *entry = table->buckets[index];
    HashEntry *prev = NULL;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev)
                prev->next = entry->next;
            else
                table->buckets[index] = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            return true;
        }
        prev = entry;
        entry = entry->next;
    }
    return false;
}

// ==================== Utility Functions ====================

// Check if a string is a numeric constant (integer or float)
static bool isNumericConstant(const char *str)
{
    if (!str || *str == '\0')
        return false;
    const char *p = str;
    if (*p == '+' || *p == '-')
        p++;

    bool hasDigit = false;
    bool hasDot = false;
    while (*p)
    {
        if (*p == '.')
        {
            if (hasDot)
                return false;
            hasDot = true;
        }
        else if (!isdigit((unsigned char)*p))
        {
            return false;
        }
        else
        {
            hasDigit = true;
        }
        p++;
    }

    return hasDigit;
}

// Parse a string to double, handling errors
static double parseDouble(const char *str)
{
    errno = 0;
    double val = strtod(str, NULL);
    return val;
}

// ==================== Helper Functions ====================

// Helper Function to Process a TAC Field for Array Accesses
static bool replaceArrayIndexInField(char **field, HashTable *knownVars) {
    bool changed = false;
    if (!field || !(*field))
        return changed;

    char *p = strstr(*field, "[");
    while (p != NULL) {
        char *start = p;
        char *end = strchr(p, ']');
        if (!end)
            break; // Malformed array access

        // Extract index between '[' and ']'
        size_t index_len = end - p - 1;
        if (index_len <= 0)
            break; // Empty index

        char index[256];
        if (index_len >= sizeof(index)) {
            fprintf(stderr, "Index length exceeds buffer size in replaceArrayIndexInField.\n");
            return changed;
        }
        strncpy(index, p + 1, index_len);
        index[index_len] = '\0';

        // Trim whitespace
        char *index_trimmed = index;
        while (isspace((unsigned char)*index_trimmed)) index_trimmed++;
        char *end_trimmed = index_trimmed + strlen(index_trimmed) - 1;
        while (end_trimmed > index_trimmed && isspace((unsigned char)*end_trimmed)) {
            *end_trimmed = '\0';
            end_trimmed--;
        }

        // Check if index is a variable with a known constant
        if (isVariable(index_trimmed)) {
            const char *constVal = getConstant(knownVars, index_trimmed);
            if (constVal && isNumericConstant(constVal)) {
                // Replace variable index with constant
                size_t prefix_len = p - *field + 1; // Include '['
                size_t suffix_len = strlen(end + 1); // After ']'
                size_t new_field_size = prefix_len + strlen(constVal) + 1 + suffix_len + 1; // +1 for ']', +1 for '\0'

                char *new_field = (char*)malloc(new_field_size);
                if (!new_field) {
                    perror("Failed to allocate memory for new field");
                    exit(EXIT_FAILURE);
                }

                // Build the new field string
                strncpy(new_field, *field, prefix_len); // Copy up to '[' including '['
                new_field[prefix_len] = '\0';
                strcat(new_field, constVal);
                strcat(new_field, "]");
                strcat(new_field, end + 1);

                // Replace the field with the new string
                free(*field);
                *field = strdup(new_field);
                if (!(*field)) {
                    perror("Failed to duplicate new field string");
                    free(new_field);
                    exit(EXIT_FAILURE);
                }
                free(new_field);

                DEBUG_PRINT("ArrayIndexPropagation: Replaced index variable '%s' with constant '%s' in field.\n", index_trimmed, constVal);
                changed = true;

                // Continue searching for other array accesses in the field
                p = strstr(*field, "[");
                continue;
            }
        }

        // Move past this array access
        p = strstr(end, "[");
    }

    return changed;
}

// ==================== Optimization Passes ====================

// Constant Propagation: Replace variables with known constant values
int constantPropagation(TAC **head)
{
    int changes = 0;
    HashTable *knownVars = createHashTable();
    HashTable *knownArrs = createHashTable(); // Separate hash table for array elements

    for (TAC *current = *head; current; current = current->next) {
        // Stop at function boundaries or calls
        if (current->op && (strcmp(current->op, "prologue") == 0 ||
                            strcmp(current->op, "epilogue") == 0 ||
                            strcmp(current->op, "call") == 0))
        {
            // Clear known constants at boundaries
            freeHashTableStruct(knownVars);
            freeHashTableStruct(knownArrs);
            knownVars = createHashTable();
            knownArrs = createHashTable();
            continue;
        }

        // Propagate constants into arg1/arg2 if known
        if (current->arg1) {
            const char *vc = getConstant(knownVars, current->arg1);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg1) != 0) {
                DEBUG_PRINT("Constant Propagation: %s -> %s\n", current->arg1, vc);
                free(current->arg1);
                current->arg1 = strdup(vc);
                if (!current->arg1) {
                    perror("Failed to duplicate constant value for arg1");
                    exit(EXIT_FAILURE);
                }
                changes++;
            }
        }

        if (current->arg2) {
            const char *vc = getConstant(knownVars, current->arg2);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg2) != 0) {
                DEBUG_PRINT("Constant Propagation: %s -> %s\n", current->arg2, vc);
                free(current->arg2);
                current->arg2 = strdup(vc);
                if (!current->arg2) {
                    perror("Failed to duplicate constant value for arg2");
                    exit(EXIT_FAILURE);
                }
                changes++;
            }
        }

        // Handle array load and store to propagate indices and values
        if (current->op && strcmp(current->op, "=[]") == 0) {
            // var = arr[index]
            // Make index a constant if possible
            if (current->arg2) {
                const char *idxConst = getConstant(knownVars, current->arg2);
                if (idxConst && isNumericConstant(idxConst) && strcmp(idxConst, current->arg2) != 0) {
                    DEBUG_PRINT("Constant Propagation: Replacing array index %s with constant %s\n", current->arg2, idxConst);
                    free(current->arg2);
                    current->arg2 = strdup(idxConst);
                    if (!current->arg2) {
                        perror("Failed to duplicate constant value for array index");
                        exit(EXIT_FAILURE);
                    }
                    changes++;
                }
            }

            if (isNumericConstant(current->arg2)) {
                // Construct the key for the array element
                char arrKey[512];
                snprintf(arrKey, sizeof(arrKey), "%s[%s]", current->arg1, current->arg2); // e.g., arr[3]

                DEBUG_PRINT("Constant Propagation: Looking up arrConstant for key: %s\n", arrKey);
                const char *ac = getConstant(knownArrs, arrKey);
                if (ac && isNumericConstant(ac)) {
                    // Replace with var = ac
                    DEBUG_PRINT("Constant Propagation: Replacing array access %s = %s\n", current->result, ac);
                    free(current->op);
                    current->op = strdup("=");
                    if (!current->op) {
                        perror("Failed to duplicate '=' operation");
                        exit(EXIT_FAILURE);
                    }
                    free(current->arg1);
                    current->arg1 = strdup(ac);
                    if (!current->arg1) {
                        perror("Failed to duplicate constant value for arg1");
                        exit(EXIT_FAILURE);
                    }
                    free(current->arg2);
                    current->arg2 = NULL;
                    changes++;

                    if (current->result) {
                        setConstant(knownVars, current->result, ac);
                    }
                } else {
                    // Not known, invalidate var
                    if (current->result) {
                        DEBUG_PRINT("Constant Propagation: Invalidating constant for variable: %s\n", current->result);
                        invalidateConstant(knownVars, current->result);
                    }
                }
            } else {
                if (current->result) {
                    DEBUG_PRINT("Constant Propagation: Invalidating constant for variable due to non-constant index: %s\n", current->result);
                    invalidateConstant(knownVars, current->result);
                }
            }
        }
        else if (current->op && strcmp(current->op, "[]=") == 0) {
            // arr[index] = value
            if (current->arg2) {
                const char *idxConst = getConstant(knownVars, current->arg2);
                if (idxConst && isNumericConstant(idxConst) && strcmp(idxConst, current->arg2) != 0) {
                    DEBUG_PRINT("Constant Propagation: Replacing array index %s with constant %s\n", current->arg2, idxConst);
                    free(current->arg2);
                    current->arg2 = strdup(idxConst);
                    if (!current->arg2) {
                        perror("Failed to duplicate constant value for array index");
                        exit(EXIT_FAILURE);
                    }
                    changes++;
                }
            }

            if (isNumericConstant(current->arg2)) {
                char arrKey[512];
                snprintf(arrKey, sizeof(arrKey), "%s[%s]", current->arg1, current->arg2); // e.g., arr[3]
                if (current->result && isNumericConstant(current->result)) {
                    DEBUG_PRINT("Constant Propagation: Setting arrConstant %s = %s\n", arrKey, current->result);
                    setConstant(knownArrs, arrKey, current->result);
                } else {
                    // If result is a var, check if var is known constant
                    if (current->result) {
                        const char *vc = getConstant(knownVars, current->result);
                        if (vc && isNumericConstant(vc)) {
                            DEBUG_PRINT("Constant Propagation: Setting arrConstant from variable %s = %s\n", arrKey, vc);
                            setConstant(knownArrs, arrKey, vc);
                        } else {
                            DEBUG_PRINT("Constant Propagation: Invalidating arrConstant %s\n", arrKey);
                            invalidateConstant(knownArrs, arrKey);
                        }
                    }
                }
            } else {
                // Unknown index; no action needed
                DEBUG_PRINT("Constant Propagation: Unknown array index for store: %s[%s]\n", current->arg1, current->arg2);
            }
        }
        else if (current->op && strcmp(current->op, "=") == 0) {
            // var = something
            if (current->arg1 && isNumericConstant(current->arg1)) {
                if (current->result) {
                    DEBUG_PRINT("Constant Propagation: Setting varConstant %s = %s\n", current->result, current->arg1);
                    setConstant(knownVars, current->result, current->arg1);
                }
            } else {
                if (current->arg1) {
                    const char *vc = getConstant(knownVars, current->arg1);
                    if (vc && isNumericConstant(vc)) {
                        DEBUG_PRINT("Constant Propagation: Propagating varConstant %s = %s\n", current->result, vc);
                        free(current->arg1);
                        current->arg1 = strdup(vc);
                        if (!current->arg1) {
                            perror("Failed to duplicate propagated constant for arg1");
                            exit(EXIT_FAILURE);
                        }
                        changes++;
                        if (current->result) {
                            setConstant(knownVars, current->result, vc);
                        }
                    } else {
                        if (current->result) {
                            DEBUG_PRINT("Constant Propagation: Invalidating varConstant %s\n", current->result);
                            invalidateConstant(knownVars, current->result);
                        }
                    }
                } else {
                    if (current->result) {
                        DEBUG_PRINT("Constant Propagation: Invalidating varConstant %s\n", current->result);
                        invalidateConstant(knownVars, current->result);
                    }
                }
            }
        }
        else {
            // For other ops that define a result, not guaranteed constant
            if (current->result && strcmp(current->op, "[]=") != 0 &&
                strcmp(current->op, "=[]") != 0 && strcmp(current->op, "=") != 0)
            {
                DEBUG_PRINT("Constant Propagation: Invalidating varConstant %s due to operation %s\n", current->result, current->op);
                invalidateConstant(knownVars, current->result);
            }
        }

        // Final propagation step for arguments after possible updates
        if (current->arg1) {
            const char *vc = getConstant(knownVars, current->arg1);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg1) != 0) {
                DEBUG_PRINT("Constant Propagation: Final propagation %s -> %s\n", current->arg1, vc);
                free(current->arg1);
                current->arg1 = strdup(vc);
                if (!current->arg1) {
                    perror("Failed to duplicate final propagated constant for arg1");
                    exit(EXIT_FAILURE);
                }
                changes++;
            }
        }
        if (current->arg2) {
            const char *vc = getConstant(knownVars, current->arg2);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->arg2) != 0) {
                DEBUG_PRINT("Constant Propagation: Final propagation %s -> %s\n", current->arg2, vc);
                free(current->arg2);
                current->arg2 = strdup(vc);
                if (!current->arg2) {
                    perror("Failed to duplicate final propagated constant for arg2");
                    exit(EXIT_FAILURE);
                }
                changes++;
            }
        }

        // Check for []= result propagation
        if (current->op && strcmp(current->op, "[]=") == 0 && current->result) {
            const char *vc = getConstant(knownVars, current->result);
            if (vc && isNumericConstant(vc) && strcmp(vc, current->result) != 0) {
                DEBUG_PRINT("Constant Propagation: Propagating []= result %s -> %s\n", current->result, vc);
                free(current->result);
                current->result = strdup(vc);
                if (!current->result) {
                    perror("Failed to duplicate propagated constant for result");
                    exit(EXIT_FAILURE);
                }
                changes++;
            }
        }

        // Additional step: Ensure array indices are constants if possible
        if (current->op && (strcmp(current->op, "=[]") == 0 || strcmp(current->op, "[]=") == 0)) {
            if (current->arg2 && !isNumericConstant(current->arg2)) {
                // Try once more
                const char *vc = getConstant(knownVars, current->arg2);
                if (vc && isNumericConstant(vc)) {
                    DEBUG_PRINT("Constant Propagation: Additional propagation for array index %s -> %s\n", current->arg2, vc);
                    free(current->arg2);
                    current->arg2 = strdup(vc);
                    if (!current->arg2) {
                        perror("Failed to duplicate additional propagated constant for array index");
                        exit(EXIT_FAILURE);
                    }
                    changes++;
                }
            }
        }
    }

    // Free the hash tables
    freeHashTableStruct(knownVars);
    freeHashTableStruct(knownArrs);

    return changes;
}

// Array Index Propagation: Replace variable indices in array accesses with constants
int arrayIndexPropagation(TAC **head)
{
    int changes = 0;
    HashTable *knownVars = createHashTable();

    for (TAC *current = *head; current; current = current->next) {
        // Stop at function boundaries or calls
        if (current->op && (strcmp(current->op, "prologue") == 0 ||
                            strcmp(current->op, "epilogue") == 0 ||
                            strcmp(current->op, "call") == 0))
        {
            // Clear known constants at boundaries
            freeHashTableStruct(knownVars);
            knownVars = createHashTable();
            continue;
        }

        // Track variables with known constants
        if (current->op && strcmp(current->op, "=") == 0) {
            if (current->arg1 && isNumericConstant(current->arg1)) {
                if (current->result) {
                    setConstant(knownVars, current->result, current->arg1);
                    DEBUG_PRINT("ArrayIndexPropagation: Setting varConstant '%s' = '%s'\n", current->result, current->arg1);
                }
            }
            else if (current->arg1 && getConstant(knownVars, current->arg1)) {
                const char *constVal = getConstant(knownVars, current->arg1);
                if (isNumericConstant(constVal)) {
                    if (current->result) {
                        setConstant(knownVars, current->result, constVal);
                        DEBUG_PRINT("ArrayIndexPropagation: Propagating varConstant '%s' = '%s'\n", current->result, constVal);
                    }
                }
                else {
                    if (current->result) {
                        invalidateConstant(knownVars, current->result);
                        DEBUG_PRINT("ArrayIndexPropagation: Invalidating varConstant '%s'\n", current->result);
                    }
                }
            }
            else {
                if (current->result) {
                    invalidateConstant(knownVars, current->result);
                    DEBUG_PRINT("ArrayIndexPropagation: Invalidating varConstant '%s'\n", current->result);
                }
            }
        }
        else {
            // For other operations, if they define a variable, invalidate its constant
            if (current->result && strcmp(current->op, "=[]") != 0 &&
                strcmp(current->op, "[]=") != 0 && strcmp(current->op, "=") != 0)
            {
                invalidateConstant(knownVars, current->result);
                DEBUG_PRINT("ArrayIndexPropagation: Invalidating varConstant '%s' due to operation '%s'\n", current->result, current->op);
            }
        }

        // Now, specifically look for array accesses in all fields and replace variable indices with constants
        // Fields to check: op, arg1, arg2, result
        // Typically, array accesses are in arg1 or arg2, but checking all for robustness

        // Check 'op' field for array accesses like "arr[i]"
        if (current->op) {
            bool changed = replaceArrayIndexInField(&(current->op), knownVars);
            if (changed)
                changes++;
        }

        // Check 'arg1' field
        if (current->arg1) {
            bool changed = replaceArrayIndexInField(&(current->arg1), knownVars);
            if (changed)
                changes++;
        }

        // Check 'arg2' field
        if (current->arg2) {
            bool changed = replaceArrayIndexInField(&(current->arg2), knownVars);
            if (changed)
                changes++;
        }

        // Check 'result' field
        if (current->result) {
            bool changed = replaceArrayIndexInField(&(current->result), knownVars);
            if (changed)
                changes++;
        }
    }

    // Free the hash table
    freeHashTableStruct(knownVars);

    return changes;
}

// Constant Folding: Replace operations with constant results
int constantFolding(TAC **head)
{
    int changes = 0;
    TAC *current = *head;

    while (current != NULL)
    {
        // Skip complex operations or those that must not be folded
        if (current->op && (
            strcmp(current->op, "prologue") == 0 ||
            strcmp(current->op, "epilogue") == 0 ||
            strcmp(current->op, "call") == 0 ||
            strcmp(current->op, "return") == 0 ||
            strcmp(current->op, "ifFalse") == 0 ||
            strcmp(current->op, "goto") == 0 ||
            strcmp(current->op, "label") == 0 ||
            strcmp(current->op, "param") == 0 ||
            strcmp(current->op, "write") == 0 ||
            strcmp(current->op, "write_float") == 0 ||
            strcmp(current->op, "=[]") == 0 ||
            strcmp(current->op, "[]=") == 0 ||
            strcmp(current->op, "!") == 0 ||
            strcmp(current->op, "&&") == 0 ||
            strcmp(current->op, "||") == 0 ||
            strcmp(current->op, "==") == 0 ||
            strcmp(current->op, "!=") == 0 ||
            strcmp(current->op, "<") == 0 ||
            strcmp(current->op, "<=") == 0 ||
            strcmp(current->op, ">") == 0 ||
            strcmp(current->op, ">=") == 0))
        {
            current = current->next;
            continue;
        }

        // Handle integer operations
        if (current->op && (strcmp(current->op, "+") == 0 ||
                            strcmp(current->op, "-") == 0 ||
                            strcmp(current->op, "*") == 0 ||
                            strcmp(current->op, "/") == 0))
        {
            if (isNumericConstant(current->arg1) && isNumericConstant(current->arg2))
            {
                bool isFloatArg = (strchr(current->arg1, '.') != NULL) || (strchr(current->arg2, '.') != NULL);
                if (!isFloatArg)
                {
                    int operand1 = atoi(current->arg1);
                    int operand2 = atoi(current->arg2);
                    int result = 0;
                    bool divisionByZero = false;

                    if (strcmp(current->op, "+") == 0)
                        result = operand1 + operand2;
                    else if (strcmp(current->op, "-") == 0)
                        result = operand1 - operand2;
                    else if (strcmp(current->op, "*") == 0)
                        result = operand1 * operand2;
                    else if (strcmp(current->op, "/") == 0)
                    {
                        if (operand2 == 0)
                            divisionByZero = true;
                        else
                            result = operand1 / operand2;
                    }

                    if (!divisionByZero)
                    {
                        char resultStr[20];
                        snprintf(resultStr, sizeof(resultStr), "%d", result);

                        free(current->arg1);
                        free(current->arg2);
                        free(current->op);

                        current->arg1 = strdup(resultStr);
                        if (!current->arg1) {
                            perror("Failed to duplicate folded integer result for arg1");
                            exit(EXIT_FAILURE);
                        }
                        current->op = strdup("=");
                        if (!current->op) {
                            perror("Failed to duplicate '=' operation in constant folding");
                            exit(EXIT_FAILURE);
                        }
                        current->arg2 = NULL;

                        DEBUG_PRINT("Constant Folding: Replaced operation with %s = %s\n", current->result, resultStr);
                        changes++;
                    }
                }
                else
                {
                    double operand1 = parseDouble(current->arg1);
                    double operand2 = parseDouble(current->arg2);
                    double result = 0.0;
                    bool divisionByZero = false;

                    if (strcmp(current->op, "+") == 0)
                        result = operand1 + operand2;
                    else if (strcmp(current->op, "-") == 0)
                        result = operand1 - operand2;
                    else if (strcmp(current->op, "*") == 0)
                        result = operand1 * operand2;
                    else if (strcmp(current->op, "/") == 0)
                    {
                        if (operand2 == 0.0)
                            divisionByZero = true;
                        else
                            result = operand1 / operand2;
                    }

                    if (!divisionByZero)
                    {
                        char resultStr[64];
                        snprintf(resultStr, sizeof(resultStr), "%f", result);

                        free(current->arg1);
                        free(current->arg2);
                        free(current->op);

                        current->arg1 = strdup(resultStr);
                        if (!current->arg1) {
                            perror("Failed to duplicate folded floating result for arg1");
                            exit(EXIT_FAILURE);
                        }
                        current->op = strdup("=");
                        if (!current->op) {
                            perror("Failed to duplicate '=' operation in constant folding");
                            exit(EXIT_FAILURE);
                        }
                        current->arg2 = NULL;

                        DEBUG_PRINT("Constant Folding: Replaced floating operation with %s = %s\n", current->result, resultStr);
                        changes++;
                    }
                }
            }
        }
        // Handle floating-point operations
        else if (current->op && (strcmp(current->op, "fadd") == 0 ||
                                 strcmp(current->op, "fsub") == 0 ||
                                 strcmp(current->op, "fmul") == 0 ||
                                 strcmp(current->op, "fdiv") == 0))
        {
            if (isNumericConstant(current->arg1) && isNumericConstant(current->arg2))
            {
                double operand1 = parseDouble(current->arg1);
                double operand2 = parseDouble(current->arg2);
                double result = 0.0;
                bool divisionByZero = false;

                if (strcmp(current->op, "fadd") == 0)
                    result = operand1 + operand2;
                else if (strcmp(current->op, "fsub") == 0)
                    result = operand1 - operand2;
                else if (strcmp(current->op, "fmul") == 0)
                    result = operand1 * operand2;
                else if (strcmp(current->op, "fdiv") == 0)
                {
                    if (operand2 == 0.0)
                        divisionByZero = true;
                    else
                        result = operand1 / operand2;
                }

                if (!divisionByZero)
                {
                    char resultStr[64];
                    snprintf(resultStr, sizeof(resultStr), "%f", result);

                    free(current->arg1);
                    free(current->arg2);
                    free(current->op);

                    current->arg1 = strdup(resultStr);
                    if (!current->arg1) {
                        perror("Failed to duplicate folded floating result for arg1");
                        exit(EXIT_FAILURE);
                    }
                    current->op = strdup("=");
                    if (!current->op) {
                        perror("Failed to duplicate '=' operation in constant folding");
                        exit(EXIT_FAILURE);
                    }
                    current->arg2 = NULL;

                    DEBUG_PRINT("Constant Folding: Replaced floating operation with %s = %s\n", current->result, resultStr);
                    changes++;
                }
            }
        }

        current = current->next;
    }

    return changes;
}

// Copy Propagation: Replace copies with original variables
int copyPropagation(TAC **head)
{
    int changes = 0;
    TAC *current = *head;
    while (current != NULL)
    {
        if (current->op && strcmp(current->op, "=") == 0 && isVariable(current->arg1))
        {
            char *sourceVar = current->arg1;
            char *destVar = current->result;
            TAC *temp = current->next;

            while (temp != NULL)
            {
                if (temp->op && (strcmp(temp->op, "prologue") == 0 ||
                                 strcmp(temp->op, "epilogue") == 0 ||
                                 strcmp(temp->op, "call") == 0))
                    break;

                if (temp->arg1 && strcmp(temp->arg1, destVar) == 0)
                {
                    DEBUG_PRINT("Copy Propagation: Replacing %s with %s in arg1\n", destVar, sourceVar);
                    free(temp->arg1);
                    temp->arg1 = strdup(sourceVar);
                    if (!temp->arg1) {
                        perror("Failed to duplicate source variable in arg1");
                        exit(EXIT_FAILURE);
                    }
                    changes++;
                }
                if (temp->arg2 && strcmp(temp->arg2, destVar) == 0)
                {
                    DEBUG_PRINT("Copy Propagation: Replacing %s with %s in arg2\n", destVar, sourceVar);
                    free(temp->arg2);
                    temp->arg2 = strdup(sourceVar);
                    if (!temp->arg2) {
                        perror("Failed to duplicate source variable in arg2");
                        exit(EXIT_FAILURE);
                    }
                    changes++;
                }

                // If the destination is redefined, stop propagation
                if (temp->result && strcmp(temp->result, destVar) == 0)
                    break;

                temp = temp->next;
            }
        }
        current = current->next;
    }
    return changes;
}

// Dead Code Elimination: Remove instructions whose results are never used
int deadCodeElimination(TAC **head)
{
    int changes = 0;
    TAC *current = *head;
    TAC *prev = NULL;

    while (current != NULL)
    {
        if (current->op && (strcmp(current->op, "prologue") == 0 ||
                            strcmp(current->op, "epilogue") == 0))
        {
            prev = current;
            current = current->next;
            continue;
        }

        if (current->result && !hasSideEffect(current))
        {
            int isUsed = 0;
            TAC *temp = current->next;
            while (temp != NULL && !(temp->op && strcmp(temp->op, "epilogue") == 0))
            {
                if ((temp->arg1 && strcmp(temp->arg1, current->result) == 0) ||
                    (temp->arg2 && strcmp(temp->arg2, current->result) == 0))
                {
                    isUsed = 1;
                    break;
                }

                if (temp->result && strcmp(temp->result, current->result) == 0)
                    break;

                temp = temp->next;
            }

            if (!isUsed)
            {
                DEBUG_PRINT("Dead Code Elimination: Removing unused instruction with result %s\n", current->result);
                TAC *toDelete = current;
                if (prev == NULL)
                    *head = current->next;
                else
                    prev->next = current->next;

                current = current->next;

                if (toDelete->op) free(toDelete->op);
                if (toDelete->arg1) free(toDelete->arg1);
                if (toDelete->arg2) free(toDelete->arg2);
                if (toDelete->result) free(toDelete->result);
                free(toDelete);

                changes++;
                continue;
            }
        }
        prev = current;
        current = current->next;
    }
    return changes;
}

// Check if an instruction has side effects
bool hasSideEffect(TAC *instr)
{
    if (instr == NULL || instr->op == NULL)
        return false;

    if (strcmp(instr->op, "return") == 0) return true;
    if (instr->result && strcmp(instr->result, "result") == 0)
        return true;
    if (strcmp(instr->op, "[]=") == 0) return true;
    if (strcmp(instr->op, "write") == 0 || strcmp(instr->op, "write_float") == 0) return true;
    if (strcmp(instr->op, "call") == 0) return true;
    if (strcmp(instr->op, "prologue") == 0) return true;
    if (strcmp(instr->op, "epilogue") == 0) return true;
    if (strcmp(instr->op, "param") == 0) return true;
    if (strcmp(instr->op, "label") == 0) return true;
    if (strcmp(instr->op, "ifFalse") == 0) return true;
    if (strcmp(instr->op, "goto") == 0) return true;

    return false;
}

// ==================== Main Optimization Function ====================

void optimizeTAC(TAC **head)
{
    printf("=================Optimizer=================\n");
    DEBUG_PRINT("Running optimizer...\n");
    TAC *current = *head;

    while (current != NULL)
    {
        // Find start of function block
        if (current->op && strcmp(current->op, "prologue") == 0)
        {
            TAC *funcStart = current;
            TAC *funcEnd = funcStart;

            // Find corresponding epilogue
            while (funcEnd != NULL && !(funcEnd->op && strcmp(funcEnd->op, "epilogue") == 0))
            {
                funcEnd = funcEnd->next;
            }

            if (funcEnd == NULL)
                break;

            int changes;
            int iterationCount = 0;
            const int MAX_ITERATIONS = 100;

            do
            {
                changes = 0;
                // Apply constant propagation
                changes += constantPropagation(&funcStart);
                // Apply array index propagation
                changes += arrayIndexPropagation(&funcStart);
                // Apply constant folding
                changes += constantFolding(&funcStart);
                // Apply copy propagation
                changes += copyPropagation(&funcStart);

                iterationCount++;
                if (iterationCount >= MAX_ITERATIONS)
                {
                    fprintf(stderr, "Warning: Optimization reached %d iterations and stopped.\n", MAX_ITERATIONS);
                    break;
                }
            } while (changes > 0);

            // Apply dead code elimination
            changes += deadCodeElimination(&funcStart);

            // Update the head if necessary
            if (*head == current)
                *head = funcStart;

            current = funcEnd->next;
        }
        else
        {
            current = current->next;
        }
    }

    printf("Optimization complete.\n");
}
