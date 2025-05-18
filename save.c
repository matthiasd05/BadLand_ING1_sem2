#include "save.h"
#include "resources.h"
#include <stdio.h>
#include <string.h>

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

void sauvegarder_niveau_pseudo(const char* pseudo, int niveau) {
    FILE *file = fopen(SAVE_FILE, "r");
    FILE *temp = fopen("temp_sauvegarde.txt", "w");
    int found = 0;
    char buffer_pseudo[50];
    int old_niveau;

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

    if (!found) {
        file = fopen(SAVE_FILE, "a");
        if (file) {
            fprintf(file, "%s %d\n", pseudo, niveau);
            fclose(file);
        }
    }
}
