#ifndef COPY_H
#define COPY_H

#include "random.h"
#include "sync.h"

#define CMD_COPY_F "cp --preserve=all "
#define CMD_CREATE_D "mkdir "
#define CMD_DATE "touch --reference="

void copier_disk(char* disk1, char* disk2, int tauxErreur);
void copier_dossiers(t_fichier** listD, int nbDos, int* nbDRecu, int* nbErr, int tauxErreur);
void copier_fichiers(t_fichier** listSrc, t_fichier** listDest, int nbFich, int* nbFRecu, int* nbErr, int tauxErreur);
void copier_dates(t_fichier** listSrc, t_fichier** listDest, int tailleListe);

void rajouter_racine(t_fichier*** liste, int taille, char* racine);
char* donner_dossier_parent(char* chemin);
int executer_cmd(char* nomCmd, char* arg1, char* arg2);

#endif

