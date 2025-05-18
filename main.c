#include <allegro.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define GAME_SCREEN_W 800
#define GAME_SCREEN_H 600

#define JUMP_STRENGTH -15

#define MAGENTA makecol(255, 0, 255)

#define MENU 0
#define LEVEL_SELECTION 1
#define PLAYING 2
#define END_SCREEN 3
#define MAX_OBSTACLES 30
#define NUM_LEVELS 3
#define EGG_EFFECT_DURATION 15 // Durée de l'effet en secondes
#define REDUCED_GRAVITY 1  // Gravité plus faible
#define AUGM_GRAVITY 3
#define SUBMENU 4
#define START_MENU 10
#define SAVE_FILE "sauvegardes.txt"
char pseudo_joueur[50] = "Anonyme";  // Valeur par défaut si pas défini

void sauvegarder_niveau_pseudo(const char* pseudo, int niveau) {
    FILE *file = fopen(SAVE_FILE, "r");
    FILE *temp = fopen("temp_sauvegarde.txt", "w");
    int found = 0;
    char buffer_pseudo[50];
    int old_niveau;

    // Met à jour la ligne du pseudo si déjà présent
    if (file && temp) {
        while (fscanf(file, "%s %d", buffer_pseudo, &old_niveau) == 2) {
            if (strcmp(buffer_pseudo, pseudo) == 0) {
                fprintf(temp, "%s %d\n", pseudo, niveau);
                found = 1;
            } else {
                fprintf(temp, "%s %d\n", buffer_pseudo, old_niveau);
            }
        }
        fclose(file);
        fclose(temp);
        remove(SAVE_FILE);
        rename("temp_sauvegarde.txt", SAVE_FILE);
    }

    // Si le pseudo n'était pas encore enregistré
    if (!found) {
        file = fopen(SAVE_FILE, "a");
        if (file) {
            fprintf(file, "%s %d\n", pseudo, niveau);
            fclose(file);
        }
    }
}
int charger_niveau_pseudo(const char* pseudo) {
    FILE *file = fopen(SAVE_FILE, "r");
    if (!file) return -1;

    char temp_pseudo[50];
    int niveau;

    while (fscanf(file, "%s %d", temp_pseudo, &niveau) == 2) {
        if (strcmp(temp_pseudo, pseudo) == 0) {
            fclose(file);
            return niveau;
        }
    }

    fclose(file);
    return -1; // Pseudo non trouvé
}


typedef struct {
    char pseudo[50];
    int niveau;
} Joueur;

Joueur joueur_actuel;
int pseudo_saisi = 0;
static float obstacle_angle = 0;
static float roue_angle = 0;



int game_state = MENU;
int gravity = 2;
int scrollspeed = 2;
BITMAP *buffer;
BITMAP *background;
BITMAP *background2;
BITMAP *background3;
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
BITMAP *end_screen_image;
SAMPLE *jungle_sound;
SAMPLE *nature_sound;
SAMPLE *current_music = NULL;  // Pointeur vers la musique en cours
SAMPLE* jump_sound;
SAMPLE* gameover_sound;
SAMPLE *victoire_sound;
SAMPLE *neige_sound;
SAMPLE *feu_sound;
BITMAP *winflag;
BITMAP *victoire;
BITMAP *obstacle;
BITMAP *eggblue;
BITMAP *eggred;
BITMAP *egggreen;
BITMAP *egggrey;
BITMAP *bombe;
BITMAP *explosion1, *explosion2, *explosion3;
BITMAP *roue;
BITMAP *heart_icon;

int player_lives = 3;

int bombe_x = 2500;
int bombe_y = 100;
int bombe_active = 1;
float bombe_vy = 0.0;     // Vitesse verticale
float bombe_gravity = 0.09; // Gravité appliquée à la bombe
int bombe_visible = 0;
int bombe_explose = 0;
int explosion_frame = 0;
int explosion_timer = 0;


int egg_x = 1500; // Position initiale dans le monde
int egg_y = 300;
int egg_active = 1; // 1 = actif, 0 = déjà collecté
time_t egg_collected_time = 0;
int player_small = 0;

int eggr_x = 3450; // Position initiale dans le monde
int eggr_y = 450;
int eggr_active = 1;// 1 = actif, 0 = déjà collecté

int eggg_x = 2350; // Position initiale dans le monde
int eggg_y = 260;
int eggg_active = 1; // 1 = actif, 0 = déjà collecté
time_t eggg_collected_time = 0;

int egggrey_x = 300; // Position initiale dans le monde
int egggrey_y = 250;
int egggrey_active = 1; // 1 = actif, 0 = déjà collecté
time_t egggrey_collected_time = 0;

int player_x = 100;
int player_y = 300;
int player_speed_y = 0;
int player_scale = 12;
int world_x = 0;
int animation_frame = 0;
int game_started = 0;
time_t start_time = 0;
int elapsed_seconds = 0;
int game_paused = 0;

int my_mouse_x, my_mouse_y;
int play_button_x, play_button_y;
int play_button_width = 200;
int play_button_height = 80;



int winflag_positions[NUM_LEVELS][2] = {
        {6000, 200},  // Niveau 0
        {6050, 450},  // Niveau 1
        {6000, 200}   // Niveau 2
};





