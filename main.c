#include <allegro.h>

#define SCREEN_W 1000
#define SCREEN_H 1000
#define GRAVITY 1
#define JUMP_STRENGTH -15
#define MAGENTA makecol(255, 0, 255)

BITMAP *buffer;
BITMAP *background;
BITMAP *player_original;
BITMAP *player;

int player_x = 100;
int player_y = 300;
int player_speed_y = 0;

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
    player_original = load_bitmap("player.bmp", NULL);  // image originale

    if (!background || !player_original) {
        allegro_message("Erreur lors du chargement des bitmaps !");
        exit(1);
    }

    // Créer une version réduite avec transparence manuelle
    player = copy_bitmap_with_transparency(player_original, 2);  // 2 = réduction à 50%
}

void deinit() {
    destroy_bitmap(buffer);
    destroy_bitmap(background);
    destroy_bitmap(player_original);
    destroy_bitmap(player);
}

void update_physics() {
    if (key[KEY_SPACE]) {
        player_speed_y = JUMP_STRENGTH;
    }

    player_speed_y += GRAVITY;
    player_y += player_speed_y;

    if (player_y > SCREEN_H - player->h) {
        player_y = SCREEN_H - player->h;
        player_speed_y = 0;
    }

    if (player_y < 0) {
        player_y = 0;
        player_speed_y = 0;
    }
}

void draw() {
    draw_sprite(buffer, background, 0, 0);

    // Dessiner le joueur manuellement avec transparence
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
        update_physics();
        draw();
        rest(20);
    }

    deinit();
    return 0;
}
END_OF_MAIN();
