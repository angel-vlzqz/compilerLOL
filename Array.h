#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>

typedef struct ArrayElement
{
    int index;                 // Index of the element in the array
    char *value;               // Value stored at this index (as a string)
    struct ArrayElement *next; // Pointer to the next element
} ArrayElement;

typedef struct Array
{
    char *dataType;         // Data type of the array elements
    int size;               // Declared size of the array
    ArrayElement *elements; // Linked list of initialized elements
} Array;

// Function prototypes

// Create a new array
Array *createArray(char *dataType, int size);

// Free an array
void freeArray(Array *array);

// Set value at a specific index
bool setArrayValue(Array *array, int index, const char *value);

// Get value at a specific index
char *getArrayValue(Array *array, int index);

// Get the size of the array
int getArraySize(Array *array);

#endif // ARRAY_H
