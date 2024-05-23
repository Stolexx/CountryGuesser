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
    char answer[10];
    printf("Voulez-vous démarrer une nouvelle partie ?\n Oui (O) ou Non (N): ");
    scanf("%s", answer);

    if (to_lowercase(answer)[0] == 'o') {
        init();
        new_game();
    }
}

void end() {
    free_dictionary(COUNTRIES);
}

int main() {
    printf("Loading countries...\n");
    load_countries();
    printf("Countries loaded!\n");

    printf("\n >>> " BLK WHTB " Country Guesser 1.0 " reset " <<<\n");
    printf(BRED "/!\\ " reset RED "Les espaces dans les noms de pays doivent être remplacés par des tirets bas " BRED "/!\\\n\n" reset);

    init();
    new_game();

    end();
    return 0;
}
