#ifndef STAT_H
#define STAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define CHEMIN "stats.txt"

void lire_fichier(char **contenu_fichier);
void ecrire_fichier(char *texte);
void incrementer_stat(char *info, char *valeur);

#endif

