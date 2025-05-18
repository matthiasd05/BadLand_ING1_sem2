#ifndef RESOURCES_H
#define RESOURCES_H

#include <allegro.h>

#define GAME_SCREEN_W 800
#define GAME_SCREEN_H 600
#define JUMP_STRENGTH -15
#define MAGENTA makecol(255, 0, 255)
#define MENU 0
#define LEVEL_SELECTION 1
#define PLAYING 2
#define END_SCREEN 3
#define SUBMENU 4
#define START_MENU 10
#define MAX_OBSTACLES 30
#define NUM_LEVELS 3
#define EGG_EFFECT_DURATION 15
#define REDUCED_GRAVITY 1
#define AUGM_GRAVITY 3
#define SAVE_FILE "sauvegardes.txt"

BITMAP* copy_bitmap_with_transparency(BITMAP src, int scale_factor);
void play_music(SAMPLE music);

extern char pseudo_joueur[50];

#endif