int obstacle_positions[MAX_OBSTACLES][2] = {
        {600, 200},
        {2350, 100},
        {2950, 450},
        {3600, 50},
        {3850, 500},
        {4000, 50},
        {5100, 450},
        {5380, 280},
        {5650, 450},
        {5900, 280},

};
int obstacle2_positions[MAX_OBSTACLES][2] = {
        {1600, 200},
        {1750, 480},
        {4000, 150},
        {5000, 350},
        {5650, 500},


};
int roue_positions[MAX_OBSTACLES][2] = {
        {500,  -50},
        {700, 450},
        {3400, -100},
        {3200, 450},

};


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
// Sous-programmes
void init();
void deinit();
void update_physics();
void show_end_screen();
void draw_timer();
void draw_game();
void draw_menu();
void draw_level_selection();
void draw();
void show_victory_screen();
void handle_level_completion();
void play_music(SAMPLE* music);
void show_pause_screen();
void draw_rotating_obstacle(int x, int y);
void draw_rotating_roue(int x, int y);
void load_map_for_selected_level();
int charger_niveau_pseudo(const char* pseudo);
void draw_start_menu();
void init() {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, GAME_SCREEN_W, GAME_SCREEN_H, 0, 0);
    install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);

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

    background2 = load_bitmap("background2.bmp", NULL);
    if (!background2) {
        allegro_message("Erreur chargement background2.bmp !");
        exit(1);
    }
    background3 = load_bitmap("background3.bmp", NULL);
    if (!background3) {
        allegro_message("Erreur chargement background3.bmp !");
        exit(1);
    }


    jungle_sound = load_sample("jungle.wav");
    nature_sound = load_sample("nature.wav");
    neige_sound = load_sample("neige.wav");
    feu_sound = load_sample("feu.wav");

    if (!jungle_sound || !nature_sound || !neige_sound || !feu_sound) {
        allegro_message("Erreur chargement musique !");
        exit(1);
    }
    jump_sound = load_sample("jump.wav");
    if (!jump_sound) {
        allegro_message("Erreur de chargement du son de saut !");
        exit(EXIT_FAILURE);
    }
    gameover_sound = load_sample("gameover.wav");
    if (!gameover_sound) {
        allegro_message("Erreur de chargement du son de game over !");
        exit(EXIT_FAILURE);
    }
    victoire_sound = load_sample("victoire.wav");
    if (!victoire_sound) {
        allegro_message("Erreur de chargement du son de game over !");
        exit(EXIT_FAILURE);
    }
    winflag = load_bitmap("winflag.bmp", NULL);
    winflag = copy_bitmap_with_transparency(winflag, 2);
    if (!winflag) {
        allegro_message("Erreur chargement winflag.bmp !");
        exit(1);
    }
    obstacle = load_bitmap("obstacle.bmp", NULL);
    obstacle = copy_bitmap_with_transparency(obstacle, 2);
    if (!obstacle) {
        allegro_message("Erreur chargement obstacle.bmp !");
        exit(1);
    }
    roue = load_bitmap("roue.bmp", NULL);
    roue = copy_bitmap_with_transparency(roue, 2);
    if (!roue) {
        allegro_message("Erreur chargement roue.bmp !");
        exit(1);
    }

    eggblue = load_bitmap("eggblue.bmp", NULL);
    eggblue = copy_bitmap_with_transparency(eggblue, 2);
    if (!eggblue) {
        allegro_message("Erreur chargement eggblue.bmp !");
        exit(1);
    }

    eggred = load_bitmap("eggred.bmp", NULL);
    eggred = copy_bitmap_with_transparency(eggred, 2);
    if (!eggred) {
        allegro_message("Erreur chargement eggred.bmp !");
        exit(1);
    }

    egggreen = load_bitmap("egggreen.bmp", NULL);
    egggreen = copy_bitmap_with_transparency(egggreen, 2);
    if (!egggreen) {
        allegro_message("Erreur chargement egggreen.bmp !");
        exit(1);
    }

    egggrey= load_bitmap("egggrey.bmp", NULL);
    egggrey = copy_bitmap_with_transparency(egggrey, 2);
    if (!egggrey) {
        allegro_message("Erreur chargement egggrey.bmp !");
        exit(1);
    }

    bombe = load_bitmap("bombe.bmp", NULL);
    bombe = copy_bitmap_with_transparency(bombe, 2);  // utilise ton système de transparence
    if (!bombe) {
        allegro_message("Erreur chargement bombe.bmp !");
        exit(1);
    }
    explosion1 = load_bitmap("explosion1.bmp", NULL);
    explosion2 = load_bitmap("explosion2.bmp", NULL);
    explosion3 = load_bitmap("explosion3.bmp", NULL);
    if (!explosion1 || !explosion2 || !explosion3) {
        allegro_message("Erreur chargement explosion.bmp !");
        exit(1);
    }



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
    BITMAP *temp_end = load_bitmap("terminer.bmp", NULL);
    if (!temp_end) {
        allegro_message("Erreur lors du chargement de terminer.bmp !");
        exit(1);
    }
    end_screen_image = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
    stretch_blit(temp_end, end_screen_image,
                 0, 0, temp_end->w, temp_end->h,
                 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    destroy_bitmap(temp_end);

    BITMAP *temp_victory = load_bitmap("victoire.bmp", NULL);
    if (!temp_victory) {
        allegro_message("Erreur lors du chargement de victoire.bmp !");
        exit(1);
    }

    victoire = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
    stretch_blit(temp_victory, victoire,
                 0, 0, temp_victory->w, temp_victory->h,
                 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    destroy_bitmap(temp_victory);


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

    BITMAP *heart_original = load_bitmap("coeur.bmp", NULL);
    if (!heart_original) {
        allegro_message("Erreur chargement coeur.bmp !");
        exit(1);
    }
    heart_icon = copy_bitmap_with_transparency(heart_original, 5);  // Réduit à 20%
    destroy_bitmap(heart_original);
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
    destroy_bitmap(end_screen_image);
    destroy_bitmap(background2);
    destroy_bitmap(background3);
    destroy_bitmap(heart_icon);

}
int bombe_collide_with_map() {
    if (!map_overlay) return 0;

    for (int y = 0; y < bombe->h; y++) {
        for (int x = 0; x < bombe->w; x++) {
            int map_x = bombe_x + x;
            int map_y = bombe_y + y;

            if (map_x >= 0 && map_x < map_overlay->w &&
                map_y >= 0 && map_y < map_overlay->h) {

                int color = getpixel(map_overlay, map_x, map_y);
                if (color == makecol(0, 0, 0)) {
                    return 1;  // collision détectée
                }
            }
        }
    }

    return 0;
}


void update_physics() {
    // 1) Ajuste de la vitesse de défilement selon le niveau
    switch (selected_level) {
        case 0: scrollspeed = 4; break;
        case 1: scrollspeed = 4; break;
        case 2: scrollspeed = 6; break;
        default: scrollspeed = 2; break;
    }

    // 2) Ne rien faire si on n'est pas en train de jouer
    if (game_state != PLAYING) return;

    static int player_blocked_right = 0;

    // 3) Saut
    if (key[KEY_SPACE]) {
        player_speed_y = JUMP_STRENGTH;
        if (!game_started) {
            game_started = 1;
            start_time = time(NULL);
        }
    }

    // 4) Gravité
    player_speed_y += gravity;
    int new_y = player_y + player_speed_y;
    player_blocked_right = 0;

    // 5) Collisions haut / bas
    if (map_overlay) {
        int collision = 0;
        // bas
        if (player_speed_y > 0) {
            for (int x = 0; x < player->w; x++) {
                int px = player_x + x + world_x;
                int py = new_y + player->h;
                if (py < GAME_SCREEN_H && px >= 0 && px < map_overlay->w &&
                    getpixel(map_overlay, px, py) == makecol(0,0,0)) {
                    collision = 1; break;
                }
            }
            if (collision) player_speed_y = 0;
            else           player_y = new_y;
        }
        // haut
        else if (player_speed_y < 0) {
            for (int x = 0; x < player->w; x++) {
                int px = player_x + x + world_x;
                int py = new_y;
                if (py >= 0 && px >= 0 && px < map_overlay->w &&
                    getpixel(map_overlay, px, py) == makecol(0,0,0)) {
                    collision = 1; break;
                }
            }
            if (collision) player_speed_y = 0;
            else           player_y = new_y;
        }
        else {
            player_y = new_y;
        }

        // mur à droite
        {
            int fx = player_x + player->w + world_x;
            for (int y = 0; y < player->h; y++) {
                int my = player_y + y;
                if (fx >= 0 && fx < map_overlay->w && my >= 0 && my < map_overlay->h &&
                    getpixel(map_overlay, fx, my) == makecol(0,0,0)) {
                    player_blocked_right = 1;
                    break;
                }
            }
        }
    } else {
        // pas de map
        player_y = new_y;
        if (player_y > GAME_SCREEN_H - player->h) { player_y = GAME_SCREEN_H - player->h; player_speed_y = 0; }
        if (player_y < 0)                         { player_y = 0;                         player_speed_y = 0; }
    }

    // 6) Pause
    if (key[KEY_P]) { game_paused = !game_paused; rest(200); }

    // 7) Sécurité verticale
    if (player_y > GAME_SCREEN_H - player->h) { player_y = GAME_SCREEN_H - player->h; player_speed_y = 0; }
    if (player_y < 0)                         { player_y = 0;                         player_speed_y = 0; }

    // 8) Scrolling
    if (game_started) {
        world_x += scrollspeed;
        if (world_x >= background->w) world_x = 0;
    }

    // 9) Mort si mur à droite (retourne au début du niveau ou menu)
    if (player_blocked_right) {
        player_x -= scrollspeed;
        if (player_x + player->w < 0) {
            player_lives--;
            if (player_lives > 0) {
                // ré-init joueur + objets du niveau
                player_x       = 100; player_y       = 300;
                player_speed_y = 0;   world_x        = 0;
                game_started   = 0;   start_time     = time(NULL);
                egg_active = eggr_active = eggg_active = 1;
                bombe_visible = bombe_active = 1;
                bombe_x = 2500; bombe_y = 100; bombe_vy = 0.0; bombe_explose = 0;
                player_scale = 12; gravity = 2;
                destroy_bitmap(player);
                player  = copy_bitmap_with_transparency(player_original, player_scale);
                player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                player2 = copy_bitmap_with_transparency(player2_original, player_scale);
            } else {
                // plus de vies → retour au menu
                show_end_screen();
                return;
            }
            return;
        }
    }

        // 10) Fin de niveau (drapeau)
    if (selected_level >= 0 && selected_level < NUM_LEVELS) {
        int fx = winflag_positions[selected_level][0] - world_x;
        int fy = winflag_positions[selected_level][1];
        int pw = player_x + player->w, ph = player_y + player->h;
        if (pw > fx && player_x < fx + winflag->w &&
            ph > fy && player_y < fy + winflag->h) {
            handle_level_completion();
            return;
        }
    }

    // 10bis) — ramassage œuf bleu (niveau 1)
    if (selected_level == 1) {
        int player_right = player_x + player->w;
        int player_bottom = player_y + player->h;

        for (int i = 0; i < MAX_OBSTACLES; i++) {
            int obs_screen_x = obstacle_positions[i][0] - world_x;
            int obs_screen_y = obstacle_positions[i][1];
            int obs_right = obs_screen_x + obstacle->w;
            int obs_bottom = obs_screen_y + obstacle->h;

            if (player_right > obs_screen_x &&
                player_x < obs_right &&
                player_bottom > obs_screen_y &&
                player_y < obs_bottom) {
                // Collision avec un obstacle
                player_lives--;
                if (player_lives > 0) {
                    // reset simple, même que pour le mur
                    player_x = 100; player_y = 300; player_speed_y = 0;
                    world_x = 0; game_started = 0; start_time = time(NULL);
                    egg_active = eggr_active = eggg_active = 1;
                    bombe_visible = bombe_active = 1;
                    bombe_x = 2500; bombe_y = 100; bombe_vy = 0; bombe_explose = 0;
                    player_scale = 12; gravity = 2;
                    destroy_bitmap(player);
                    player  = copy_bitmap_with_transparency(player_original, player_scale);
                    player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                    player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                } else {
                    show_end_screen();
                    return;
                }
                return;
            }
        }
        if (player_small && time(NULL) - egg_collected_time > EGG_EFFECT_DURATION) {
            player_scale = 12; // Restaure la taille normale
            player = copy_bitmap_with_transparency(player_original, player_scale);
            player1 = copy_bitmap_with_transparency(player1_original, player_scale);
            player2 = copy_bitmap_with_transparency(player2_original, player_scale);
            player_small = 0;
            gravity = 2;  // Restaure la gravité normale
        }
        if (egg_active) {
            int egg_screen_x = egg_x - world_x;
            int player_right = player_x + player->w;
            int player_bottom = player_y + player->h;

            if (player_right > egg_screen_x &&
                player_x < egg_screen_x + eggblue->w &&
                player_bottom > egg_y &&
                player_y < egg_y + eggblue->h) {

                egg_active = 0;
                player_scale = 40;
                player = copy_bitmap_with_transparency(player_original, player_scale);
                player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                player_small = 1;
                egg_collected_time = time(NULL);
                gravity = REDUCED_GRAVITY;
            }
        }
        if (eggr_active) {
            int eggr_screen_x = eggr_x - world_x;
            int player_right = player_x + player->w;
            int player_bottom = player_y + player->h;

            if (player_right > eggr_screen_x &&
                player_x < eggr_screen_x + eggred->w &&
                player_bottom > eggr_y &&
                player_y < eggr_y + eggred->h) {

                eggr_active = 0; // Désactive l'œuf après collecte
                player_scale = 5; // augmentet la taille
                player = copy_bitmap_with_transparency(player_original, player_scale);
                player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                player_small = 1;
                egg_collected_time = time(NULL); // Enregistre le temps
                gravity = AUGM_GRAVITY ; // AUGMENTE la gravité
            }
        }
    }


    if (selected_level == 2) {
        int player_right = player_x + player->w;
        int player_bottom = player_y + player->h;

        for (int i = 0; i < MAX_OBSTACLES; i++) {
            int obs2_screen_x = obstacle2_positions[i][0] - world_x;
            int obs2_screen_y = obstacle2_positions[i][1];
            int obs_right = obs2_screen_x + obstacle->w;
            int obs_bottom = obs2_screen_y + obstacle->h;

            if (player_right > obs2_screen_x &&
                player_x < obs_right &&
                player_bottom > obs2_screen_y &&
                player_y < obs_bottom) {
                // Collision avec un obstacle
                player_lives--;
                if (player_lives > 0) {
                    // reset simple, même que pour le mur
                    player_x = 100; player_y = 300; player_speed_y = 0;
                    world_x = 0; game_started = 0; start_time = time(NULL);
                    egg_active = eggr_active = eggg_active = egggrey_active = 1;
                    bombe_visible = bombe_active = 1;
                    bombe_x = 2500; bombe_y = 100; bombe_vy = 0; bombe_explose = 0;
                    player_scale = 12; gravity = 2;
                    destroy_bitmap(player);
                    player  = copy_bitmap_with_transparency(player_original, player_scale);
                    player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                    player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                } else {
                    show_end_screen();
                    return;
                }
                return;
            }
        }
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            int roue_screen_x = roue_positions[i][0] - world_x;
            int roue_screen_y = roue_positions[i][1];
            int roue_right = roue_screen_x + roue->w;
            int roue_bottom = roue_screen_y + roue->h;


            if (player_right > roue_screen_x &&
                player_x < roue_right &&
                player_bottom > roue_screen_y &&
                player_y < roue_bottom) {
                // Collision avec un obstacle
                player_lives--;
                if (player_lives > 0) {
                    // reset simple, même que pour le mur
                    player_x = 100; player_y = 300; player_speed_y = 0;
                    world_x = 0; game_started = 0; start_time = time(NULL);
                    egg_active = eggr_active = eggg_active = egggrey_active = 1;
                    bombe_visible = bombe_active = 1;
                    bombe_x = 2500; bombe_y = 100; bombe_vy = 0; bombe_explose = 0;
                    player_scale = 12; gravity = 2;
                    destroy_bitmap(player);
                    player  = copy_bitmap_with_transparency(player_original, player_scale);
                    player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                    player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                } else {
                    show_end_screen();
                    return;
                }
                return;
            }

        }

        if (player_small && time(NULL) - egg_collected_time > EGG_EFFECT_DURATION) {
            player_scale = 12; // Restaure la taille normale
            player = copy_bitmap_with_transparency(player_original, player_scale);
            player1 = copy_bitmap_with_transparency(player1_original, player_scale);
            player2 = copy_bitmap_with_transparency(player2_original, player_scale);
            player_small = 0;
            gravity = 2;  // Restaure la gravité normale
        }

// Vérifie la collision avec l'œuf
        if (egg_active) {
            int egg_screen_x = egg_x - world_x;
            int player_right = player_x + player->w;
            int player_bottom = player_y + player->h;

            if (player_right > egg_screen_x &&
                player_x < egg_screen_x + eggblue->w &&
                player_bottom > egg_y &&
                player_y < egg_y + eggblue->h) {

                egg_active = 0; // Désactive l'œuf après collecte
                player_scale = 40; // Réduit la taille
                player = copy_bitmap_with_transparency(player_original, player_scale);
                player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                player_small = 1;
                egg_collected_time = time(NULL); // Enregistre le temps
                gravity = REDUCED_GRAVITY; // Réduit la gravité
            }
        }
        // COLLISION AVEC L'ŒUF VERT
        // Collision avec l'œuf vert
        if (eggg_active && player_x + player->w >= eggg_x - world_x &&
            player_x <= eggg_x - world_x + egggreen->w &&
            player_y + player->h >= eggg_y &&
            player_y <= eggg_y + egggreen->h) {

            eggg_active = 0;
            eggg_collected_time = time(NULL);
            scrollspeed = 2;
            bombe_active = 1; // Active la bombe
            bombe_visible = 1;
            bombe_x = 2700; // Place la bombe au-dessus du joueur ou d'une position fixe
            bombe_y = 75;      // Commence en haut de l'écran
            bombe_vy = 0.0;
            // Réduction immédiate
            player_scale = 40; // taille réduite
            destroy_bitmap(player);
            player = copy_bitmap_with_transparency(player_original, player_scale);
            player1 = copy_bitmap_with_transparency(player1_original, player_scale);
            player2 = copy_bitmap_with_transparency(player2_original, player_scale);
        }
// Vérifie si 2 secondes se sont écoulées après avoir touché l'œuf vert
        if (!eggg_active && eggg_collected_time != 0) {
            if (difftime(time(NULL), eggg_collected_time) >= 2.0) {
                scrollspeed = 10;
                player_scale = 12;
                gravity = 3;// retour à la taille normale
                destroy_bitmap(player);
                player = copy_bitmap_with_transparency(player_original, player_scale);
                player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                eggg_collected_time = 0; // réinitialiser pour ne pas répéter
            }
        }

        if (eggg_collected_time > 0) {
            int effect_duration = (int) difftime(time(NULL), eggg_collected_time);
            if (effect_duration >= EGG_EFFECT_DURATION) {
                scrollspeed = 2;// retour à la normale
                eggg_collected_time = 0; // désactive le chrono
            }
        }
        if (egggrey_active) {
            int egggrey_screen_x = egggrey_x - world_x;
            int player_right = player_x + player->w;
            int player_bottom = player_y + player->h;

            if (player_right > egggrey_screen_x &&
                player_x < egggrey_screen_x + egggrey->w &&
                player_bottom > egggrey_y &&
                player_y < egggrey_y + egggrey->h) {

                egggrey_active = 0; // Désactive l'œuf après collecte

                player = copy_bitmap_with_transparency(player_original, player_scale);
                player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                egggrey_collected_time = time(NULL); // Enregistre le temps

            }
        }
        if (bombe_active) {
            int player_right = player_x + player->w;
            int player_bottom = player_y + player->h;

            if (player_right > bombe_x - world_x &&
                player_x < bombe_x - world_x + bombe->w &&
                player_bottom > bombe_y &&
                player_y < bombe_y + bombe->h) {
                player_lives--;
                if (player_lives > 0) {
                    // reset simple, même que pour le mur
                    player_x = 100; player_y = 300; player_speed_y = 0;
                    world_x = 0; game_started = 0; start_time = time(NULL);
                    egg_active = eggr_active = eggg_active = 1;
                    bombe_visible = bombe_active = 1;
                    bombe_x = 2500; bombe_y = 100; bombe_vy = 0; bombe_explose = 0;
                    player_scale = 12; gravity = 2;
                    destroy_bitmap(player);
                    player  = copy_bitmap_with_transparency(player_original, player_scale);
                    player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                    player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                } else {
                    show_end_screen();
                    return;
                }
                return;
            }
        }
        //Simuler la chute de la bombe
        if (selected_level == 2 && bombe_active && !bombe_explose) {
            bombe_vy += bombe_gravity;
            bombe_y += bombe_vy;

            if (bombe_collide_with_map()) {
                bombe_explose = 1;
                bombe_active = 0;
                explosion_timer = 0;
                explosion_frame = 0;
            }
        }
    }
}


