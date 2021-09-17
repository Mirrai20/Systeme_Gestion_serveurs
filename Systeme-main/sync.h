#ifndef SYNCH_H
#define SYNCH_H

#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "stat.h"

#define ERREUR 1
#define RECU 0
#define P sem_wait
#define V sem_post

sem_t sem1,sem2;

typedef struct {
	struct stat info; //structure permettant d'obtenir des informations sur les fichiers comme la date de modification
	char* nom; //chemin du dossier ou du fichier
}t_fichier;

/*FONCTION PRINCIPALE POUR SYNCHRONISER*/
void synch(char* diskProd, char* diskBackUp);

/*FONCTIONS QUI DONNENT LA LISTE DES DOSSIERS ET DES FICHIERS*/
void donner_listDossier(char *racine, t_fichier*** listDossier, int *i);
void donner_listFichier(char* racine, t_fichier** listDossier, t_fichier*** listFichier, int nbDoss, int *i);

/*FONCTION QUI SAUVEGARDE LES INFORMATIONS DES FICHIERS*/
void inserer_info(t_fichier*** listFichier, int tailleList);

/*FONCTION QUI RETIRE LA RACINE POUR ENVOYER A COPY POUR COPIER*/
void changer_racine(t_fichier*** listFichier, int tailleList, char* racine);

/*FONCTION QUI SYNCHRONISE*/
void donner_nouvListFichier(t_fichier*** listFichier1, t_fichier*** listFichier2, int taille1, int* taille2);

/*FONCTION QUI DÉTRUIT LES LISTE EN MEMOIRE*/
void desallouer_listFichier(t_fichier*** listFichier, int nbFichier);

#endif

