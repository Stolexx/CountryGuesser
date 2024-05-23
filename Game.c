#include "Game.h"
#include "Colors.h"

int GAMES = 0;

void start_game(Game* game) {
    Entry* entry = get_entry(COUNTRIES, get_keys(COUNTRIES)[arc4random() % COUNTRIES_COUNT]);
    game->country = entry->key;
    for(int i = 0; i < entry->values_count; i++) {
        switch (i)
        {
        case 0:
            game->continent = entry->values[i];
            break;
        case 1:
            game->language = entry->values[i];
            break;
        case 2:
            game->population = entry->values[i];
            break;
        case 3:
            game->currency = entry->values[i];
            break;
        case 4:
            game->borders = entry->values[i];
            break;
        case 5:
            game->colors = entry->values[i];
            break;
        default:
            break;
        }
    }
    printf("Game started! (ID:%d)\n", get_id(game));
}

int guess(Game* game) {
    game->guess++;
    char guess[50];
    printf("Nom du pays: ");
    scanf("%s", guess);
    Entry* entry = get_entry(COUNTRIES, guess);
    if(entry != NULL) {
        if(strcmp(to_lowercase(guess), to_lowercase(game->country)) == 0) {
            printf(GRN "Correct!\n" reset);
            printf(GRN "Vous avez trouvé en " UWHT "%d" reset GRN " essai(s)\n" reset, game->guess);
            print_result(game);
            return 1;
        } else {
            printf(RED "Faux!\n" reset);
            compare_countries(game, entry);
            return 0;
        }
    }else {
        printf(RED "Faux! Ce pays n'est pas dans la liste\n" BRED "/!\\" RED " Ne pas oublier les majuscules et de remplacer les accents\n" reset);
        return 0;
    }
}

void compare_countries(Game* game, Entry* entry) {
    // Si résultat bool : 0 = faux / 1 = vrai
    // Si résultat +/-/= : 0 = moins / 1 = plus / 2 = égal
    int results[6] = {0,0,0,0,0,0};
    // Continent
    if(!strcmp(to_lowercase(game->continent), to_lowercase(entry->values[0]))) {
        results[0] = 1;
    }
    // Langue
    int size_languages_r = -1, size_languages_e = -1;
    char** result_languages = split_by(game->language, ",", &size_languages_r);
    char** entry_languages = split_by(entry->values[1], ",", &size_languages_e);
    for(int i = 0; i < size_languages_r; i++) {
        for(int j = 0; j < size_languages_e; j++) {
            if(!strcmp(to_lowercase(result_languages[i]), to_lowercase(entry_languages[j]))) {
                results[1] = 1;
                break;
            }
        }
    }
    free(result_languages);
    free(entry_languages);
    // Population
    if(atoi(game->population) < atoi(entry->values[2])) {
        results[2] = 0;
    } else if(atoi(game->population) > atoi(entry->values[2])) {
        results[2] = 1;
    } else if(atoi(game->population) == atoi(entry->values[2])) {
        results[2] = 2;
    }
    // Devise
    if(!strcmp(to_lowercase(game->currency), to_lowercase(entry->values[3]))) {
        results[3] = 1;
    }
    // Frontières
    if(atoi(game->borders) < atoi(entry->values[4])) {
        results[4] = 0;
    } else if(atoi(game->borders) > atoi(entry->values[4])) {
        results[4] = 1;
    } else if(atoi(game->borders) == atoi(entry->values[4])) {
        results[4] = 2;
    }
    // Couleurs
    int size_colors_r = -1, size_colors_e = -1;
    char** result_colors = split_by(game->colors, ",", &size_colors_r);
    char** entry_colors = split_by(entry->values[5], ",", &size_colors_e);
    for(int i = 0; i < size_colors_r; i++) {
        for(int j = 0; j < size_colors_e; j++) {
            if(strcmp(to_lowercase(result_colors[i]), to_lowercase(entry_colors[j])) == 0) {
                results[5] = 1;
                break;
            }
        }
    }
    free(result_colors);
    free(entry_colors);

    printf(BCYN "RESULTATS\n" reset);
    printf("| " CYN "Continent: " BCYN "%s %s\n" reset, entry->values[0], results[0] == 1 ? GRN "(Correct)" : RED "(Faux)");
    printf("| " CYN "Langue: " BCYN "%s %s\n" reset, entry->values[1], results[1] == 1 ? GRN "(Correct)" : RED "(Faux)");
    printf("| " CYN "Population: " BCYN "%s %s\n" reset, entry->values[2], results[2] == 2 ? GRN "(Correct)" : (results[2] == 0 ? RED "↓ (Moins)" : RED "↑ (Plus)"));
    printf("| " CYN "Devise: " BCYN "%s %s\n" reset, entry->values[3], results[3] == 1 ? GRN "(Correct)" : RED "(Faux)");
    printf("| " CYN "Frontières: " BCYN "%s %s\n" reset, entry->values[4], results[4] == 2 ? GRN "(Correct)" : (results[4] == 0 ? RED "↓ (Moins)" : RED "↑ (Plus)"));
    printf("| " CYN "Couleurs: " BCYN "%s %s\n" reset, entry->values[5], results[5] == 1 ? GRN "(Correct)" : RED "(Faux)");
    printf("\n");
}

void print_result(Game* game) {
    printf(BCYN "RESULTATS\n" reset);
    printf("| " CYN "Pays: " BCYN "%s\n" reset, game->country);
    printf("| " CYN "Continent: " BCYN "%s\n" reset, game->continent);
    printf("| " CYN "Langue: " BCYN "%s\n" reset, game->language);
    printf("| " CYN "Population: " BCYN "%s\n" reset, game->population);
    printf("| " CYN "Devise: " BCYN "%s\n" reset, game->currency);
    printf("| " CYN "Frontières: " BCYN "%s\n" reset, game->borders);
    printf("| " CYN "Couleurs: " BCYN "%s\n" reset, game->colors);
}

int get_id(Game* game) {
    return game->id;
}