void show_end_screen() {
    clear_keybuf();
    stop_sample(nature_sound);
    stop_sample(neige_sound);
    stop_sample(feu_sound);
    play_sample(gameover_sound, 255, 128, 1000, FALSE);

    int button_w = 200;
    int button_h = 50;
    int start_y = GAME_SCREEN_H - 200;
    const char *options[] = {"Rejouer", "Quitter"};

    int choice_made = 0;

    while (!choice_made) {
        poll_mouse();
        my_mouse_x = mouse_x;
        my_mouse_y = mouse_y;

        // Fond Game Over
        draw_sprite(buffer, end_screen_image, 0, 0);

        for (int i = 0; i < 2; i++) {
            int button_x = (GAME_SCREEN_W - button_w) / 2;
            int button_y = start_y + i * (button_h + 20);

            int mouse_over = (my_mouse_x >= button_x && my_mouse_x <= button_x + button_w &&
                              my_mouse_y >= button_y && my_mouse_y <= button_y + button_h);

            // état du bouton survolé ou non
            rectfill(buffer, button_x, button_y,
                     button_x + button_w, button_y + button_h,
                     mouse_over
                       ? makecol(120, 120, 200)
                       : makecol(80, 80, 150));

            textout_centre_ex(buffer, font, options[i],
                              GAME_SCREEN_W / 2, button_y + 15,
                              makecol(255, 255, 255), -1);

            // clic souris
            if (mouse_over && (mouse_b & 1)) {
                rest(200); // anti double-clic
                while (mouse_b & 1) poll_mouse();

                if (i == 0) {  // Rejouer
                    stop_sample(gameover_sound);

                    // --- réinitialisation du joueur & objets ---
                    player_x = 100;
                    player_y = 300;
                    player_speed_y = 0;
                    player_scale = 12;
                    destroy_bitmap(player);
                    player  = copy_bitmap_with_transparency(player_original, player_scale);
                    player1 = copy_bitmap_with_transparency(player1_original, player_scale);
                    player2 = copy_bitmap_with_transparency(player2_original, player_scale);
                    world_x = 0;
                    animation_frame = 0;
                    game_started = 0;
                    start_time = time(NULL);
                    elapsed_seconds = 0;
                    game_paused = 0;
                    gravity = 2;
                    player_lives = 3;

                    egg_active = 1;
                    eggr_active = 1;
                    eggg_active = 1;
                    bombe_active = 1;
                    bombe_x = 2500;
                    bombe_y = 100;
                    bombe_vy = 0.0;
                    bombe_visible = 0;
                    bombe_explose = 0;
                    explosion_frame = 0;
                    explosion_timer = 0;

                    // --- LANCER FORCÉ DU NIVEAU 1 ---
                    selected_level = 0;
                    load_map_for_selected_level();
                    play_sample(neige_sound, 100, 128, 1000, FALSE);

                    game_state = PLAYING;
                }
                else {  // Quitter → retour menu
                    game_state = MENU;
                    game_started = 0;
                    selected_level = -1;
                }

                choice_made = 1;
            }
        }

        blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    }
}



