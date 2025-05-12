// main.c — refacto progressif : Étape 3
// Objectif : migrer les bitmaps de décor (background*, map_overlay) vers la
//            structure Game et rendre update_physics() & draw_game() sans
//            dépendances globales pour ces ressources.
// ---------------------------------------------------------------------------

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
#define EGG_EFFECT_DURATION 15
#define REDUCED_GRAVITY 1
#define AUGM_GRAVITY 3
static float obstacle_angle = 0;

/* ---------------------------------------------------------------------------
   Structure Game (enrichie à chaque étape) */
typedef struct {
    /* --- Menu ----------------------------------------------------------- */
    BITMAP *menu_background;
    BITMAP *play_button;
    BITMAP *play_button_hover;
    BITMAP *badland_logo;

    /* --- Décor & niveaux ---------------------------------------------- */
    BITMAP *background;
    BITMAP *background2;
    BITMAP *background3;
    BITMAP *map_level1;
    BITMAP *map_level2;
    BITMAP *map_level3;
    BITMAP *level_selection_background;

    /* --- Sprites joueur ---------------------------------------------- */
    BITMAP *player_original;
    BITMAP *player;
    BITMAP *player1_original;
    BITMAP *player2_original;
    BITMAP *player1;
    BITMAP *player2;
    int     player_scale;          /* (valeur 12 au départ) */

    /* --- Sons --------------------------------------------------------- */
    SAMPLE *jungle_sound;
    SAMPLE *nature_sound;
    SAMPLE *neige_sound;
    SAMPLE *feu_sound;
    SAMPLE *jump_sound;
    SAMPLE *gameover_sound;
    SAMPLE *victoire_sound;

    SAMPLE *current_music;   /* pour savoir quelle musique est en cours */

    /* --- Physique & animation ----------------------------------------- */
    int player_x, player_y;
    int player_speed_y;
    int world_x;
    int gravity;
    int scrollspeed;
    int animation_frame;
    int     saved_player_scale;    /* mémorise la taille normale     */

    /* --- État du jeu --------------------------------------------------- */
    int selected_level;         /* 0-2 ; -1 tant qu’aucun niveau choisi */

    /* --- Bitmaps gameplay --------------------------------------------- */
    BITMAP *winflag;
    BITMAP *obstacle;
    BITMAP *egg_blue;
    BITMAP *egg_red;
    BITMAP *egg_green;
    BITMAP *bomb;

    /* --- Écrans de fin ------------------------------------------------- */
    BITMAP *end_screen;   /* game-over */
    BITMAP *victory;      /* victoire */

    /* --- État des œufs & bombe --------------------------------------- */
    int  egg_blue_x,  egg_blue_y,  egg_blue_active;
    int  egg_red_x,   egg_red_y,   egg_red_active;
    int  egg_green_x, egg_green_y, egg_green_active;

    int  bomb_x, bomb_y;
    int  bomb_active;
    int  bomb_visible;
    float bomb_vy;
    float bomb_gravity;   /* ex : 0.09f */

    /* --- Effets temporaires ----------------------------------------- */
    int   egg_timer;          /* compte à rebours après collision œuf   */
    int   saved_gravity;      /* gravité avant application de l’œuf     */


} Game;

/* ---------------------------------------------------------------------------
   Variables globales ENCORE non migrées */
int game_state = MENU;
BITMAP *buffer;

/* ------------------------------------------------------------------ */
/* Obstacles fixes du niveau 1 (centre du sprite de l’obstacle)       */
static const int OBSTACLES_LVL2[10][2] = {
    {600,200},{2350,100},{2950,450},{3600,50},
    {3850,500},{4000,50},{5100,450},{5380,280},
    {5650,450},{5900,280}
};

/* Obstacles fixes du niveau 3 (selected_level == 2) — choisis tes coords */
static const int OBSTACLES_LVL3[12][2] = {
    { 800,100},{1550,420},{2300, 70},{3050,500},
    {3800,220},{4550,350},{5300,130},{6050,480},
    {6800,260},{7550, 60},{8300,400},{9000,180}
};
/* ------------------------------------------------------------------ */

/* (le reste des variables de gameplay)… */
int my_mouse_x, my_mouse_y;
int play_button_x, play_button_y;
int play_button_width = 200;
int play_button_height = 80;

/* retourne 1 si deux rectangles (A et B) se recouvrent, 0 sinon */
static int rect_overlap(int ax,int ay,int aw,int ah,
                        int bx,int by,int bw,int bh)
{
    return (ax < bx + bw) && (ax + aw > bx) &&
           (ay < by + bh) && (ay + ah > by);
}

