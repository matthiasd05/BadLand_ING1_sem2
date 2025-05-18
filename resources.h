#ifndef RESOURCES_H
#define RESOURCES_H

#include <allegro.h>

#define GAME_SCREEN_W 800
#define GAME_SCREEN_H 600
#define MAGENTA makecol(255, 0, 255)

// États du jeu
#define MENU 0
#define LEVEL_SELECTION 1
#define PLAYING 2
#define END_SCREEN 3
#define SUBMENU 4
#define START_MENU 10

#define NUM_LEVELS 3
#define MAX_OBSTACLES 30

extern BITMAP *buffer;
extern BITMAP *background, *background2, *background3;
extern BITMAP *badland_logo, *menu_background, *level_selection_background;
extern BITMAP *player, *player1, *player2;
extern BITMAP *player_original, *player1_original, *player2_original;
extern BITMAP *play_button, *play_button_hover;
extern BITMAP *end_screen_image, *victoire;
extern BITMAP *map_overlay;
extern BITMAP *winflag, *obstacle, *roue;
extern BITMAP *eggblue, *eggred, *egggreen, *egggrey;
extern BITMAP *bombe, *explosion1, *explosion2, *explosion3;
extern BITMAP *heart_icon;

extern SAMPLE *jungle_sound, *nature_sound, *neige_sound, *feu_sound;
extern SAMPLE *jump_sound, *gameover_sound, *victoire_sound;
extern SAMPLE *current_music;

extern char pseudo_joueur[50];

BITMAP* copy_bitmap_with_transparency(BITMAP *src, int scale_factor);
void play_music(SAMPLE* music);

#endif
