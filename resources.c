#include "resources.h"
#include <stdio.h>

BITMAP *buffer;
BITMAP *background, *background2, *background3;
BITMAP *badland_logo, *menu_background, *level_selection_background;
BITMAP *player, *player1, *player2;
BITMAP *player_original, *player1_original, *player2_original;
BITMAP *play_button, *play_button_hover;
BITMAP *end_screen_image, *victoire;
BITMAP *map_overlay;
BITMAP *winflag, *obstacle, *roue;
BITMAP *eggblue, *eggred, *egggreen, *egggrey;
BITMAP *bombe, *explosion1, *explosion2, *explosion3;
BITMAP *heart_icon;

SAMPLE *jungle_sound, *nature_sound, *neige_sound, *feu_sound;
SAMPLE *jump_sound, *gameover_sound, *victoire_sound;
SAMPLE *current_music = NULL;

char pseudo_joueur[50] = "Anonyme";

BITMAP* copy_bitmap_with_transparency(BITMAP *src, int scale_factor) {
    int new_w = src->w / scale_factor;
    int new_h = src->h / scale_factor;

    BITMAP *result = create_bitmap(new_w, new_h);
    clear_to_color(result, MAGENTA);

    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            int src_color = getpixel(src, x * scale_factor, y * scale_factor);
            if (src_color != MAGENTA) {
                putpixel(result, x, y, src_color);
            }
        }
    }

    return result;
}

void play_music(SAMPLE* music) {
    if (current_music != music) {
        stop_sample(current_music);
        current_music = music;
        if (current_music) {
            play_sample(current_music, 250, 128, 1000, TRUE);
        }
    }
}
