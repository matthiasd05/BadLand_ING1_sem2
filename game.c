#include "game.h"
#include "resources.h"
#include <allegro.h>

void init(void) {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, GAME_SCREEN_W, GAME_SCREEN_H, 0, 0);
    install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);

    buffer = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);

}

void deinit(void) {
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    destroy_bitmap(background2);
    destroy_bitmap(background3);
    destroy_bitmap(badland_logo);
    destroy_bitmap(player_original);
    destroy_bitmap(player);
    destroy_bitmap(menu_background);
    destroy_bitmap(level_selection_background);
    destroy_bitmap(play_button);
    destroy_bitmap(play_button_hover);
    destroy_bitmap(map_overlay);
    destroy_bitmap(player1_original);
    destroy_bitmap(player2_original);
    destroy_bitmap(player1);
    destroy_bitmap(player2);
    destroy_bitmap(end_screen_image);
    destroy_bitmap(victoire);
    destroy_bitmap(heart_icon);
    destroy_bitmap(winflag);
    destroy_bitmap(obstacle);
    destroy_bitmap(roue);
    destroy_bitmap(eggblue);
    destroy_bitmap(eggred);
    destroy_bitmap(egggreen);
    destroy_bitmap(egggrey);
    destroy_bitmap(bombe);
    destroy_bitmap(explosion1);
    destroy_bitmap(explosion2);
    destroy_bitmap(explosion3);

    // Libération des sons
    destroy_sample(jungle_sound);
    destroy_sample(nature_sound);
    destroy_sample(neige_sound);
    destroy_sample(feu_sound);
    destroy_sample(jump_sound);
    destroy_sample(gameover_sound);
    destroy_sample(victoire_sound);
}

void show_pause_screen(void) {
    clear_to_color(buffer, makecol(0, 0, 0));
    textout_centre_ex(buffer, font, "PAUSE", GAME_SCREEN_W / 2, GAME_SCREEN_H / 2 - 20, makecol(255, 255, 255), -1);
    textout_centre_ex(buffer, font, "Appuyez sur 'P' pour reprendre", GAME_SCREEN_W / 2, GAME_SCREEN_H / 2 + 20, makecol(200, 200, 200), -1);
    blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
}

void show_victory_screen(void) {
    int blink = 0;
    clear_keybuf();
    stop_sample(nature_sound);
    stop_sample(neige_sound);
    stop_sample(feu_sound);
    play_sample(victoire_sound, 100, 128, 1000, FALSE);
    while (!key[KEY_ENTER]) {
        clear_bitmap(buffer);
        draw_sprite(buffer, victoire, 0, 0);
        if (blink) {
            textout_centre_ex(buffer, font, "Restez Appuyé sur ENTRER pour retourner au menu",
                              GAME_SCREEN_W/2, GAME_SCREEN_H - 50, makecol(255,255,255), -1);
        }
        blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
        blink = !blink;
        rest(500);
    }
    game_state = MENU;
    clear_keybuf();
}