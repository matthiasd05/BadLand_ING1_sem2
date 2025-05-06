#include <allegro.h>
#include <stdio.h>
#include <time.h>



#define GAME_SCREEN_W 800
#define GAME_SCREEN_H 600
#define GRAVITY 1
#define JUMP_STRENGTH -15
#define SCROLL_SPEED 2
#define MAGENTA makecol(255, 0, 255)

#define MENU 0
#define LEVEL_SELECTION 1
#define PLAYING 2


int game_state = MENU;

BITMAP *buffer;
BITMAP *background;
BITMAP *badland_logo;
BITMAP *player_original;
BITMAP *player;
BITMAP *menu_background;
BITMAP *level_selection_background;
BITMAP *play_button;
BITMAP *play_button_hover;
BITMAP *map_overlay = NULL;
BITMAP *player1_original;
BITMAP *player2_original;
BITMAP *player1;
BITMAP *player2;
FONT *font_large;
int player_x = 100;
int player_y = 300;
int player_speed_y = 0;
int player_scale = 8;
int world_x = 0;
int animation_frame = 0;

int game_started = 0;
time_t start_time = 0;
int elapsed_seconds = 0;

int my_mouse_x, my_mouse_y;
int play_button_x, play_button_y;
int play_button_width = 200;
int play_button_height = 80;

// Niveau (0=Facile, 1=Moyen, 2=Difficile)
int selected_level = -1;

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

