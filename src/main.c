#include "lab.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Generates a random string with length between min_len and max_len
 * @param min_len Minimum length of the string
 * @param max_len Maximum length of the string
 * @return Pointer to the newly allocated random string, or NULL on failure
 * AI Use: Written By AI
 */
static char *random_string(int min_len, int max_len) {
    int len = min_len + rand() % (max_len - min_len + 1);
    char *s = malloc((size_t)len + 1);
    if (!s) return NULL;

    for (int i = 0; i < len; i++) {
        s[i] = (char)('a' + (rand() % 26));
    }
    s[len] = '\0';
    return s;
}

/**
 * @brief Structure containing parameters for a sorting task
 * AI Use: Written By AI
 */
typedef struct {
    List *list;
    size_t start;
    size_t end;
    CompareFunc cmp;
} SortTask;

/**
 * @brief Thread function that performs sorting on a portion of a list
 * @param arg Pointer to SortTask structure containing sort parameters
 * @return NULL
 * AI Use: Written By AI
 */
static void *thread_sort(void *arg) {
    SortTask *task = (SortTask *)arg;
    sort(task->list, task->start, task->end, task->cmp);
    return NULL;
}

/**
 * @brief Verifies that a list is sorted according to the compare function
 * @param list Pointer to the list to verify
 * @param cmp Compare function to use for verification
 * @return 1 if sorted, 0 if not sorted
 * AI Use: Written By AI
 */
static int verify_sorted(const List *list, CompareFunc cmp) {
    if (!list || !cmp) return 0;
    if (list_size(list) < 2) return 1;

    for (size_t i = 0; i + 1 < list_size(list); i++) {
        void *a = list_get(list, i);
        void *b = list_get(list, i + 1);
        if (cmp(a, b) > 0) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Main function that demonstrates parallel sorting and merging
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 * AI Use: Written By AI
 */
#ifndef TEST
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <int|string> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand((unsigned)time(NULL));

    int is_int = strcmp(argv[1], "int") == 0;
    int is_string = strcmp(argv[1], "string") == 0;
    if (!is_int && !is_string) {
        fprintf(stderr, "First argument must be 'int' or 'string'\n");
        return EXIT_FAILURE;
    }

    int n = atoi(argv[2]);
    if (n <= 0) {
        fprintf(stderr, "List length must be > 0\n");
        return EXIT_FAILURE;
    }

    CompareFunc cmp = is_int ? compare_int : compare_str;

    // Create list and fill with random data
    List *list = list_create(LIST_LINKED_SENTINEL);
    for (int i = 0; i < n; i++) {
        if (is_int) {
            int *val = malloc(sizeof(int));
            *val = rand() % 1000;
            list_append(list, val);
        } else {
            char *s = random_string(5, 15);
            list_append(list, s);
        }
    }

    // Split into halves
    size_t mid = (size_t)n / 2;

    // Sort first half
    SortTask task1 = { list, 0, mid - 1, cmp };
    pthread_t t1;
    pthread_create(&t1, NULL, thread_sort, &task1);

    // Sort second half
    SortTask task2 = { list, mid, (size_t)n - 1, cmp };
    pthread_t t2;
    pthread_create(&t2, NULL, thread_sort, &task2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Build two sublists for merging
    List *left = list_create(LIST_LINKED_SENTINEL);
    for (size_t i = 0; i < mid; i++) {
        list_append(left, list_get(list, i));
    }
    List *right = list_create(LIST_LINKED_SENTINEL);
    for (size_t i = mid; i < (size_t)n; i++) {
        list_append(right, list_get(list, i));
    }

    // Merge halves
    List *sorted = merge(left, right, cmp);

    // Free old containers (but not data)
    list_destroy(left, NULL);
    list_destroy(right, NULL);
    list_destroy(list, NULL);

    // Verify sorted
    if (verify_sorted(sorted, cmp)) {
        printf("List is sorted!\n");
    } else {
        printf("Error: list is not sorted!\n");
    }

    // Print results
    for (size_t i = 0; i < list_size(sorted); i++) {
        if (is_int) {
            printf("%d\n", *(int *)list_get(sorted, i));
        } else {
            printf("%s\n", (char *)list_get(sorted, i));
        }
    }

    // Cleanup
    list_destroy(sorted, free);

    return EXIT_SUCCESS;
}
#endif