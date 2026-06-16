#include "Utils.h"
#include "Colors.h"
#include <termios.h>
#include <unistd.h>
#include <signal.h>

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

// Équivalent ASCII d'un caractère Latin-1 (codepoint 0xC0..0xFF) dont l'octet de
// continuation UTF-8 (celui qui suit 0xC3) est c2. Renvoie 0 si pas d'équivalent.
char accent_to_ascii(unsigned char c2) {
    switch (c2) {
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: return 'A'; // À..Æ
        case 0x87: return 'C';                                                                   // Ç
        case 0x88: case 0x89: case 0x8A: case 0x8B: return 'E';                                  // È..Ë
        case 0x8C: case 0x8D: case 0x8E: case 0x8F: return 'I';                                  // Ì..Ï
        case 0x91: return 'N';                                                                   // Ñ
        case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x98: return 'O';            // Ò..Ö, Ø
        case 0x99: case 0x9A: case 0x9B: case 0x9C: return 'U';                                  // Ù..Ü
        case 0x9D: return 'Y';                                                                   // Ý
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: return 'a'; // à..æ
        case 0xA7: return 'c';                                                                   // ç
        case 0xA8: case 0xA9: case 0xAA: case 0xAB: return 'e';                                  // è..ë
        case 0xAC: case 0xAD: case 0xAE: case 0xAF: return 'i';                                  // ì..ï
        case 0xB1: return 'n';                                                                   // ñ
        case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB8: return 'o';            // ò..ö, ø
        case 0xB9: case 0xBA: case 0xBB: case 0xBC: return 'u';                                  // ù..ü
        case 0xBD: case 0xBF: return 'y';                                                        // ý, ÿ
        default: return 0;
    }
}

// Remplace, en place, les accents (UTF-8 Latin-1) par leur équivalent ASCII :
// "Équateur" -> "Equateur", "israël" -> "israel". Les séquences non reconnues
// (et le bloc 0xC2) sont retirées. Idempotent sur du texte déjà ASCII.
void fold_accents(char *txt) {
    unsigned char *s = (unsigned char *) txt;
    char *d = txt;
    while (*s) {
        if (*s == 0xC3 && s[1]) {
            char a = accent_to_ascii(s[1]);
            if (a) *d++ = a;
            s += 2;
        } else if (*s == 0xC2 && s[1]) {
            s += 2; // ponctuation Latin-1 (° °, etc.) : ignorée
        } else {
            *d++ = (char) *s++;
        }
    }
    *d = '\0';
}

// Normalise la saisie du joueur : replie les accents, supprime les espaces de
// début/fin et remplace les espaces internes par des tirets bas
// ("Équateur" -> "Equateur", "costa rica" -> "costa_rica").
void normalize_guess(char *txt) {
    fold_accents(txt);
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

// SAISIE INTERACTIVE AVEC AUTO-COMPLÉTION (Tab)

// Sauvegarde du mode terminal pour le restaurer même en cas de Ctrl+C / exit.
static struct termios g_term_saved;
static volatile sig_atomic_t g_term_raw = 0;

static void term_restore(void) {
    if (g_term_raw) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_term_saved);
        g_term_raw = 0;
    }
}

static void term_signal(int sig) {
    term_restore();
    signal(sig, SIG_DFL);
    raise(sig);
}

// Vide une séquence d'échappement (flèches, etc. : ESC [ ... lettre finale).
static void drain_escape(void) {
    unsigned char seq;
    if (read(STDIN_FILENO, &seq, 1) <= 0) return;
    if (seq == '[' || seq == 'O') {
        unsigned char f;
        do {
            if (read(STDIN_FILENO, &f, 1) <= 0) return;
        } while (f < 0x40 || f > 0x7E);
    }
}