void init() {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, GAME_SCREEN_W, GAME_SCREEN_H, 0, 0);

    buffer = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
    background = load_bitmap("background.bmp", NULL);
    player_original = load_bitmap("player.bmp", NULL);

    play_button = load_bitmap("play_button.bmp", NULL);
    play_button_hover = load_bitmap("play_button_hover.bmp", NULL);

    player1_original = load_bitmap("player1.bmp", NULL);
    player2_original = load_bitmap("player2.bmp", NULL);

    BITMAP *original_logo = load_bitmap("badland_logo.bmp", NULL);
    if (!original_logo) {
        allegro_message("Erreur lors du chargement du logo badland_logo.bmp !");
        exit(1);
    }
    // Redimensionner à 60% de la largeur de l’écran
    int new_logo_width = GAME_SCREEN_W * 0.6;
    int new_logo_height = original_logo->h * new_logo_width / original_logo->w;

    badland_logo = create_bitmap(new_logo_width, new_logo_height);
    stretch_blit(original_logo, badland_logo,
                 0, 0, original_logo->w, original_logo->h,
                 0, 0, new_logo_width, new_logo_height);

    destroy_bitmap(original_logo);


    BITMAP *temp_menu_bg = load_bitmap("menu_background.bmp", NULL);
    if (temp_menu_bg) {
        menu_background = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        stretch_blit(temp_menu_bg, menu_background,
                     0, 0, temp_menu_bg->w, temp_menu_bg->h,
                     0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
        destroy_bitmap(temp_menu_bg);
    } else {
        menu_background = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        clear_to_color(menu_background, makecol(50, 50, 100));
        textout_centre_ex(menu_background, font, "BADLAND GAME", GAME_SCREEN_W/2, GAME_SCREEN_H/3, makecol(255, 255, 255), -1);
    }

    BITMAP *temp_level_bg = load_bitmap("level_selection_background.bmp", NULL);
    if (temp_level_bg) {
        level_selection_background = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        stretch_blit(temp_level_bg, level_selection_background,
                     0, 0, temp_level_bg->w, temp_level_bg->h,
                     0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
        destroy_bitmap(temp_level_bg);
    } else {
        level_selection_background = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        clear_to_color(level_selection_background, makecol(30, 30, 60));
    }



    if (!menu_background) {
        menu_background = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        clear_to_color(menu_background, makecol(50, 50, 100));
        textout_centre_ex(menu_background, font, "BADLAND GAME", GAME_SCREEN_W/2, GAME_SCREEN_H/3, makecol(255, 255, 255), -1);
    }

    if (!play_button) {
        play_button = create_bitmap(play_button_width, play_button_height);
        clear_to_color(play_button, makecol(70, 70, 150));
        textout_centre_ex(play_button, font, "JOUER", play_button_width/2, play_button_height/2 - text_height(font)/2, makecol(255, 255, 255), -1);
    }

    if (!play_button_hover) {
        play_button_hover = create_bitmap(play_button_width, play_button_height);
        clear_to_color(play_button_hover, makecol(100, 100, 200));
        textout_centre_ex(play_button_hover, font, "JOUER", play_button_width/2, play_button_height/2 - text_height(font)/2, makecol(255, 255, 0), -1);
    }

    if (!background || !player_original) {
        allegro_message("Erreur lors du chargement des bitmaps !");
        exit(1);
    }

    map_overlay = load_bitmap("map_level1.bmp", NULL);
    if (!map_overlay) {
        // Map optionnelle, pas critique, on peut continuer
        map_overlay = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        clear_to_color(map_overlay, makecol(0, 0, 0)); // ou ne rien faire
    }


    player = copy_bitmap_with_transparency(player_original, player_scale);
    player1 = copy_bitmap_with_transparency(player1_original, player_scale);
    player2 = copy_bitmap_with_transparency(player2_original, player_scale);

    play_button_x = (GAME_SCREEN_W - play_button_width) / 2;

    int logo_height = badland_logo ? badland_logo->h : 150;  // Hauteur estimée si logo non chargé
    int spacing_between = 20;  // espace logo <-> bouton
    int spacing_text = 20;     // espace bouton <-> texte

    int total_block_height = logo_height + play_button_height + spacing_between + spacing_text + text_height(font);
    int top_of_block = (GAME_SCREEN_H - total_block_height) / 2;

    play_button_y = top_of_block + logo_height + spacing_between;


}

void deinit() {
    destroy_bitmap(buffer); // libère la memoire des buffers graphiques
    destroy_bitmap(background);
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

}

void update_physics() {
    if (game_state != PLAYING) return;

    static int player_blocked_right = 0;

    // Gestion du saut du joueur
    if (key[KEY_SPACE]) {
        player_speed_y = JUMP_STRENGTH;
        if (!game_started) {
            game_started = 1;
            start_time = time(NULL);
        }
    }

    // Gestion de la gravité
    player_speed_y += GRAVITY;
    int new_y = player_y + player_speed_y;

    player_blocked_right = 0;

    if (map_overlay && selected_level == 0) {
        int collision = 0;

        // COLLISION BAS
        if (player_speed_y > 0) {
            for (int x = 0; x < player->w; x++) {
                int px = player_x + x + world_x;
                int py = new_y + player->h;
                if (py < GAME_SCREEN_H && px >= 0 && px < map_overlay->w) {
                    int color = getpixel(map_overlay, px, py);
                    if (color == makecol(0, 0, 0)) {
                        collision = 1;
                        break;
                    }
                }
            }
            if (collision) {
                player_speed_y = 0;
            } else {
                player_y = new_y;
            }
        }
            // COLLISION HAUT
        else if (player_speed_y < 0) {
            for (int x = 0; x < player->w; x++) {
                int px = player_x + x + world_x;
                int py = new_y;
                if (py >= 0 && px >= 0 && px < map_overlay->w) {
                    int color = getpixel(map_overlay, px, py);
                    if (color == makecol(0, 0, 0)) {
                        collision = 1;
                        break;
                    }
                }
            }
            if (collision) {
                player_speed_y = 0;
            } else {
                player_y = new_y;
            }
        } else {
            player_y = new_y;
        }

        // DÉTECTION DU MUR À DROITE
        int player_front_x = player_x + player->w + world_x;
        int collision_front = 0;

        for (int y = 0; y < player->h; y++) {
            int map_y = player_y + y;
            if (player_front_x >= 0 && player_front_x < map_overlay->w &&
                map_y >= 0 && map_y < map_overlay->h) {
                int color = getpixel(map_overlay, player_front_x, map_y);
                if (color == makecol(0, 0, 0)) {
                    collision_front = 1;
                    break;
                }
            }
        }

        if (collision_front) {
            player_blocked_right = 1;
        } else {
            player_blocked_right = 0;
        }

    } else {
        // Pas de map : libre
        player_y = new_y;

        if (player_y > GAME_SCREEN_H - player->h) {
            player_y = GAME_SCREEN_H - player->h;
            player_speed_y = 0;
        }
        if (player_y < 0) {
            player_y = 0;
            player_speed_y = 0;
        }
    }

    // Sécurité verticale
    if (player_y > GAME_SCREEN_H - player->h) {
        player_y = GAME_SCREEN_H - player->h;
        player_speed_y = 0;
    }
    if (player_y < 0) {
        player_y = 0;
        player_speed_y = 0;
    }

    // SCROLLING CONSTANT
    if (game_started) {
        // Défilement constant du fond
        world_x += SCROLL_SPEED;
        if (world_x >= background->w) {
            world_x = 0;
        }
    }

    if (player_blocked_right) {
        player_x -= SCROLL_SPEED;

        if (player_x + player->w < 0) {
            int blink = 0;
            clear_keybuf();
            while (!key[KEY_ENTER]) {
                // 1) Prépare le buffer
                clear_bitmap(buffer);
                draw_sprite(buffer, menu_background, 0, 0);

                textout_centre_ex(buffer, font, "GAME OVER",
                                  GAME_SCREEN_W/2,
                                  GAME_SCREEN_H/2 - text_height(font),
                                  makecol(255,0,0), -1);

                if (blink) {
                    textout_centre_ex(buffer, font, "Appuyer sur ENTRER pour quitter",
                                      GAME_SCREEN_W/2,
                                      GAME_SCREEN_H/2 + text_height(font),
                                      makecol(255,255,255), -1);
                }

                blit(buffer, screen, 0,0, 0,0, GAME_SCREEN_W, GAME_SCREEN_H);

                blink = !blink;
                rest(500);
            }

            game_state      = MENU;
            game_started    = 0;
            selected_level  = -1;
            player_x        = 100;
            player_y        = 300;
            world_x         = 0;
            player_speed_y  = 0;
            clear_keybuf();
        }
    }
}



void draw_timer() {
    if (game_started) {
        elapsed_seconds = (int)difftime(time(NULL), start_time);
        int minutes = elapsed_seconds / 60;
        int seconds = elapsed_seconds % 60;

        char time_str[20];
        sprintf(time_str, "Time: %02d:%02d", minutes, seconds);

        int text_width = text_length(font, time_str);
        int text_x = (GAME_SCREEN_W - text_width) / 2;

        rectfill(buffer, text_x - 10, 10, text_x + text_width + 10, 30, makecol(0, 0, 0));
        textout_ex(buffer, font, time_str, text_x, 15, makecol(255, 255, 255), -1);
    }
}

void draw_game() {
    int bg_pos1 = -world_x;
    int bg_pos2 = bg_pos1 + background->w;

    draw_sprite(buffer, background, bg_pos1, 0);

    if (bg_pos2 < GAME_SCREEN_W) {
        draw_sprite(buffer, background, bg_pos2, 0);
    }

    int map_pos1 = -world_x;
    int map_pos2 = map_pos1 + map_overlay->w;

    draw_sprite(buffer, map_overlay, map_pos1, 0);
    if (map_pos2 < GAME_SCREEN_W) {
        draw_sprite(buffer, map_overlay, map_pos2, 0);
    }



    for (int y = 0; y < player->h; y++) {
        for (int x = 0; x < player->w; x++) {
            int color = getpixel(player, x, y);
            if (color != MAGENTA) {
                putpixel(buffer, player_x + x, player_y + y, color);
            }
        }
    }

    draw_timer();
    // Sélection du sprite actif
    BITMAP *current_sprite = player;

    if (key[KEY_SPACE]) {
        animation_frame++;
        if ((animation_frame / 5) % 2 == 0) {
            current_sprite = player1;
        } else {
            current_sprite = player2;
        }
    } else {
        animation_frame = 0;
        current_sprite = player;
    }

    // Dessiner le sprite choisi
    masked_blit(current_sprite, buffer, 0, 0, player_x, player_y, current_sprite->w, current_sprite->h);

}

void draw_menu() {
    draw_sprite(buffer, menu_background, 0, 0);

    poll_mouse();
    my_mouse_x = mouse_x;
    my_mouse_y = mouse_y;

    // Affichage du logo (centré horizontalement, au-dessus du bouton)
    if (badland_logo) {
        int logo_x = (GAME_SCREEN_W - badland_logo->w) / 2;
        int logo_y = play_button_y - 20 - badland_logo->h;  // juste au-dessus du bouton
        draw_sprite(buffer, badland_logo, logo_x, logo_y);
    }

    // Détection du survol du bouton
    int mouse_over_button = (my_mouse_x >= play_button_x && my_mouse_x <= play_button_x + play_button_width &&
                             my_mouse_y >= play_button_y && my_mouse_y <= play_button_y + play_button_height);

    if (mouse_over_button) {
        draw_sprite(buffer, play_button_hover, play_button_x, play_button_y);
        if (mouse_b & 1) {
            game_state = LEVEL_SELECTION;
            rest(200);
        }
    } else {
        draw_sprite(buffer, play_button, play_button_x, play_button_y);
    }

    // Affichage du texte d'instruction sous le bouton
    char *msg = "Ou appuyez sur ESPACE pour commencer";
    int text_width = text_length(font, msg);
    int text_x = (GAME_SCREEN_W - text_width) / 2;
    int text_y = play_button_y + play_button_height + 10;

    textout_ex(buffer, font, msg, text_x, text_y, makecol(255, 255, 255), -1);
}



void draw_level_selection() {
    draw_sprite(buffer, level_selection_background, 0, 0);

    textout_centre_ex(buffer, font, "Choisissez la difficulté", GAME_SCREEN_W/2, 100, makecol(255, 255, 255), -1);

    int button_w = 200;
    int button_h = 50;
    int start_y = 200;

    const char* levels[] = {"Facile", "Moyen", "Difficile"};

    poll_mouse();
    my_mouse_x = mouse_x;
    my_mouse_y = mouse_y;

    for (int i = 0; i < 3; i++) {
        int button_x = (GAME_SCREEN_W - button_w) / 2;
        int button_y = start_y + i * 100;

        int mouse_over = (my_mouse_x >= button_x && my_mouse_x <= button_x + button_w &&
                          my_mouse_y >= button_y && my_mouse_y <= button_y + button_h);

        if (mouse_over) {
            rectfill(buffer, button_x, button_y, button_x + button_w, button_y + button_h, makecol(100, 100, 200));
            if (mouse_b & 1) {
                selected_level = i;
                game_state = PLAYING;
                rest(200);
            }
        } else {
            rectfill(buffer, button_x, button_y, button_x + button_w, button_y + button_h, makecol(70, 70, 150));
        }

        textout_centre_ex(buffer, font, levels[i], GAME_SCREEN_W/2, button_y + 15, makecol(255, 255, 255), -1);
    }
}

void draw() {
    clear_bitmap(buffer);

    if (game_state == MENU) {
        draw_menu();
    } else if (game_state == LEVEL_SELECTION) {
        draw_level_selection();
    } else if (game_state == PLAYING) {
        draw_game();
    }

    blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
}

int main() {
    init();

    while (!key[KEY_ESC]) {
        show_mouse(screen);

        if (game_state == MENU && key[KEY_SPACE]) {
            game_state = LEVEL_SELECTION;
            rest(200);
        }

        update_physics();
        draw();
        rest(20);
    }

    deinit();
    return 0;
}
END_OF_MAIN();