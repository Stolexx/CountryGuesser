#ifndef GAME_H
#define GAME_H

#include "Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern int GAMES;
extern int COUNTRIES_COUNT;
extern Dictionary *COUNTRIES;

typedef struct {
    const int id;
    char* country;
    char* continent;
    char* language;
    char* population;
    char* currency;
    char* borders;
    char* colors;
    // Dynamique
    int guess;
} Game;

// Loader.c
void load_countries();
// Game.c
void start_game(Game* game);
int guess(Game* game);
void compare_countries(Game* game, Entry* entry);
void print_result(Game* game);
int get_id(Game* game);

#endif // GAME_H