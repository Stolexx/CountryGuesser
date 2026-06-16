#include "Game.h"
#include "Colors.h"

void init() {
    Game game = {.id = GAMES++, .guess = 0};
    start_game(&game);

    printf("Première tentative...\n");
    while (guess(&game) != 1) {
        printf("Nouvelle tentative...\n");
    }
}

void new_game() {
    char answer[16];
    printf("Voulez-vous démarrer une nouvelle partie ?\n Oui (O) ou Non (N) : ");
    if (!read_line(answer, sizeof(answer))) {
        printf("\n");
        return;
    }
    if (tolower((unsigned char) answer[0]) == 'o') {
        init();
        new_game();
    }
}

void end() {
    free_dictionary(COUNTRIES);
}

int main() {
    printf("Chargement des pays...\n");
    load_countries();
    printf("Pays chargés !\n");

    printf("\n >>> " BLK WHTB " Country Guessr 1.0 " reset " <<<\n");
    printf(RED "Astuce : la casse et les espaces sont ignorés (« costa rica » = « Costa_Rica »). Écrivez les noms sans accents.\n" reset);
    printf(RED "Tapez " BRED "/abandon" reset RED " pour révéler la réponse.\n\n" reset);

    init();
    new_game();

    end();
    return 0;
}