// Complétion Tab : complète buf au plus long préfixe commun des pays qui
// commencent par la saisie (insensible à la casse/accents/espaces). S'il y a
// plusieurs candidats, les liste. Renvoie la nouvelle longueur de buf.
static int complete_country(const char *prompt, char *buf, int len, int size,
                            char **keys, int n_keys) {
    char pfx[128];
    int pl = (len < (int) sizeof(pfx) - 1) ? len : (int) sizeof(pfx) - 1;
    memcpy(pfx, buf, pl);
    pfx[pl] = '\0';
    normalize_guess(pfx);
    int plen = (int) strlen(pfx);
    if (plen == 0) return len;

    int idx[256];
    int m = 0;
    for (int i = 0; i < n_keys && m < (int) (sizeof(idx) / sizeof(idx[0])); i++) {
        if (strncasecmp(keys[i], pfx, plen) == 0) idx[m++] = i;
    }
    if (m == 0) {
        putchar('\a');
        fflush(stdout);
        return len;
    }

    // Plus long préfixe commun (insensible à la casse), basé sur idx[0].
    int common = (int) strlen(keys[idx[0]]);
    for (int t = 1; t < m; t++) {
        int j = 0;
        while (j < common && keys[idx[t]][j] &&
               tolower((unsigned char) keys[idx[t]][j]) ==
               tolower((unsigned char) keys[idx[0]][j])) {
            j++;
        }
        common = j;
    }
    if (common > size - 1) common = size - 1;

    if (m > 1) {
        // Tri alphabétique des candidats puis affichage.
        for (int a = 0; a < m - 1; a++) {
            for (int b = a + 1; b < m; b++) {
                if (strcasecmp(keys[idx[a]], keys[idx[b]]) > 0) {
                    int tmp = idx[a]; idx[a] = idx[b]; idx[b] = tmp;
                }
            }
        }
        putchar('\n');
        for (int t = 0; t < m; t++) printf("  %s", keys[idx[t]]);
        putchar('\n');
        fputs(prompt, stdout);
    } else {
        // Un seul candidat : on réécrit la ligne courante proprement.
        fputs("\r\033[K", stdout);
        fputs(prompt, stdout);
    }

    memcpy(buf, keys[idx[0]], common);
    buf[common] = '\0';
    fwrite(buf, 1, common, stdout);
    fflush(stdout);
    return common;
}

// Lit un nom de pays sur stdin avec auto-complétion (touche Tab) et repli des
// accents à la frappe. En mode non interactif (entrée redirigée), bascule sur
// read_line (la complétion nécessite un terminal). Renvoie 1, ou 0 sur EOF.
int read_country_line(const char *prompt, char *buf, int size,
                      char **keys, int n_keys) {
    fputs(prompt, stdout);
    fflush(stdout);

    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        return read_line(buf, size);
    }

    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &g_term_saved) != 0) {
        return read_line(buf, size);
    }
    raw = g_term_saved;
    raw.c_lflag &= ~(tcflag_t) (ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    g_term_raw = 1;

    static int handlers_installed = 0;
    if (!handlers_installed) {
        handlers_installed = 1;
        signal(SIGINT, term_signal);
        signal(SIGTERM, term_signal);
        atexit(term_restore);
    }

    int len = 0;
    buf[0] = '\0';
    int eof = 0;
    for (;;) {
        unsigned char c;
        ssize_t r = read(STDIN_FILENO, &c, 1);
        if (r <= 0) { eof = 1; break; }

        if (c == '\n' || c == '\r') {
            break;
        } else if (c == 4) { // Ctrl+D
            if (len == 0) eof = 1;
            break;
        } else if (c == 127 || c == 8) { // Backspace
            if (len > 0) {
                len--;
                buf[len] = '\0';
                fputs("\b \b", stdout);
                fflush(stdout);
            }
        } else if (c == '\t') {
            len = complete_country(prompt, buf, len, size, keys, n_keys);
        } else if (c == 27) { // ESC : séquence (flèches...) à ignorer
            drain_escape();
        } else if (c == 0xC3 || c == 0xC2) { // accent UTF-8 : replié à la frappe
            unsigned char c2;
            if (read(STDIN_FILENO, &c2, 1) <= 0) { eof = 1; break; }
            char a = (c == 0xC3) ? accent_to_ascii(c2) : 0;
            if (a && len < size - 1) {
                buf[len++] = a;
                buf[len] = '\0';
                putchar(a);
                fflush(stdout);
            }
        } else if (c >= 32 && c < 127) { // ASCII imprimable
            if (len < size - 1) {
                buf[len++] = (char) c;
                buf[len] = '\0';
                putchar((char) c);
                fflush(stdout);
            }
        }
        // autres octets de contrôle : ignorés
    }

    term_restore();
    putchar('\n');
    fflush(stdout);
    return (eof && len == 0) ? 0 : 1;
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
