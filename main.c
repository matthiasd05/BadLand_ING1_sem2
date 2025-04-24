#include <allegro.h>
#include <stdio.h>

#define SCREEN_W 1000
#define SCREEN_H 1000
#define GRAVITY 1
#define JUMP_STRENGTH -15
#define PLAYER_SPEED 5
#define MAGENTA makecol(255, 0, 255)

BITMAP *buffer;
BITMAP *background;
BITMAP *player_original;
BITMAP *player;

int player_x = 100;
int player_y = 300;
int player_speed_y = 0;
int player_scale = 5; // Facteur de mise à l'échelle du personnage

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
    install_keyboard();  // Assure-toi que le clavier est bien installé
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, SCREEN_W, SCREEN_H, 0, 0);

    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    background = load_bitmap("background.bmp", NULL);
    player_original = load_bitmap("player.bmp", NULL);

    if (!background || !player_original) {
        allegro_message("Erreur lors du chargement des bitmaps !");
        exit(1);
    }

    player = copy_bitmap_with_transparency(player_original, player_scale);  // utilisation de player_scale
}

void deinit() {
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    destroy_bitmap(player_original);
    destroy_bitmap(player);
}

void update_physics() {
    int moving = 0;

    if (key[KEY_D]) {
        player_x += PLAYER_SPEED;
        moving = 1;
    }
    if (key[KEY_A]) {
        player_x -= PLAYER_SPEED;
        moving = 1;
    }

    // Limites de l'écran
    if (player_x < 0) player_x = 0;
    if (player_x > SCREEN_W - player->w) player_x = SCREEN_W - player->w;
    // Saut possible à tout moment (type "vol battement")
    if (key[KEY_SPACE]) {
        player_speed_y = JUMP_STRENGTH;
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

void draw() {
    draw_sprite(buffer, background, 0, 0);

    for (int y = 0; y < player->h; y++) {
        for (int x = 0; x < player->w; x++) {
            int color = getpixel(player, x, y);
            if (color != MAGENTA) {
                putpixel(buffer, player_x + x, player_y + y, color);
            }
        }
    }

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

int main() {
    init();

    while (!key[KEY_ESC]) {
        // Ajout d'un débogage pour afficher les touches pressées
        if (key[KEY_Q]) {
            printf("Touche Q pressée\n");
        }

        update_physics();
        draw();
        rest(20);  // ~50 FPS
    }

    deinit();
    return 0;
}
END_OF_MAIN();
