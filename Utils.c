#include "Utils.h"
#include "Colors.h"

// STRINGS

// Lit une ligne complète sur stdin dans buf (taille size), retire le saut de
// ligne final et vide le reste de la ligne si elle dépasse le tampon.
// Renvoie 1 en cas de succès, 0 sur fin d'entrée (EOF / Ctrl+D).
int read_line(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) {
        return 0;
    }
    size_t len = strcspn(buf, "\n");
    if (buf[len] == '\n') {
        buf[len] = '\0';
    } else {
        // Pas de saut de ligne lu : on jette le reste de la ligne
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }
    return 1;
}

// Normalise la saisie du joueur : supprime les espaces de début/fin et
// remplace les espaces internes par des tirets bas ("costa rica" -> "costa_rica").
void normalize_guess(char *txt) {
    size_t n = strlen(txt);
    size_t i = 0, j = n;
    while (i < n && isspace((unsigned char) txt[i])) i++;
    while (j > i && isspace((unsigned char) txt[j - 1])) j--;
    size_t k = 0;
    for (size_t p = i; p < j; p++) {
        txt[k++] = (txt[p] == ' ') ? '_' : txt[p];
    }
    txt[k] = '\0';
}

char* remove_spaces(char* txt) {
    char* result = malloc(strlen(txt) + 1);
    char *temp = result;
    while(*txt) {
        if (*txt != ' ') {
            *temp++ = *txt;
        }
        txt++;
    }
    *temp = '\0';
    return result;
}

// Découpe txt selon delimiter. Travaille sur une COPIE (strtok_r écrit des '\0'
// dans la source ; sans copie il corromprait définitivement les données du
// dictionnaire). Chaque token est dupliqué (à libérer par l'appelant) et ses
// espaces de début/fin sont retirés. Une valeur sans délimiteur renvoie 1 token.
char **split_by(char *txt, char *delimiter, int *count) {
    int capacity = 10;
    char **result = malloc(capacity * sizeof(char *));
    *count = 0;

    char *work = strdup(txt);
    char *rest = work;
    char *token = strtok_r(rest, delimiter, &rest);
    while (token != NULL) {
        // Trim des espaces de début/fin (ex. " Pashto" -> "Pashto")
        while (*token == ' ') token++;
        char *end = token + strlen(token);
        while (end > token && end[-1] == ' ') end--;
        *end = '\0';

        if (*count >= capacity) {
            capacity *= 2;
            result = realloc(result, capacity * sizeof(char *));
        }
        result[(*count)++] = strdup(token);
        token = strtok_r(rest, delimiter, &rest);
    }
    free(work);
    return result;
}

// PRINT TABLE

void print_separator(int *column_widths, int columns);
void print_row(char **row, int *column_widths, int columns);

void ascii_table(char ***data, int rows, int columns, int header) {
    int *column_widths = (int *)malloc(columns * sizeof(int));
    if (column_widths == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    for (int i = 0; i < columns; i++) {
        column_widths[i] = 0;
        for (int j = 0; j < rows; j++) {
            if (data[j][i] != NULL) {
                int length = strlen(data[j][i]);
                if (length > column_widths[i]) {
                    column_widths[i] = length;
                }
            }
        }
    }

    print_separator(column_widths, columns);
    for (int i = 0; i < rows; i++) {
        print_row(data[i], column_widths, columns);
        if (header && i == 0) {
            print_separator(column_widths, columns);
        }
    }
    print_separator(column_widths, columns);

    free(column_widths);
}

void print_separator(int *column_widths, int columns) {
    printf(YEL "+");
    for (int i = 0; i < columns; i++) {
        for (int j = 0; j < column_widths[i] + 2; j++) {
            printf("-");
        }
        printf("+");
    }
    printf("\n");
}

void print_row(char **row, int *column_widths, int columns) {
    printf("|");
    for (int i = 0; i < columns; i++) {
        printf(" %s ", row[i] ? row[i] : "");
        int length = row[i] ? strlen(row[i]) : 0;
        for (int j = length; j < column_widths[i]; j++) {
            printf(" ");
        }
        printf("|");
    }
    printf("\n");
}

void print_table(int rows, int columns, char *data[rows][columns]) {
    char ***table_data = (char ***)malloc(rows * sizeof(char **));
    for (int i = 0; i < rows; i++) {
        table_data[i] = (char **)malloc(columns * sizeof(char *));
        for (int j = 0; j < columns; j++) {
            table_data[i][j] = (char *) malloc(100 * sizeof(char));
            table_data[i][j] = data[i][j];
        }
    }

    ascii_table(table_data, rows, columns, 1);
    printf(reset);

    /*for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            free(table_data[i][j]);
        }
        free(table_data[i]);
    }
    free(table_data);*/
}
