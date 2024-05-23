#include "Utils.h"

// Fonction de hachage simple
unsigned int hash(const char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash << 5) + *key++;
    }
    return hash % TABLE_SIZE;
}

// Fonction pour créer un nouveau dictionnaire
Dictionary* create_dictionary() {
    Dictionary *dict = malloc(sizeof(Dictionary));
    dict->entries = malloc(TABLE_SIZE * sizeof(Entry*));
    dict->size = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        dict->entries[i] = NULL;
    }
    return dict;
}

// Fonction pour créer une nouvelle entrée
Entry* create_entry(const char *key, const char *value) {
    Entry *entry = malloc(sizeof(Entry));
    entry->key = strdup(key);
    entry->values_count = 1;
    entry->values_capacity = 4;
    entry->values = malloc(entry->values_capacity * sizeof(char*));
    entry->values[0] = strdup(value);
    entry->next = NULL;
    return entry;
}

// Fonction pour ajouter une valeur à une entrée existante
void add_value_to_entry(Entry *entry, const char *value) {
    if (entry->values_count == entry->values_capacity) {
        entry->values_capacity *= 2;
        entry->values = realloc(entry->values, entry->values_capacity * sizeof(char*));
    }
    entry->values[entry->values_count++] = strdup(value);
}

// Fonction pour ajouter une entrée au dictionnaire
void add_entry(Dictionary *dict, const char *key, const char *value) {
    unsigned int index = hash(key);
    Entry *entry = dict->entries[index];

    if (entry == NULL) {
        // Pas de collision
        dict->entries[index] = create_entry(key, value);
        dict->size++;
        return;
    }

    // Collision : ajout à la fin de la liste chaînée
    Entry *prev;
    while (entry != NULL) {
        // Mise à jour de la valeur si la clé existe déjà
        if (strcmp(entry->key, key) == 0) {
            add_value_to_entry(entry, value);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
    prev->next = create_entry(key, value);
}

// Fonction pour rechercher une entrée dans le dictionnaire
Entry* get_entry(Dictionary *dict, const char *key) {
    unsigned int index = hash(key);
    Entry *entry = dict->entries[index];

    while (entry != NULL) {
        if (strcmp(to_lowercase(entry->key), to_lowercase(key)) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL; // Clé non trouvée
}

// Fonction pour supprimer une entrée du dictionnaire
void delete_entry(Dictionary *dict, const char *key) {
    unsigned int index = hash(key);
    Entry *entry = dict->entries[index];
    Entry *prev = NULL;

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        // Clé non trouvée
        return;
    }

    if (prev == NULL) {
        // La clé à supprimer est la première de la liste chaînée
        dict->entries[index] = entry->next;
    } else {
        prev->next = entry->next;
    }

    for (int i = 0; i < entry->values_count; i++) {
        free(entry->values[i]);
    }
    dict->size--;
    free(entry->values);
    free(entry->key);
    free(entry);
}

char** get_keys(Dictionary *dict) {
    char **keys = malloc(dict->size * sizeof(char*));
    int index = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *entry = dict->entries[i];
        while (entry != NULL) {
            keys[index++] = entry->key;
            entry = entry->next;
        }
    }
    return keys;
}

// Fonction pour libérer toute la mémoire allouée par le dictionnaire
void free_dictionary(Dictionary *dict) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *entry = dict->entries[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            for (int j = 0; j < temp->values_count; j++) {
                free(temp->values[j]);
            }
            free(temp->values);
            free(temp->key);
            free(temp);
        }
    }
    free(dict->entries);
    free(dict);
}