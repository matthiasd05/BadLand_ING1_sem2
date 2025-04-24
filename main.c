#include <allegro.h>
#include <stdio.h>
#include <time.h>

#define SCREEN_W 1000
#define SCREEN_H 1000
#define GRAVITY 1
#define JUMP_STRENGTH -15
#define SCROLL_SPEED 2  // Vitesse de défilement automatique

#define MAGENTA makecol(255, 0, 255)

BITMAP *buffer;
BITMAP *background;
BITMAP *player_original;
BITMAP *player;

int player_x = 100;  // Position fixe en X
int player_y = 300;
int player_speed_y = 0;
int player_scale = 5; // Facteur de mise à l'échelle du personnage

// Position du "monde", pour le scrolling
int world_x = 0;

// Variables pour le timer et l'état du jeu
int game_started = 0;
time_t start_time = 0;
int elapsed_seconds = 0;

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
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, SCREEN_W, SCREEN_H, 0, 0);

    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    background = load_bitmap("background.bmp", NULL);
    player_original = load_bitmap("player.bmp", NULL);

    if (!background || !player_original) {
        allegro_message("Erreur lors du chargement des bitmaps !");
        exit(1);
    }

    player = copy_bitmap_with_transparency(player_original, player_scale);
}

void deinit() {
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    destroy_bitmap(player_original);
    destroy_bitmap(player);
}

void update_physics() {
    // Scrolling automatique seulement si le jeu a commencé
    if (game_started) {
        world_x += SCROLL_SPEED;

        // Limites du scrolling (si ton background est plus large que SCREEN_W)
        if (world_x > background->w - SCREEN_W) {
            world_x = background->w - SCREEN_W;
        }
    }

    // Saut possible à tout moment (type "vol battement")
    if (key[KEY_SPACE]) {
        player_speed_y = JUMP_STRENGTH;

        // Démarre le jeu et le timer au premier appui sur espace
        if (!game_started) {
            game_started = 1;
            start_time = time(NULL);
        }
    }

    // Gravité
    player_speed_y += GRAVITY;
    player_y += player_speed_y;

    // Collision sol
    if (player_y > SCREEN_H - player->h) {
        player_y = SCREEN_H - player->h;
        player_speed_y = 0;
    }

    // Collision plafond
    if (player_y < 0) {
        player_y = 0;
        player_speed_y = 0;
    }
}

void draw_timer() {
    if (game_started) {
        // Calcule le temps écoulé
        elapsed_seconds = (int)difftime(time(NULL), start_time);

        // Convertit en minutes et secondes
        int minutes = elapsed_seconds / 60;
        int seconds = elapsed_seconds % 60;

        // Formatte le texte
        char time_str[20];
        sprintf(time_str, "Time: %02d:%02d", minutes, seconds);

        // Calcule la position centrée
        int text_width = text_length(font, time_str);
        int text_x = (SCREEN_W - text_width) / 2;

        // Dessine un fond semi-transparent pour le timer
        rectfill(buffer, text_x - 10, 10, text_x + text_width + 10, 30, makecol(0, 0, 0));

        // Dessine le texte du timer
        textout_ex(buffer, font, time_str, text_x, 15, makecol(255, 255, 255), -1);
    }
}

void draw_start_message() {
    if (!game_started) {
        char *msg = "Appuyez sur ESPACE pour commencer";
        int text_width = text_length(font, msg);
        int text_x = (SCREEN_W - text_width) / 2;
        int text_y = SCREEN_H / 2;

        // Dessine un fond semi-transparent
        rectfill(buffer, text_x - 10, text_y - 10, text_x + text_width + 10, text_y + 20, makecol(0, 0, 0));
        // Dessine le message
        textout_ex(buffer, font, msg, text_x, text_y, makecol(255, 255, 255), -1);
    }
}

void draw() {
    // Dessine le fond défilant
    draw_sprite(buffer, background, -world_x, 0);

    // Dessine le personnage (position X fixe)
    for (int y = 0; y < player->h; y++) {
        for (int x = 0; x < player->w; x++) {
            int color = getpixel(player, x, y);
            if (color != MAGENTA) {
                putpixel(buffer, player_x + x, player_y + y, color);
            }
        }
    }

    // Dessine le message de départ si le jeu n'a pas commencé
    draw_start_message();

    // Dessine le timer
    draw_timer();

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

int main() {
    init();

    while (!key[KEY_ESC]) {
        update_physics();
        draw();
        rest(20);  // ~50 FPS
    }

    deinit();
    return 0;
}
END_OF_MAIN();