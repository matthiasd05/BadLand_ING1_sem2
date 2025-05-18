#include "levels.h"
#include "resources.h"
#include "player.h"
#include <allegro.h>
#include <stdio.h>

int selected_level = -1;
int winflag_positions[NUM_LEVELS][2] = {
    {6000, 200},
    {6050, 450},
    {6000, 200}
};

void handle_level_completion(void) {
    stop_sample(nature_sound);
    stop_sample(neige_sound);
    stop_sample(feu_sound);
    play_sample(victoire_sound, 100, 128, 1000, FALSE);

    rest(1000);

    if (selected_level < NUM_LEVELS - 1) {
        selected_level++;
        reset_player();
        load_map_for_selected_level();
        game_state = PLAYING;

        switch (selected_level) {
            case 0: play_sample(nature_sound, 100, 128, 1000, FALSE); break;
            case 1: play_sample(neige_sound, 100, 128, 1000, FALSE); break;
            case 2: play_sample(feu_sound, 100, 128, 1000, FALSE); break;
        }
    } else {
        show_victory_screen();
    }
}

void load_map_for_selected_level(void) {
    if (map_overlay) destroy_bitmap(map_overlay);

    switch (selected_level) {
        case 0:
            map_overlay = load_bitmap("map_level1.bmp", NULL);
            play_music(nature_sound);
            break;
        case 1:
            map_overlay = load_bitmap("map_level2.bmp", NULL);
            play_music(neige_sound);
            break;
        case 2:
            map_overlay = load_bitmap("map_level3.bmp", NULL);
            play_music(feu_sound);
            break;
        default:
            map_overlay = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
            clear_to_color(map_overlay, makecol(0, 0, 0));
            break;
    }

    if (!map_overlay) {
        allegro_message("Erreur de chargement de la carte pour le niveau sélectionné !");
        exit(1);
    }
}

void draw_level_selection(void) {
    draw_sprite(buffer, level_selection_background, 0, 0);
    textout_centre_ex(buffer, font, "Choisissez la difficulté", GAME_SCREEN_W / 2, 100, makecol(255, 255, 255), -1);

    int button_w = 200, button_h = 50, start_y = 200;
    const char* levels[] = {"Facile", "Moyen", "Difficile"};

    poll_mouse();
    int my_mouse_x = mouse_x;
    int my_mouse_y = mouse_y;

    for (int i = 0; i < 3; i++) {
        int button_x = (GAME_SCREEN_W - button_w) / 2;
        int button_y = start_y + i * 100;
        int mouse_over = (my_mouse_x >= button_x && my_mouse_x <= button_x + button_w &&
                          my_mouse_y >= button_y && my_mouse_y <= button_y + button_h);

        int color = mouse_over ? makecol(100, 100, 200) : makecol(70, 70, 150);
        rectfill(buffer, button_x, button_y, button_x + button_w, button_y + button_h, color);
        rect(buffer, button_x, button_y, button_x + button_w, button_y + button_h, makecol(255, 255, 255));
        textout_centre_ex(buffer, font, levels[i], GAME_SCREEN_W / 2, button_y + 15, makecol(255, 255, 255), -1);

        if (mouse_over && (mouse_b & 1)) {
            selected_level = i;
            game_state = PLAYING;
            rest(200);
        }
    }
}
