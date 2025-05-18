#include "player.h"
#include "resources.h"
#include "levels.h"
#include "save.h"
#include <allegro.h>
#include <stdio.h>
#include <time.h>

int player_x = 100, player_y = 300;
int player_speed_y = 0;
int player_scale = 12;
int player_lives = 3;
int world_x = 0;
int animation_frame = 0;
int game_started = 0;
time_t start_time = 0;
int elapsed_seconds = 0;
int game_paused = 0;
int gravity = 2;
int scrollspeed = 2;

void update_physics(void) {
    if (game_state != PLAYING) return;

    if (key[KEY_SPACE]) {
        player_speed_y = JUMP_STRENGTH;
        if (!game_started) {
            game_started = 1;
            start_time = time(NULL);
        }
    }

    player_speed_y += gravity;
    player_y += player_speed_y;

    if (player_y < 0) player_y = 0;
    if (player_y > GAME_SCREEN_H - player->h) {
        player_y = GAME_SCREEN_H - player->h;
        player_speed_y = 0;
    }

    world_x += scrollspeed;
}

void draw_game(void) {
    blit(background, buffer, world_x, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);

    for (int y = 0; y < player->h; y++) {
        for (int x = 0; x < player->w; x++) {
            int color = getpixel(player, x, y);
            if (color != MAGENTA) {
                putpixel(buffer, player_x + x, player_y + y, color);
            }
        }
    }

    char time_str[20];
    elapsed_seconds = (int)difftime(time(NULL), start_time);
    int minutes = elapsed_seconds / 60;
    int seconds = elapsed_seconds % 60;
    sprintf(time_str, "Time: %02d:%02d", minutes, seconds);
    textout_ex(buffer, font, time_str, 10, 10, makecol(255, 255, 255), -1);

    for (int i = 0; i < player_lives; i++) {
        draw_sprite(buffer, heart_icon, 10 + i * (heart_icon->w + 5), 40);
    }
}

void reset_player(void) {
    player_x = 100;
    player_y = 300;
    player_speed_y = 0;
    world_x = 0;
    animation_frame = 0;
    game_started = 0;
    start_time = time(NULL);
    elapsed_seconds = 0;
    game_paused = 0;
    gravity = 2;
    player_lives = 3;

    destroy_bitmap(player);
    player = copy_bitmap_with_transparency(player_original, player_scale);
}