/*  Copie un bitmap dans le format vidéo courant (profondeur écran)  */
static BITMAP *convert_to_screen_depth(BITMAP *src)
{
    if (!src) return NULL;                             /* sécurité */

    int depth = bitmap_color_depth(screen);            /* 15/16/24/32 */
    BITMAP *dst = create_bitmap_ex(depth, src->w, src->h);
    if (!dst) return src;                              /* si échec, on garde l’original */

    blit(src, dst, 0,0, 0,0, src->w, src->h);          /* recopie intégrale */
    destroy_bitmap(src);                               /* on libère l’ancienne surface */
    return dst;
}


/* retourne 1 si le pixel (x,y) du calque n’est PAS magenta (= obstacle) */
/* renvoie 1 si le pixel (x,y) du calque est plein (≠ MAGENTA) */
static inline int map_solid(Game *g, int x, int y)
{
    BITMAP *ov = (g->selected_level == 2) ? g->map_level3 :
                 (g->selected_level == 1) ? g->map_level2 :
                                            g->map_level1;
    if (!ov) return 0;
    int mx = (x + g->world_x) % ov->w;   /* translation + boucle */
    if (y < 0 || y >= ov->h) return 0;
    return getpixel(ov, mx, y) != MAGENTA;   /* noir = plein, magenta = vide */
}




/* ---------------------------------------------------------------------------
   Prototypes : update_physics et draw_game reçoivent Game* */
BITMAP* copy_bitmap_with_transparency(BITMAP *src, int scale_factor);
void game_init(Game *g);
void game_deinit(Game *g);
void update_physics(Game *g);
void draw_timer();
void draw_game(Game *g);
void draw_menu(Game *g);
void draw_level_selection(Game *g);
void draw(Game *g);
void play_music(Game *g, SAMPLE *track);
void show_end_screen(Game *g);
void show_victory_screen(Game *g);

/* --------------------------------------------------------------------------- */
BITMAP* copy_bitmap_with_transparency(BITMAP *src, int scale_factor) {
    int new_w = src->w / scale_factor;
    int new_h = src->h / scale_factor;
    BITMAP *result = create_bitmap(new_w, new_h);
    clear_to_color(result, MAGENTA);
    for (int y = 0; y < new_h; y++)
        for (int x = 0; x < new_w; x++) {
            int c = getpixel(src, x * scale_factor, y * scale_factor);
            if (c != MAGENTA) putpixel(result, x, y, c);
        }
    return result;
}

/* redimensionne les 3 sprites du joueur selon g->player_scale */
static void rebuild_player_sprites(Game *g)
{
    /* libère les anciennes versions */
    if (g->player)  destroy_bitmap(g->player);
    if (g->player1) destroy_bitmap(g->player1);
    if (g->player2) destroy_bitmap(g->player2);

    g->player  = copy_bitmap_with_transparency(g->player_original,  g->player_scale);
    g->player1 = copy_bitmap_with_transparency(g->player1_original, g->player_scale);
    g->player2 = copy_bitmap_with_transparency(g->player2_original, g->player_scale);
}


/* ---------------------------------------------------------------------------
   game_init : charge backgrounds + menu dans la structure */


