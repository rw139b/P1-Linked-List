#include "lab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/**
 * @brief structure for the doubly linked list
 * AI Use: No AI
 */
typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

/**
 * @brief structure containing the sentinel node and metadata
 * AI Use: No AI
 */
struct List {
    Node *sentinel;
    size_t size;
    ListType type;
};

/**
 * @brief Create a new node with the given data
 * @param data Pointer to the data to store in the node
 * @return Pointer to the newly created node, or NULL on failure
 * AI Use: Assisted AI
 */
static Node *node_create(void *data) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

/**
 * @brief Create a new list of the specified type
 * @param type The type of list to create
 * @return Pointer to the newly created list, or NULL on failure
 * AI Use: Assisted AI
 */
List *list_create(ListType type) {
    if (type != LIST_LINKED_SENTINEL) {
        return NULL; 
    }
    
    List *list = (List *)malloc(sizeof(List));
    if (list == NULL) {
        return NULL;
    }
    
    list->sentinel = node_create(NULL);
    if (list->sentinel == NULL) {
        free(list);
        return NULL;
    }
    
    // sentinel points to itself
    list->sentinel->next = list->sentinel;
    list->sentinel->prev = list->sentinel;
    
    list->size = 0;
    list->type = type;
    
    return list;
}

/**
 * @brief the list and free all associated memory
 * @param list Pointer to the list to destroy
 * @param free_func Function to free individual elements (can be NULL)
 * AI Use: Assisted AI
 */
void list_destroy(List *list, FreeFunc free_func) {
    if (list == NULL) {
        return;
    }
    
    // Remove all nodes except sentinel
    Node *current = list->sentinel->next;
    while (current != list->sentinel) {
        Node *next = current->next;
        if (free_func != NULL && current->data != NULL) {
            free_func(current->data);
        }
        free(current);
        current = next;
    }
    
    free(list->sentinel);
    free(list);
}

/**
 * @brief Append an element to the end of the list
 * @param list Pointer to the list
 * @param data Pointer to the data to append
 * @return true on success, false on failure
 * AI Use: Assisted AI
 */
bool list_append(List *list, void *data) {
    if (list == NULL) {
        return false;
    }
    
    Node *new_node = node_create(data);
    if (new_node == NULL) {
        return false;
    }
    
    // Insert new node before sentinel (at the end)
    Node *last = list->sentinel->prev;
    
    new_node->next = list->sentinel;
    new_node->prev = last;
    last->next = new_node;
    list->sentinel->prev = new_node;
    
    list->size++;
    return true;
}

/**
 * @brief Insert an element at a specific index
 * @param list Pointer to the list
 * @param index Index at which to insert the element
 * @param data Pointer to the data to insert
 * @return true on success, false on failure
 * AI Use: Assisted AI
 */
bool list_insert(List *list, size_t index, void *data) {
    if (list == NULL || index > list->size) {
        return false;
    }
    
    Node *new_node = node_create(data);
    if (new_node == NULL) {
        return false;
    }
    
    // Find the position to insert
    Node *current = list->sentinel;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    // Insert after current
    Node *next = current->next;
    new_node->next = next;
    new_node->prev = current;
    current->next = new_node;
    next->prev = new_node;
    
    list->size++;
    return true;
}

/**
 * @brief Remove an element at a specific index
 * @param list Pointer to the list
 * @param index Index of the element to remove
 * @return Pointer to the element data, or NULL if index is out of bounds
 * AI Use: Assisted AI
 */
void *list_remove(List *list, size_t index) {
    if (list == NULL || index >= list->size) {
        return NULL;
    }
    
    // Find the node to remove
    Node *current = list->sentinel->next;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    // Remove the node from the list
    void *data = current->data;
    current->prev->next = current->next;
    current->next->prev = current->prev;
    
    free(current);
    list->size--;
    
    return data;
}

/**
 * @brief Get a pointer to the element at a specific index
 * @param list Pointer to the list
 * @param index Index of the element to retrieve
 * @return Pointer to the element, or NULL if index is out of bounds
 * AI Use: Assisted AI
 */
void *list_get(const List *list, size_t index) {
    if (list == NULL || index >= list->size) {
        return NULL;
    }
    
    // Find the node at the specified index
    Node *current = list->sentinel->next;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current->data;
}

/**
 * @brief Get the current size of the list
 * @param list Pointer to the list
 * @return The number of elements in the list
 * AI Use: Written by AI
 */
size_t list_size(const List *list) {
    if (list == NULL) {
        return 0;
    }
    return list->size;
}

/**
 * @brief Check if the list is empty
 * @param list Pointer to the list
 * @return true if the list is empty, false otherwise
 * AI Use: Written by AI
 */
bool list_is_empty(const List *list) {
    if (list == NULL) {
        return true;
    }
    return list->size == 0;
}

//P2

/**
 * @brief Sorts a portion of the list between start and end indices (inclusive)
 * using bubble sort and the given compare function.
 */
void sort(List *list, size_t start, size_t end, CompareFunc cmp) {
    if (!list || start >= end || end >= list->size) return;

    for (size_t i = start; i < end; i++) {
        Node *a = list->sentinel->next;
        for (size_t k = 0; k < i; k++) {
            a = a->next;
        }
        for (size_t j = start; j < end - (i - start); j++) {
            Node *b = a->next;
            if (cmp(a->data, b->data) > 0) {
                void *tmp = a->data;
                a->data = b->data;
                b->data = tmp;
            }
            a = b;
        }
    }
}

/**
 * @brief Merges two sorted lists into a new sorted list.
 */
List *merge(const List *a, const List *b, CompareFunc cmp) {
    if (!a || !b || !cmp) return NULL;

    List *out = list_create(LIST_LINKED_SENTINEL);
    if (!out) return NULL;

    Node *na = a->sentinel->next;
    Node *nb = b->sentinel->next;

    while (na != a->sentinel && nb != b->sentinel) {
        if (cmp(na->data, nb->data) <= 0) {
            list_append(out, na->data);
            na = na->next;
        } else {
            list_append(out, nb->data);
            nb = nb->next;
        }
    }

    while (na != a->sentinel) {
        list_append(out, na->data);
        na = na->next;
    }

    while (nb != b->sentinel) {
        list_append(out, nb->data);
        nb = nb->next;
    }

    return out;
}


/**
 * @brief Compare integers in descending order.
 */
int compare_int(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ib - ia);  // bigger first (descending)
}

/**
 * @brief Compare strings lexicographically (ascending).
 */
int compare_str(const void *a, const void *b) {
    const char *sa = (const char *)a;
    const char *sb = (const char *)b;
    return strcmp(sa, sb);
}

/**
 * @brief Checks if the list is sorted according to cmp.
 */
bool is_sorted(const List *list, CompareFunc cmp) {
    if (!list || list->size < 2) return true;

    Node *cur = list->sentinel->next;
    while (cur->next != list->sentinel) {
        if (cmp(cur->data, cur->next->data) > 0) {
            return false;
        }
        cur = cur->next;
    }
    return true;
}