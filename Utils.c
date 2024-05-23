#include "Utils.h"
#include "Colors.h"

// STRINGS

char* to_lowercase(const char* txt) {
    char* result = malloc(strlen(txt) + 1);
    char *temp = result;
    while(*txt) {
        *temp++ = tolower((unsigned char) *txt);
        txt++;
    }
    *temp = '\0';
    return result;
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

char **split_by(char *txt, char *delimiter, int *count) {
    char *token;
    char *rest = txt;
    int initial_size = 10;
    char **result = malloc(initial_size * sizeof(char *));
    *count = 0;

    if (strstr(txt, delimiter) != NULL) {
        token = strtok_r(rest, delimiter, &rest);
        while (token != NULL) {
            if (*count >= initial_size) {
                initial_size *= 2;
                result = realloc(result, initial_size * sizeof(char *));
            }
            result[(*count)++] = token;
            token = strtok_r(rest, delimiter, &rest);
        }
    } 
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
