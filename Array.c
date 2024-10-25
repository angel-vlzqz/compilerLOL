#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Array.h"

Array *createArray(char *dataType, int size)
{
    // invalid size
    if (size <= 0)
    {
        fprintf(stderr, "Error: Array size must be positive\n");
        return NULL;
    }

    Array *array = (Array *)malloc(sizeof(Array));
    // null check for array
    if (array == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for Array\n");
        return NULL;
    }

    array->dataType = strdup(dataType);
    // null check for dataType
    if (array->dataType == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for dataType\n");
        free(array);
        return NULL;
    }

    array->size = size;
    array->elements = NULL; // No elements allocated initially

    return array;
}

void freeArray(Array *array)
{
    if (array == NULL)
        return;

    // Free the dataType string
    free(array->dataType);

    // Free all the array elements
    ArrayElement *current = array->elements;
    while (current != NULL)
    {
        ArrayElement *next = current->next;
        free(current->value);
        free(current);
        current = next;
    }

    // Free the array itself
    free(array);
}

bool setArrayValue(Array *array, int index, const char *value)
{
    if (array == NULL)
    {
        fprintf(stderr, "Error: Array is NULL\n");
        return false;
    }

    if (index < 0 || index >= array->size)
    {
        fprintf(stderr, "Error: Array index out of bounds: %d\n", index);
        return false;
    }

    ArrayElement *current = array->elements;
    ArrayElement *prev = NULL;

    // Find the correct position to insert/update the element
    while (current != NULL && current->index < index)
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL && current->index == index)
    {
        // Update existing element
        free(current->value);
        current->value = strdup(value);
        if (current->value == NULL)
        {
            fprintf(stderr, "Error: Memory allocation failed for value\n");
            return false;
        }
    }
    else
    {
        // Create a new element
        ArrayElement *newElement = (ArrayElement *)malloc(sizeof(ArrayElement));
        if (newElement == NULL)
        {
            fprintf(stderr, "Error: Memory allocation failed for ArrayElement\n");
            return false;
        }
        newElement->index = index;
        newElement->value = strdup(value);
        if (newElement->value == NULL)
        {
            fprintf(stderr, "Error: Memory allocation failed for value\n");
            free(newElement);
            return false;
        }
        newElement->next = current;

        if (prev == NULL)
        {
            // Insert at the beginning
            array->elements = newElement;
        }
        else
        {
            prev->next = newElement;
        }
    }

    return true;
}

char *getArrayValue(Array *array, int index)
{
    if (array == NULL)
    {
        fprintf(stderr, "Error: Array is NULL\n");
        return NULL;
    }

    if (index < 0 || index >= array->size)
    {
        fprintf(stderr, "Error: Array index out of bounds: %d\n", index);
        return NULL;
    }

    ArrayElement *current = array->elements;

    while (current != NULL && current->index <= index)
    {
        if (current->index == index)
        {
            // Return a copy of the value to prevent accidental modification
            return strdup(current->value);
        }
        current = current->next;
    }

    // Element not found (uninitialized)
    return NULL;
}

int getArraySize(Array *array)
{
    if (array == NULL)
    {
        fprintf(stderr, "Error: Array is NULL\n");
        return -1; // Indicate an error
    }
    return array->size;
}