void show_victory_screen(){
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

    // Réinitialisation du jeu
    game_state = MENU;
    game_started = 0;
    selected_level = -1;
    player_x = 100;
    player_y = 300;
    world_x = 0;
    player_speed_y = 0;
    clear_keybuf();
}

void handle_level_completion() {
    stop_sample(nature_sound);
    stop_sample(neige_sound);
    stop_sample(feu_sound);
    play_sample(victoire_sound, 100, 128, 1000, FALSE);

    rest(1000); // Petite pause de transition

    if (selected_level < NUM_LEVELS - 1) {
        // Passer au niveau suivant
        selected_level++;
        player_x = 100;
        player_y = 300;
        world_x = 0;
        player_speed_y = 0;
        player_scale = 12;
        gravity = 2;
        player = copy_bitmap_with_transparency(player_original, player_scale);
        player1 = copy_bitmap_with_transparency(player1_original, player_scale);
        player2 = copy_bitmap_with_transparency(player2_original, player_scale);

        egg_active = 1;
        eggr_active = 1;
        eggg_active = 1;
        bombe_active = 1;
        bombe_x = 2500;
        bombe_y = 100;
        bombe_vy = 0.0;
        bombe_visible = 0;
        bombe_explose = 0;
        explosion_frame = 0;
        explosion_timer = 0;

        game_started = 0;
        start_time = time(NULL);
        elapsed_seconds = 0;
        game_paused = 0;

        load_map_for_selected_level();
        game_state = PLAYING;

        switch (selected_level) {
            case 0: play_sample(nature_sound, 100, 128, 1000, FALSE); break;
            case 1: play_sample(neige_sound, 100, 128, 1000, FALSE); break;
            case 2: play_sample(feu_sound, 100, 128, 1000, FALSE); break;
        }
    } else {
        // Fin du dernier niveau : écran de victoire
        show_victory_screen();
    }
}