void game_init(Game *g) {
    allegro_init();
    install_keyboard();
    install_mouse();
    install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, GAME_SCREEN_W, GAME_SCREEN_H, 0, 0);

    buffer = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);

    /* ----- Décors ------------------------------------------------------ */
    g->background  = load_bitmap("background.bmp",  NULL);
    g->background2 = load_bitmap("background2.bmp", NULL);
    g->background3 = load_bitmap("background3.bmp", NULL);
    if (!g->background || !g->background2 || !g->background3) {
        allegro_message("Erreur chargement background(s) !"); exit(1);
    }

    /* --- map_level ? --------------------------------------------------- */
    g->map_level1 = convert_to_screen_depth(load_bitmap("map_level1.bmp", NULL));
    g->map_level2 = convert_to_screen_depth(load_bitmap("map_level2.bmp", NULL));
    g->map_level3 = convert_to_screen_depth(load_bitmap("map_level3.bmp", NULL));

    if (!g->map_level1 || !g->map_level2 || !g->map_level3) {
        allegro_message("Erreur chargement map_level?.bmp !");
        exit(1);
    }



    /* ----- Sprites joueur ---------------------------------------------- */
    g->player_original  = load_bitmap("player.bmp",  NULL);
    g->player1_original = load_bitmap("player1.bmp", NULL);
    g->player2_original = load_bitmap("player2.bmp", NULL);

    g->player_scale     = 12;
    g->saved_player_scale  = 12;

    g->player  = copy_bitmap_with_transparency(g->player_original,  g->player_scale);
    g->player1 = copy_bitmap_with_transparency(g->player1_original, g->player_scale);
    g->player2 = copy_bitmap_with_transparency(g->player2_original, g->player_scale);

    if (!g->player || !g->player1 || !g->player2) {
        allegro_message("Erreur chargement sprites joueur !");
        exit(1);
    }

    /* ----- Menu (étape 2, inchangé) ------------------------------------ */
    BITMAP *tmp_logo  = load_bitmap("badland_logo.bmp", NULL);
    int nw = GAME_SCREEN_W * 0.6;
    int nh = tmp_logo ? tmp_logo->h * nw / tmp_logo->w : 150;
    g->badland_logo   = create_bitmap(nw, nh);
    if (tmp_logo) {
        stretch_blit(tmp_logo, g->badland_logo, 0,0,tmp_logo->w,tmp_logo->h, 0,0,nw,nh);
        destroy_bitmap(tmp_logo);
    }

    BITMAP *tmp_menu  = load_bitmap("menu_background.bmp", NULL);
    if (tmp_menu) {
        g->menu_background = create_bitmap(GAME_SCREEN_W,GAME_SCREEN_H);
        stretch_blit(tmp_menu, g->menu_background, 0,0,tmp_menu->w,tmp_menu->h,0,0,GAME_SCREEN_W,GAME_SCREEN_H);
        destroy_bitmap(tmp_menu);
    } else {
        g->menu_background = create_bitmap(GAME_SCREEN_W,GAME_SCREEN_H);
        clear_to_color(g->menu_background, makecol(50,50,100));
        textout_centre_ex(g->menu_background, font, "BADLAND GAME", GAME_SCREEN_W/2, GAME_SCREEN_H/3, makecol(255,255,255), -1);
    }

    g->play_button        = load_bitmap("play_button.bmp", NULL);
    g->play_button_hover  = load_bitmap("play_button_hover.bmp", NULL);
    if (!g->play_button) {
        g->play_button = create_bitmap(play_button_width,play_button_height);
        clear_to_color(g->play_button, makecol(70,70,150));
        textout_centre_ex(g->play_button, font, "JOUER", play_button_width/2, play_button_height/2 - text_height(font)/2, makecol(255,255,255), -1);
    }
    if (!g->play_button_hover) {
        g->play_button_hover = create_bitmap(play_button_width,play_button_height);
        clear_to_color(g->play_button_hover, makecol(100,100,200));
        textout_centre_ex(g->play_button_hover, font, "JOUER", play_button_width/2, play_button_height/2 - text_height(font)/2, makecol(255,255,0), -1);
    }

    /* placement */
    play_button_x = (GAME_SCREEN_W - play_button_width)/2;
    int total_h = g->badland_logo->h + 20 + play_button_height + 20 + text_height(font);
    int top = (GAME_SCREEN_H - total_h)/2;
    play_button_y = top + g->badland_logo->h + 20;

    /* ----- Sons ----------------------------------------------------------- */
    #define LOAD_WAV(dest, file)                         \
        do {                                             \
            g->dest = load_sample(file);                 \
            if (!g->dest) {                              \
                allegro_message("Erreur chargement %s", file); \
                exit(1);                                 \
            }                                            \
        } while (0)

    LOAD_WAV(jungle_sound,  "jungle.wav");
    LOAD_WAV(nature_sound,  "nature.wav");
    LOAD_WAV(neige_sound,   "neige.wav");
    LOAD_WAV(feu_sound,     "feu.wav");
    LOAD_WAV(jump_sound,    "jump.wav");
    LOAD_WAV(gameover_sound,"gameover.wav");
    LOAD_WAV(victoire_sound,"victoire.wav");

    g->current_music = NULL;
    #undef LOAD_WAV

    /* --- Fond de l’écran de sélection de niveau ----------------------- */
    BITMAP *tmp_lvl = load_bitmap("level_selection_background.bmp", NULL);
    if (tmp_lvl) {
        g->level_selection_background = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        stretch_blit(tmp_lvl, g->level_selection_background,
                     0, 0, tmp_lvl->w, tmp_lvl->h,
                     0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
        destroy_bitmap(tmp_lvl);
    } else {
        g->level_selection_background = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
        clear_to_color(g->level_selection_background, makecol(30, 30, 60));
    }

    /* --- Bitmaps gameplay --------------------------------------------- */
    g->winflag  = copy_bitmap_with_transparency(load_bitmap("winflag.bmp",  NULL),  2);
    g->obstacle = copy_bitmap_with_transparency(load_bitmap("obstacle.bmp", NULL), 2);

    if (!g->winflag || !g->obstacle) {
        allegro_message("Erreur chargement winflag/obstacle !");
        exit(1);
    }

    /* --- Œufs & bombe --------------------------------------------------- */
    g->egg_blue = copy_bitmap_with_transparency(load_bitmap("eggblue.bmp", NULL), 2);
    g->egg_red = copy_bitmap_with_transparency(load_bitmap("eggred.bmp", NULL), 2);
    g->egg_green = copy_bitmap_with_transparency(load_bitmap("egggreen.bmp", NULL), 2);
    g->bomb       = copy_bitmap_with_transparency(load_bitmap("bombe.bmp",    NULL), 2);

    if (!g->egg_blue || !g->egg_red || !g->egg_green || !g->bomb) {
        allegro_message("Erreur chargement egg/bombe !");
        exit(1);
    }

    /* --- Coordonnées / état par défaut -------------------------------- */
    g->egg_blue_x   = 1500; g->egg_blue_y   = 300; g->egg_blue_active  = 1;
    g->egg_red_x    = 3450; g->egg_red_y    = 450; g->egg_red_active   = 1;
    g->egg_green_x  = 2350; g->egg_green_y  = 260; g->egg_green_active = 1;

    g->bomb_x       = 2700; g->bomb_y       =  50;
    g->bomb_active  = 1;
    g->bomb_visible = 0;        /* elle ne tombe que quand on active l’œuf vert */
    g->bomb_vy      = 0.0f;
    g->bomb_gravity = 0.09f;


    if (!g->egg_blue)   printf("egg_blue NULL\n");
    if (!g->egg_red)    printf("egg_red  NULL\n");
    if (!g->egg_green)  printf("egg_green NULL\n");
    if (!g->bomb)       printf("bomb NULL\n");

    g->egg_timer      = 0;
    g->gravity        = 2;
    g->saved_gravity  = g->gravity;


    /* --- Écrans de fin ------------------------------------------------- */
    BITMAP *tmp_end = load_bitmap("terminer.bmp", NULL);
    g->end_screen = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
    stretch_blit(tmp_end, g->end_screen,
                 0, 0, tmp_end->w, tmp_end->h,
                 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    destroy_bitmap(tmp_end);

    BITMAP *tmp_vic = load_bitmap("victoire.bmp", NULL);
    g->victory = create_bitmap(GAME_SCREEN_W, GAME_SCREEN_H);
    stretch_blit(tmp_vic, g->victory,
                 0, 0, tmp_vic->w, tmp_vic->h,
                 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
    destroy_bitmap(tmp_vic);

    /* --- Physique ------------------------------------------------------ */
    g->player_x       = 100;
    g->player_y       = 300;
    g->player_speed_y = 0;
    g->world_x        = 0;
    g->gravity        = 2;
    g->scrollspeed    = 2;
    g->animation_frame= 0;
    g->selected_level = -1;   /* aucun niveau tant que le joueur n’a pas choisi */


}

/* --------------------------------------------------------------------------- */
void game_deinit(Game *g)
{
    /* --- surfaces génériques ----------------------------------------- */
    destroy_bitmap(buffer);

    destroy_bitmap(g->background);
    destroy_bitmap(g->background2);
    destroy_bitmap(g->background3);
    destroy_bitmap(g->map_level1);
    destroy_bitmap(g->map_level2);
    destroy_bitmap(g->map_level3);

    /* --- bitmaps du menu --------------------------------------------- */
    destroy_bitmap(g->menu_background);
    destroy_bitmap(g->play_button);
    destroy_bitmap(g->play_button_hover);
    destroy_bitmap(g->badland_logo);
    destroy_bitmap(g->level_selection_background);

    /* --- sprites joueur ---------------------------------------------- */
    destroy_bitmap(g->player_original);
    destroy_bitmap(g->player1_original);
    destroy_bitmap(g->player2_original);
    destroy_bitmap(g->player);
    destroy_bitmap(g->player1);
    destroy_bitmap(g->player2);

    destroy_sample(g->jungle_sound);
    destroy_sample(g->nature_sound);
    destroy_sample(g->neige_sound);
    destroy_sample(g->feu_sound);
    destroy_sample(g->jump_sound);
    destroy_sample(g->gameover_sound);
    destroy_sample(g->victoire_sound);

    destroy_bitmap(g->winflag);
    destroy_bitmap(g->obstacle);

    destroy_bitmap(g->end_screen);
    destroy_bitmap(g->victory);

    destroy_bitmap(g->egg_blue);
    destroy_bitmap(g->egg_red);
    destroy_bitmap(g->egg_green);
    destroy_bitmap(g->bomb);

    /* --- à venir : autres bitmaps / samples restant à migrer ---------- */
}


/* ---------------------------------------------------------------------------
   update_physics  – physique + collisions œufs / bombe / obstacles
--------------------------------------------------------------------------- */
void update_physics(Game *g)
{
    /* -------- scroll selon niveau ----------------------------------- */
    g->scrollspeed = (g->selected_level == 2) ? 6 :
                     (g->selected_level == 1) ? 4 : 2;

    /* -------- entrée saut ------------------------------------------- */
    if (key[KEY_SPACE])
        g->player_speed_y = JUMP_STRENGTH;

    /* -------- gravité & déplacement vertical ------------------------ */
    g->player_speed_y += g->gravity;
    g->player_y       += g->player_speed_y;

    if (g->player_y > GAME_SCREEN_H - g->player->h) { g->player_y = GAME_SCREEN_H - g->player->h; g->player_speed_y = 0; }
    if (g->player_y < 0)                            { g->player_y = 0;                            g->player_speed_y = 0; }

    /* -------- scrolling horizontal ---------------------------------- */
    g->world_x += g->scrollspeed;
    if (g->world_x >= g->background->w) g->world_x = 0;

    /* ---------- collision avec obstacles fixes (niveaux 2 & 3) ---------- */
    const int (*obstable)[2] = NULL;
    int nb_obs = 0;

    if      (g->selected_level == 1) { obstable = OBSTACLES_LVL2; nb_obs = 10; }
    else if (g->selected_level == 2) { obstable = OBSTACLES_LVL3; nb_obs = 12; }

    if (obstable) {
        int pright  = g->player_x + g->player->w;
        int pbottom = g->player_y + g->player->h;

        for (int i = 0; i < nb_obs; ++i) {
            int ox = obstable[i][0] - g->world_x;
            int oy = obstable[i][1];

            int oright  = ox + g->obstacle->w;
            int obottom = oy + g->obstacle->h;

            if (pright > ox && g->player_x < oright &&
                pbottom > oy && g->player_y < obottom) {
                show_end_screen(g);
                return;
                }
        }
    }


    int px = g->player_x;
    int py = g->player_y;
    int pw = g->player->w;
    int ph = g->player->h;

    /* ---- œuf bleu : gravité réduite + joueur plus petit -------------- */
    if (g->egg_blue_active &&
        rect_overlap(px,py,pw,ph,
                     g->egg_blue_x - g->world_x, g->egg_blue_y,
                     g->egg_blue->w, g->egg_blue->h))
    {
        g->egg_blue_active   = 0;

        /* --- effet gravité ------------------------------------------------ */
        g->saved_gravity     = g->gravity;
        g->gravity           = REDUCED_GRAVITY;          /* 1 */

        /* --- effet taille -------------------------------------------------- */
        g->saved_player_scale = g->player_scale;         /* mémorise 12 */
        g->player_scale       = 40;                      /* 40 → beaucoup plus petit */
        rebuild_player_sprites(g);                       /* régénère les bitmaps    */

        g->egg_timer         = EGG_EFFECT_DURATION * 50; /* ~1 s à 20 FPS */
    }

    /* ---- œuf rouge : agrandit + gravité augmentée ------------------ */
    if (g->egg_red_active &&
        rect_overlap(px,py,pw,ph,
                     g->egg_red_x - g->world_x, g->egg_red_y,
                     g->egg_red->w, g->egg_red->h))
    {
        g->egg_red_active     = 0;

        /* on sauvegarde l'état actuel pour le restaurer plus tard */
        g->saved_gravity      = g->gravity;
        g->saved_player_scale = g->player_scale;

        /* effet de l'œuf rouge */
        g->gravity      = AUGM_GRAVITY;   /* chute plus rapide */
        g->player_scale = 5;              /* sprite nettement plus grand */
        rebuild_player_sprites(g);

        g->egg_timer = EGG_EFFECT_DURATION * 50;   /* durée ≈ 1 s à 20 FPS */
    }


    /* ---- œuf vert : déclenche la chute de la bombe ----------------- */
    if (g->egg_green_active &&
        rect_overlap(px,py,pw,ph,
                     g->egg_green_x - g->world_x, g->egg_green_y,
                     g->egg_green->w, g->egg_green->h))
    {
        g->egg_green_active = 0;
        g->bomb_visible     = 1;       /* la bombe commence à tomber   */
    }

    /* ---- bombe – game-over ----------------------------------------- */
    if (g->bomb_visible && g->bomb_active)
    {
        /* physique de la bombe */
        g->bomb_vy += g->bomb_gravity;
        g->bomb_y  += (int)g->bomb_vy;

        /* collision joueur */
        if (rect_overlap(px,py,pw,ph,
                         g->bomb_x - g->world_x, g->bomb_y,
                         g->bomb->w, g->bomb->h))
        {
            g->bomb_active = 0;
            show_end_screen(g);
            return;
        }

        /* touche le sol ? -> on considère perdu également */
        if (g->bomb_y > GAME_SCREEN_H - g->bomb->h) {
            g->bomb_active = 0;
            show_end_screen(g);
            return;
        }
    }

    /* ---- temporisation des effets œuf bleu / rouge ----------------- */
    if (g->egg_timer > 0) {
        g->egg_timer--;
        if (g->egg_timer == 0) {
            /* retour à la gravité et à la taille d’origine */
            g->gravity      = g->saved_gravity;
            g->player_scale = g->saved_player_scale;
            rebuild_player_sprites(g);
        }
    }


    /* ================================================================
       COLLISION avec le calque fixe
       ================================================================*/
    {
        int px  = g->player_x;
        int py  = g->player_y;
        int pw  = g->player->w;
        int ph  = g->player->h;

        /* --------- collision vers le BAS -------------------------------- */
        if (g->player_speed_y > 0)                       /* le perso descend     */
        {
            int future_y = py + g->player_speed_y + ph - 1;
            int touch = 0;

            for (int x = 0; x < pw; x += 2)              /* sonde tous les 2 px  */
                if (map_solid(g, px + x, future_y)) { touch = 1; break; }

            if (touch) {                                 /* on plaque le perso   */
                while (map_solid(g, px, py + ph - 1)) py--;
                g->player_y       = py;
                g->player_speed_y = 0;
            }
        }
        /* --------- collision vers le HAUT ------------------------------- */
        else if (g->player_speed_y < 0)                  /* le perso monte       */
        {
            int future_y = py + g->player_speed_y;       /* < py                 */
            int touch = 0;

            for (int x = 0; x < pw; x += 2)
                if (map_solid(g, px + x, future_y)) { touch = 1; break; }

            if (touch) {
                while (map_solid(g, px, py)) py++;
                g->player_y       = py;
                g->player_speed_y = 0;
            }
        }

        /* --------- MUR À DROITE : on empêche d’avancer dans le noir ------ */
        {
            int front_x = g->player_x + g->player->w;   /* colonne juste après le sprite */
            int solid   = 0;

            /* on scanne toute la hauteur utile du perso (un pas de 2 px suffit) */
            for (int y = 0; y < g->player->h; y += 2)
                if (map_solid(g, front_x, g->player_y + y)) { solid = 1; break; }

            if (solid)
            {
                /* le perso est bloqué : on le repousse vers la gauche
                   exactement de la valeur de scroll → il paraît immobile */
                g->player_x -= g->scrollspeed;

                /* s’il sort complètement de l’écran → game-over              */
                if (g->player_x + g->player->w < 0)
                {
                    show_end_screen(g);
                    return;
                }
            }
        }
    }
    /* ---- temporisation des effets œuf bleu / rouge ----------------- */
    if (g->egg_timer > 0) {
        g->egg_timer--;
        if (g->egg_timer == 0)              /* fin de l’effet             */
            g->gravity = g->saved_gravity;
    }
}



/* ---------------------------------------------------------------------------
   draw_game : utilise les backgrounds stockés dans g */
void draw_game(Game *g)
{
    /* ======== 1. FOND QUI DÉFILE ======== */
    BITMAP *bg = (g->selected_level == 2) ? g->background3
                 : (g->selected_level == 1 ? g->background2
                                           : g->background);

    masked_blit(bg, buffer, g->world_x, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);

    /* ======== 2. CALQUE DÉCOR QUI DÉFILE AUSSI ======== */
    BITMAP *overlay = (g->selected_level == 2) ? g->map_level3
                     : (g->selected_level == 1 ? g->map_level2
                                               : g->map_level1);

    if (overlay) {
        /* partie principale */
        int src_x = g->world_x % overlay->w;
        masked_blit(overlay, buffer, src_x, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);

        /* si on arrive au bord du bitmap, on recolle la partie gauche   */
        if (src_x > overlay->w - GAME_SCREEN_W) {
            int rest_w = GAME_SCREEN_W - (overlay->w - src_x);
            blit(overlay, buffer,            /* src bitmap   */
                 0, 0,                       /* src x,y      */
                 overlay->w - src_x, 0,      /* dst x,y      */
                 rest_w, GAME_SCREEN_H);     /* taille à copier */
        }
    }

    /* ========== 3. DRAPEAU ========================================== */
    if (g->selected_level >= 0) {
        int flag_x_world = (g->selected_level == 0) ? 6000 :
                           (g->selected_level == 1) ? 6050 : 6000;
        int flag_y = (g->selected_level == 1) ? 450 : 200;

        int sx = flag_x_world - g->world_x;
        if (sx + g->winflag->w >= 0 && sx < GAME_SCREEN_W)
            draw_sprite(buffer, g->winflag, sx, flag_y);
    }

    /* ========== 4. OBSTACLES ROTATIFS (niveaux 2 & 3) ================= */
    const int (*obstable)[2] = NULL;
    int nb_obs = 0;

    if      (g->selected_level == 1) { obstable = OBSTACLES_LVL2; nb_obs = 10; }
    else if (g->selected_level == 2) { obstable = OBSTACLES_LVL3; nb_obs = 12; }

    if (obstable) {
        static float angle = 0.0f;
        angle += 0.05f; if (angle > 2*M_PI) angle -= 2*M_PI;

        for (int i = 0; i < nb_obs; ++i) {
            int sx = obstable[i][0] - g->world_x;
            int sy = obstable[i][1];

            if (sx + g->obstacle->w >= 0 && sx < GAME_SCREEN_W)
                rotate_sprite(buffer, g->obstacle, sx, sy,
                              ftofix(angle * 128 / M_PI));
        }
    }



    /* ========== 5. ŒUFS & BOMBE ===================================== */
    if (g->selected_level == 1) {
        if (g->egg_blue_active)
            draw_sprite(buffer, g->egg_blue,
                        g->egg_blue_x - g->world_x, g->egg_blue_y);
        if (g->egg_red_active)
            draw_sprite(buffer, g->egg_red,
                        g->egg_red_x  - g->world_x, g->egg_red_y);
    }

    if (g->selected_level == 2) {
        if (g->egg_blue_active)
            draw_sprite(buffer, g->egg_blue,
                        g->egg_blue_x  - g->world_x, g->egg_blue_y);
        if (g->egg_green_active)
            draw_sprite(buffer, g->egg_green,
                        g->egg_green_x - g->world_x, g->egg_green_y);
        if (g->bomb_visible && g->bomb_active)
            draw_sprite(buffer, g->bomb,
                        g->bomb_x - g->world_x, g->bomb_y);
    }

    /* ========== 6. JOUEUR =========================================== */
    BITMAP *spr = g->player;
    if (key[KEY_SPACE]) {
        g->animation_frame++;
        spr = ((g->animation_frame / 5) % 2) ? g->player1 : g->player2;
    } else {
        g->animation_frame = 0;
    }
    masked_blit(spr, buffer, 0, 0,
                g->player_x, g->player_y, spr->w, spr->h);
}

/* ---------------------------------------------------------------------------
   Affichage du menu principal – version sans variable globale
--------------------------------------------------------------------------- */
void draw_menu(Game *g)
{
    /* --- fond du menu -------------------------------------------------- */
    draw_sprite(buffer, g->menu_background, 0, 0);

    /* --- logo BADLAND centré au-dessus du bouton ----------------------- */
    if (g->badland_logo)
    {
        int logo_x = (GAME_SCREEN_W - g->badland_logo->w) / 2;
        int logo_y = play_button_y - 20 - g->badland_logo->h;   /* 20 px au-dessus */
        draw_sprite(buffer, g->badland_logo, logo_x, logo_y);
    }

    /* --- gestion souris ------------------------------------------------ */
    poll_mouse();
    my_mouse_x = mouse_x;
    my_mouse_y = mouse_y;

    int over = (my_mouse_x >= play_button_x && my_mouse_x <= play_button_x + play_button_width &&
                my_mouse_y >= play_button_y && my_mouse_y <= play_button_y + play_button_height);

    BITMAP *btn = over ? g->play_button_hover : g->play_button;
    draw_sprite(buffer, btn, play_button_x, play_button_y);

    /* clic -> écran de sélection de niveau */
    if (over && (mouse_b & 1))
    {
        game_state = LEVEL_SELECTION;
        rest(200);                         /* anti-rebond clic */
    }

    /* --- petit texte d’aide sous le bouton ----------------------------- */
    const char *msg = "Ou appuyez sur ESPACE pour commencer";
    int tx = (GAME_SCREEN_W - text_length(font, msg)) / 2;
    textout_ex(buffer, font, msg, tx,
               play_button_y + play_button_height + 10,
               makecol(255,255,255), -1);

    /* raccourci clavier Espace */
    if (key[KEY_SPACE])
    {
        game_state = LEVEL_SELECTION;
        rest(200);
    }
}

void draw_level_selection(Game *g)
{
    draw_sprite(buffer, g->level_selection_background, 0, 0);

    textout_centre_ex(buffer, font, "Choisissez la difficulté",
                      GAME_SCREEN_W / 2, 100, makecol(255, 255, 255), -1);

    const char *levels[] = {"Facile", "Moyen", "Difficile"};
    int bw = 200, bh = 50, start_y = 200;

    poll_mouse();
    my_mouse_x = mouse_x;
    my_mouse_y = mouse_y;

    for (int i = 0; i < 3; i++) {
        int bx = (GAME_SCREEN_W - bw) / 2;
        int by = start_y + i * 100;

        int over = (my_mouse_x >= bx && my_mouse_x <= bx + bw &&
                    my_mouse_y >= by && my_mouse_y <= by + bh);

        rectfill(buffer, bx, by, bx + bw, by + bh,
                 over ? makecol(100,100,200) : makecol(70,70,150));

        textout_centre_ex(buffer, font, levels[i],
                          GAME_SCREEN_W / 2, by + 15,
                          makecol(255,255,255), -1);

        if (over && (mouse_b & 1)) {
            g->selected_level = i;
            game_state = PLAYING;
            rest(200);
        }
    }
}


void draw(Game *g)
{
    clear_bitmap(buffer);

    if      (game_state == MENU)            draw_menu(g);
    else if (game_state == LEVEL_SELECTION) draw_level_selection(g);
    else if (game_state == PLAYING)         draw_game(g);

    /* Musique d’ambiance menu / sélection */
    if (game_state == MENU || game_state == LEVEL_SELECTION)
        play_music(g, g->jungle_sound);

    blit(buffer, screen, 0, 0, 0, 0, GAME_SCREEN_W, GAME_SCREEN_H);
}


void play_music(Game *g, SAMPLE *track)
{
    if (g->current_music == track) return;      /* déjà en cours */

    if (g->current_music) stop_sample(g->current_music);
    g->current_music = track;

    if (g->current_music)
        play_sample(g->current_music, 255, 128, 1000, TRUE);   /* volume max, boucle */
}

void show_end_screen(Game *g)
{
    clear_keybuf();
    play_sample(g->gameover_sound, 255, 128, 1000, FALSE);

    int blink = 0;
    while (!key[KEY_ENTER]) {
        draw_sprite(buffer, g->end_screen, 0, 0);

        if (blink)
            textout_centre_ex(buffer, font,
                              "Restez appuyé sur ENTRER pour retourner au menu",
                              GAME_SCREEN_W/2, GAME_SCREEN_H - 50,
                              makecol(255,255,255), -1);

        blit(buffer, screen, 0,0, 0,0, GAME_SCREEN_W, GAME_SCREEN_H);
        blink = !blink;
        rest(500);
    }

    /* reset de base */
    game_state      = MENU;
    g->selected_level = -1;
    g->player_x     = 100;
    g->player_y     = 300;
    g->world_x      = 0;
    g->player_speed_y= 0;
    clear_keybuf();
}

void show_victory_screen(Game *g)
{
    clear_keybuf();
    play_sample(g->victoire_sound, 255, 128, 1000, FALSE);

    int blink = 0;
    while (!key[KEY_ENTER]) {
        draw_sprite(buffer, g->victory, 0, 0);

        if (blink)
            textout_centre_ex(buffer, font,
                              "Restez appuyé sur ENTRER pour retourner au menu",
                              GAME_SCREEN_W/2, GAME_SCREEN_H - 50,
                              makecol(255,255,255), -1);

        blit(buffer, screen, 0,0, 0,0, GAME_SCREEN_W, GAME_SCREEN_H);
        blink = !blink;
        rest(500);
    }

    game_state      = MENU;
    g->selected_level = -1;
    g->player_x     = 100;
    g->player_y     = 300;
    g->world_x      = 0;
    g->player_speed_y= 0;
    clear_keybuf();
}



/* --------------------------------------------------------------------------- */
int main(void) {
    Game game;
    game_init(&game);

    while (!key[KEY_ESC]) {
        update_physics(&game);
        draw(&game);
        rest(20);
    }

    game_deinit(&game);
    return 0;
}
END_OF_MAIN();
