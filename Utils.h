#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE 200

// Dictionary
typedef struct Entry {
    char *key;
    char **values;
    int values_count;
    int values_capacity;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **entries;
    int size;
} Dictionary;

// Utils.c
char* to_lowercase(const char* txt);
char* remove_spaces(char* txt);
char **split_by(char *txt, char *delimiter, int *count);
void print_table(int rows, int columns, char *data[rows][columns]);
// Dictionary.c
unsigned int hash(const char *key);
Dictionary* create_dictionary();
Entry* create_entry(const char *key, const char *value);
void add_value_to_entry(Entry *entry, const char *value);
void add_entry(Dictionary *dict, const char *key, const char *value);
Entry* get_entry(Dictionary *dict, const char *key);
void delete_entry(Dictionary *dict, const char *key);
char** get_keys(Dictionary *dict);
void free_dictionary(Dictionary *dict);

#endif // UTILS_H