void show_pause_screen(){
    clear_to_color(buffer, makecol(0, 0, 0)); // fond noir
    textout_centre_ex(buffer, font, "PAUSE", GAME_SCREEN_W / 2, GAME_SCREEN_H / 2 - 20, makecol(255, 255, 255), -1);
    textout_centre_ex(buffer, font, "Appuyez sur 'P' pour reprendre", GAME_SCREEN_W / 2, GAME_SCREEN_H / 2 + 20, makecol(200, 200, 200), -1);
    blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
}


void draw_rotating_obstacle(int x, int y){
    obstacle_angle += 0.05; // Ajuste pour la vitesse de rotation

    if (obstacle_angle >= 2 * M_PI)
        obstacle_angle -= 2 * M_PI;

    rotate_sprite(buffer, obstacle, x, y, ftofix(obstacle_angle * 128 / M_PI));
}

void draw_rotating_roue(int x, int y){
    roue_angle += 0.1; // Ajuste pour la vitesse de rotation

    if (roue_angle >= 2 * M_PI)
        roue_angle -= 2 * M_PI;

    rotate_sprite(buffer, roue, x, y, ftofix(roue_angle * 128 / M_PI));
}
void draw_timer(){
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

void draw_game(){

    if (selected_level == 2) {
        blit(background3, buffer, world_x, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    }
    else if (selected_level == 1) {
        blit(background2, buffer, world_x, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    }
    else {
        blit(background, buffer, world_x, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    }

    if (selected_level >= 0 && selected_level < NUM_LEVELS) {
        int flag_x = winflag_positions[selected_level][0] - world_x;
        int flag_y = winflag_positions[selected_level][1];

        if (flag_x + winflag->w >= 0 && flag_x <= GAME_SCREEN_W) {
            draw_sprite(buffer, winflag, flag_x, flag_y);
        }
    }
    int map_pos1 = -world_x;
    int map_pos2 = map_pos1 + map_overlay->w;

    draw_sprite(buffer, map_overlay, map_pos1, 0);
    if (map_pos2 < GAME_SCREEN_W) {
        draw_sprite(buffer, map_overlay, map_pos2, 0);
    }


    if (selected_level == 1) {
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            int obs_screen_x = obstacle_positions[i][0] - world_x;
            int obs_screen_y = obstacle_positions[i][1];

            if (obs_screen_x + obstacle->w >= 0 && obs_screen_x < GAME_SCREEN_W) {
                draw_rotating_obstacle(obs_screen_x, obs_screen_y);
            }

        }
        if (egg_active) {
            int egg_screen_x = egg_x - world_x;
            draw_sprite(buffer, eggblue, egg_screen_x, egg_y);
        }
        if (eggr_active) {

            draw_sprite(buffer, eggred, eggr_x - world_x, eggr_y);

        }

        char texte_pseudo[60];
        sprintf(texte_pseudo, "Joueur : %s", pseudo_joueur);
        int largeur_texte = text_length(font, texte_pseudo);
        textout_ex(buffer, font, texte_pseudo,
                   GAME_SCREEN_W - largeur_texte - 10,
                   10,
                   makecol(255, 255, 255), -1);

    }

    if (selected_level == 2) {
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            int obs2_screen_x = obstacle2_positions[i][0] - world_x;
            int obs2_screen_y = obstacle2_positions[i][1];

            if (obs2_screen_x + obstacle->w >= 0 && obs2_screen_x < GAME_SCREEN_W) {
                draw_rotating_obstacle(obs2_screen_x, obs2_screen_y);
            }

        }
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            int roue_screen_x = roue_positions[i][0] - world_x;
            int roue_screen_y = roue_positions[i][1];

            if (roue_screen_x + roue->w >= 0 && roue_screen_x < GAME_SCREEN_W) {
                draw_rotating_roue(roue_screen_x, roue_screen_y);
            }

        }
        if (egg_active) {
            int egg_screen_x = egg_x - world_x;
            draw_sprite(buffer, eggblue, egg_screen_x, egg_y);
        }
        if (eggg_active) {

            draw_sprite(buffer, egggreen, eggg_x - world_x, eggg_y);
        }
        if (bombe_visible && bombe_active) {
            draw_sprite(buffer, bombe, bombe_x - world_x, bombe_y);
        }
        if (bombe_explose) {
            explosion_timer++;

            BITMAP* current_explosion = NULL;
            if (explosion_timer < 10) current_explosion = explosion1;
            else if (explosion_timer < 20) current_explosion = explosion2;
            else if (explosion_timer < 30) current_explosion = explosion3;
            else bombe_explose = 0;  // Fin de l'explosion

            if (current_explosion) {
                draw_sprite(buffer, current_explosion, bombe_x - world_x, bombe_y);
            }
        } else if (bombe_active) {
            draw_sprite(buffer, bombe, bombe_x - world_x, bombe_y);
        }



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
    // Affichage du pseudo du joueur en haut à droite
    char texte_pseudo[60];
    sprintf(texte_pseudo, "Joueur : %s", pseudo_joueur);
    int largeur_texte = text_length(font, texte_pseudo);
    textout_ex(buffer, font, texte_pseudo,
               GAME_SCREEN_W - largeur_texte - 10,  // position droite
               10,                                  // position verticale
               makecol(255, 255, 255), -1);         // couleur texte

    // Affichage des vies
    for (int i = 0; i < player_lives; i++) {
        draw_sprite(buffer, heart_icon, 10 + i * (heart_icon->w + 5), 40);
    }
    // Sélection du sprite actif
    BITMAP *current_sprite = player;

    if (key[KEY_SPACE]) {
        animation_frame++;
        play_sample(jump_sound, 20, 40, 1000, FALSE);
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
    draw_sprite(buffer, menu_background, 0, 0); // fond du menu

    // Affichage du logo BADLAND centré
    if (badland_logo) {
        int logo_x = (GAME_SCREEN_W - badland_logo->w) / 2;
        int logo_y = 60;
        draw_sprite(buffer, badland_logo, logo_x, logo_y);
    }

    // Coordonnées du bouton "JOUER"
    int button_width = 200;
    int button_height = 60;
    int button_x = (GAME_SCREEN_W - button_width) / 2;
    int button_y = 300;

    poll_mouse();
    my_mouse_x = mouse_x;
    my_mouse_y = mouse_y;

    int mouse_over_button = (my_mouse_x >= button_x && my_mouse_x <= button_x + button_width &&
                             my_mouse_y >= button_y && my_mouse_y <= button_y + button_height);

    int fill_color = mouse_over_button ? makecol(100, 0, 0) : makecol(50, 0, 0);
    int border_color = makecol(255, 255, 255);
    int text_color = makecol(255, 255, 255);

    // Bouton JOUER
    rectfill(buffer, button_x, button_y, button_x + button_width, button_y + button_height, fill_color);
    rect(buffer, button_x, button_y, button_x + button_width, button_y + button_height, border_color);

    textout_centre_ex(buffer, font, "JOUER", button_x + button_width / 2,
                      button_y + (button_height / 2) - text_height(font) / 2,
                      text_color, -1);

    // Bouton CHARGER
    int charger_button_y = button_y + 100;

    int charger_mouse_over = (my_mouse_x >= button_x && my_mouse_x <= button_x + button_width &&
                              my_mouse_y >= charger_button_y && my_mouse_y <= charger_button_y + button_height);

    int charger_fill_color = charger_mouse_over ? makecol(100, 0, 0) : makecol(50, 0, 0);

    rectfill(buffer, button_x, charger_button_y,
             button_x + button_width, charger_button_y + button_height, charger_fill_color);
    rect(buffer, button_x, charger_button_y,
         button_x + button_width, charger_button_y + button_height, border_color);

    textout_centre_ex(buffer, font, "CHARGER",
                      button_x + button_width / 2,
                      charger_button_y + (button_height / 2) - text_height(font) / 2,
                      text_color, -1);

    // Message entre les deux boutons
    int message_y = button_y + button_height + 10;
    textout_centre_ex(buffer, font, "Appuyez sur ESPACE pour commencer",
                      GAME_SCREEN_W / 2, message_y,
                      text_color, -1);

    // Actions clics
    if (charger_mouse_over && (mouse_b & 1)) {
        rest(200);
        allegro_message("Aucune sauvegarde trouvée");
    }
    if (mouse_over_button && (mouse_b & 1)) {
        game_state = START_MENU;  // vers écran de pseudo
        rest(200);
    }
}


void load_map_for_selected_level() {
    if (map_overlay) destroy_bitmap(map_overlay);  // Nettoie l’ancienne map si besoin

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
            break;    // Ajoute d'autres niveaux ici
        default:
            map_overlay = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);  // Map vide par défaut
            clear_to_color(map_overlay, makecol(0, 0, 0));
            break;
    }

    if (!map_overlay) {
        allegro_message("Erreur de chargement de la carte pour le niveau sélectionné !");
        exit(1);
    }
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
void play_music(SAMPLE* music) {
    if (current_music != music) {
        stop_sample(current_music);  // Arrête l'ancienne musique si besoin
        current_music = music;
        if (current_music) {
            play_sample(current_music, 250, 128, 1000, TRUE);
        }
    }
}void draw_start_menu() {
    clear_bitmap(buffer);
    draw_sprite(buffer, menu_background, 0, 0);

    // Afficher le logo Badland en haut
    if (badland_logo) {
        int logo_x = (GAME_SCREEN_W - badland_logo->w) / 2;
        draw_sprite(buffer, badland_logo, logo_x, 30);
    }

    // Cadre pour la saisie du pseudo
    int box_w = 400;
    int box_h = 200;
    int box_x = (GAME_SCREEN_W - box_w) / 2;
    int box_y = 200;

    rectfill(buffer, box_x, box_y, box_x + box_w, box_y + box_h, makecol(0, 0, 0));
    rect(buffer, box_x, box_y, box_x + box_w, box_y + box_h, makecol(255, 255, 255));

    textout_centre_ex(buffer, font, "Entrez votre pseudo :", GAME_SCREEN_W / 2, box_y + 30, makecol(255, 255, 255), -1);

    // Zone de texte (pseudo actuel saisi)
    static char pseudo[50] = "";
    static int pos = 0;

    rectfill(buffer, box_x + 50, box_y + 70, box_x + box_w - 50, box_y + 100, makecol(30, 30, 60));
    rect(buffer, box_x + 50, box_y + 70, box_x + box_w - 50, box_y + 100, makecol(200, 200, 200));
    textout_ex(buffer, font, pseudo, box_x + 60, box_y + 75, makecol(255, 255, 255), -1);

    // Instructions
    textout_centre_ex(buffer, font, "ENTRER : Charger / Créer   |   ESC : Retour",
                      GAME_SCREEN_W / 2, box_y + 150, makecol(180, 180, 180), -1);

    // Lecture clavier
    if (keypressed()) {
        int c = readkey() & 0xFF;
        if (c == 8 && pos > 0) { // Backspace
            pos--;
            pseudo[pos] = '\0';
        } else if (c >= 32 && c <= 126 && pos < 48) { // caractères visibles
            pseudo[pos++] = (char)c;
            pseudo[pos] = '\0';
        } else if (c == 13 && strlen(pseudo) > 0) { // Entrée
            strcpy(pseudo_joueur, pseudo);
            int niv = charger_niveau_pseudo(pseudo);
            selected_level = (niv >= 0) ? niv : 0;
            game_state = LEVEL_SELECTION;
        } else if (c == 27) { // ESC
            game_state = MENU;
        }
    }

    // Affichage final
    blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
}


void draw() {
    clear_bitmap(buffer);

    if (game_state == MENU) {
        draw_menu();
    }
    else if (game_state == LEVEL_SELECTION) {
        draw_level_selection();
        load_map_for_selected_level();
    }
    else if (game_state == PLAYING) {
        draw_game();
    }
    if (game_state == MENU || game_state == LEVEL_SELECTION) {
        play_music(jungle_sound);
    }
    else if (game_state == SUBMENU) {
        draw_start_menu();
    }
    else if (game_state == START_MENU) {
        draw_start_menu();
    }



    blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
}



int main() {
    init();

    while (!key[KEY_ESC]) {
        show_mouse(screen);

        if (game_state == MENU && key[KEY_SPACE]) {
            game_state = SUBMENU;  // Redirige vers le nouveau menu
            rest(200);
        }


        if (!game_paused) {
            update_physics();
            draw();
            rest(20);
        } else {
            show_pause_screen();
            if (key[KEY_P]) {
                game_paused = !game_paused;
                rest(200); // anti-rebond
            }
        }
    }

    deinit();
    return 0;
}
END_OF_MAIN();