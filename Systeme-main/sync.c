#include "sync.h"

void synch(char* diskProd, char* diskBackUp) {
	t_fichier** listDossProd = NULL;
	t_fichier** listFichierProd = NULL;
	t_fichier** listDossBackUp = NULL;
	t_fichier** listFichierBackUp = NULL;
	int nbDossProd = 0;
	int nbFichierProd = 0;
	int nbDossBackUp = 0;
	int nbFichierBackUp = 0;

	/*diskProd*/
	// On récupère les chemins
	donner_listDossier(diskProd, &listDossProd, &nbDossProd);
	donner_listFichier(diskProd, listDossProd, &listFichierProd, nbDossProd, &nbFichierProd);
	// On insère les infos (date de modification, etc)
	inserer_info(&listDossProd, nbDossProd);
	inserer_info(&listFichierProd, nbFichierProd);

	/*diskBackUp*/
	// On récupère les chemins
	donner_listDossier(diskBackUp, &listDossBackUp, &nbDossBackUp);
	donner_listFichier(diskBackUp, listDossBackUp, &listFichierBackUp, nbDossBackUp, &nbFichierBackUp);
	// On insère les infos (date de modification, etc)
	inserer_info(&listDossBackUp, nbDossBackUp);
	inserer_info(&listFichierBackUp, nbFichierBackUp);
	// On retire disckProd/ et diskBackUp/ dans les nom des fichiers
	changer_racine(&listDossProd, nbDossProd, diskProd);
	changer_racine(&listFichierProd, nbFichierProd, diskProd);
	changer_racine(&listDossBackUp, nbDossBackUp, diskBackUp);
	changer_racine(&listFichierBackUp, nbFichierBackUp, diskBackUp);

	// On fait la synchronisation
	donner_nouvListFichier(&listDossProd, &listDossBackUp, nbDossProd, &nbDossBackUp);
	donner_nouvListFichier(&listFichierProd, &listFichierBackUp, nbFichierProd, &nbFichierBackUp);
	// On ajoute dans les log et les stats le nombre de dossiers synchronisés
	int taille = snprintf(NULL, 0, "%d", nbDossBackUp);
	char* tailleChar = malloc(taille + 1);
	snprintf(tailleChar, taille+1, "%d", nbDossBackUp);
	incrementer_stat("Nombre de dossiers synchronisés", tailleChar);
	char* messageLog = NULL;
	messageLog = malloc(sizeof(char)*(26 + strlen(tailleChar) + 11 + 1));
	snprintf(messageLog, (26 + strlen(tailleChar) + 11 + 1), "Synchronisation reussie : %s dossier(s)", tailleChar);
	free(tailleChar);
	ecrire_log("sync", messageLog);
	free(messageLog);
	// On ajoute dans les log et les stats le nombre de fichiers synchronisés
	taille = snprintf(NULL, 0, "%d", nbFichierBackUp);
	tailleChar = malloc(taille + 1);
	snprintf(tailleChar, taille+1, "%d", nbFichierBackUp);
	incrementer_stat("Nombre de fichiers synchronisés", tailleChar);
	messageLog = malloc(sizeof(char)*(26 + strlen(tailleChar) + 11 + 1));
	snprintf(messageLog, (26 + strlen(tailleChar) + 11 + 1), "Synchronisation reussie : %s fichier(s)", tailleChar);
	free(tailleChar);
	ecrire_log("sync", messageLog);
	free(messageLog);

	// On libère les liste de dossier/fichier
	desallouer_listFichier(&listDossProd, nbDossProd);
	desallouer_listFichier(&listFichierProd, nbFichierProd);

	// On envoie à copy la liste des dossiers et des fichiers synchronisés
	mode_t droits = S_IRWXG | S_IRWXO | S_IRWXU;
	int tubeSC;
	int tubeCS;
	int reception = ERREUR;

	if(mkfifo("TUBEsc", droits) == -1) {
		system("rm -f TUBEsc");
	}

	// Écriture nbDossier
	V(&sem1);
	tubeSC = open("TUBEsc", O_WRONLY);
	if(write(tubeSC, &nbDossBackUp, sizeof(int)) == -1) {
		system("rm -f TUBEsc");
	}
	close(tubeSC);

	P(&sem2);
	// Accusé de reception
	tubeCS = open("TUBEcs", O_RDONLY);
	if(read(tubeCS, &reception, sizeof(int)) <= 0) {
		system("rm -f TUBEcs");
	}
	close(tubeCS);

	V(&sem1);

	// On envoie la liste des dossiers seulement s'il y a des dossiers
	if(nbDossBackUp > 0) {
		if(reception == RECU) {
			tubeSC = open("TUBEsc", O_WRONLY);
			if(write(tubeSC, listDossBackUp, sizeof(t_fichier*)*nbDossBackUp) == -1) {
				system("rm -f TUBEsc");
			}
			close(tubeSC);
		}
		free(listDossBackUp);

		P(&sem2);
		// Accusé de reception
		tubeCS = open("TUBEcs", O_RDONLY);
		if(read(tubeCS, &reception, sizeof(int)) <= 0) {
			system("rm -f TUBEcs");
		}
		close(tubeCS);

		V(&sem1);
	}

	// Écriture nbFichier
	if(reception == RECU) {
		tubeSC = open("TUBEsc", O_WRONLY);
		if(write(tubeSC, &nbFichierBackUp, sizeof(int)) == -1) {
			system("rm -f TUBEsc");
		}
		close(tubeSC);
	}

	P(&sem2);
	// Accusé de reception
	tubeCS = open("TUBEcs", O_RDONLY);
	if(read(tubeCS, &reception, sizeof(int)) <= 0) {
		system("rm -f TUBEcs");
	}
	close(tubeCS);

	V(&sem1);

	// On envoie la liste des fichier seulement s'il y en a
	if(nbFichierBackUp > 0) {
		if(reception == RECU) {
			tubeSC = open("TUBEsc", O_WRONLY);
			if(write(tubeSC, listFichierBackUp, sizeof(t_fichier*)*nbFichierBackUp) == -1) {
				system("rm -f TUBEsc");
			}
			close(tubeSC);
		}
		free(listFichierBackUp);

		P(&sem2);
		// Accusé de reception
		tubeCS = open("TUBEcs", O_RDONLY);
		if(read(tubeCS, &reception, sizeof(int)) <= 0) {
			system("rm -f TUBEcs");
		}

		V(&sem1);
	}
	system("rm -f TUBEsc");
}

void donner_listDossier(char *racine, t_fichier*** listDossier, int* i) {
	struct dirent *dir;
	DIR *d = opendir(racine);
	int j = 0;

	if(d) {
		while((dir = readdir(d)) != NULL) {    	// On parcours les fichiers
			if(dir->d_type == DT_DIR		         	// On ne prend que des dossiers
					&& strcmp(".", dir->d_name) != 0	// On ne prend pas en compte les dossiers . et ..
					&& strcmp("..", dir->d_name) != 0) {
				char* nom = NULL;
				*listDossier = (t_fichier**) realloc(*listDossier, sizeof(t_fichier*)*(*i+1));
				(*listDossier)[*i] = (t_fichier*) malloc(sizeof(t_fichier));

				// Concaténation du nom de dossier
				nom = (char*) malloc(sizeof(char)*(strlen(racine) + strlen(dir->d_name) + 2));
				sprintf(nom,"%s/%s",racine,dir->d_name);

				(*listDossier)[*i]->nom = (char*) malloc(sizeof(char) * (strlen(nom)+1));
				(*listDossier)[*i]->nom = strcpy((*listDossier)[*i]->nom, nom);

				// On liste les dossier dans le dossier (récursivité)
				j = *i;
				*i = *i + 1;
				free(nom);

				// On explore les autres dossiers de ce dossier
				donner_listDossier((*listDossier)[j]->nom, listDossier, i);
			}
		}
		closedir(d);
	}
}

void inserer_info(t_fichier*** listFichier, int tailleList) {
	// On enregistre les info du dossier/fichier (date de modification, etc)
	for(int i = 0; i < tailleList; i++)
		stat((*listFichier)[i]->nom, &(*listFichier)[i]->info);
}

void donner_listFichier(char* racine, t_fichier** listDossier, t_fichier*** listFichier, int nbDoss, int *i) {
	struct dirent *dir;
	DIR *d = opendir(racine);

	if(d) {
		while((dir = readdir(d)) != NULL)	{ // On parcourt les fichiers dans la racine du serveur
			if(dir->d_type != DT_DIR) {
				char* nom = NULL;
				*listFichier = (t_fichier**) realloc(*listFichier, sizeof(t_fichier*)*(*i+1));
				(*listFichier)[*i] = (t_fichier*) malloc(sizeof(t_fichier));

				// Concaténation du nom du fichier
				nom = malloc(sizeof nom * (strlen(racine + strlen(dir->d_name) + 2)));
				sprintf(nom, "%s/%s", racine, dir->d_name);

				(*listFichier)[*i]->nom = (char*) malloc(sizeof(char) * (strlen(nom)+1));
				(*listFichier)[*i]->nom = strcpy((*listFichier)[*i]->nom, nom);

				*i = *i + 1;
				free(nom);
			}
		}
		closedir(d);
	}

	for(int j = 0; j < nbDoss; j++) {
		d = opendir(listDossier[j]->nom);

		if(d) {
			while((dir = readdir(d)) != NULL)	{ // On parcourt les fichiers dans les dossiers du serveur
				if(dir->d_type != DT_DIR) {
					char* nom = NULL;
					*listFichier = (t_fichier**) realloc(*listFichier, sizeof(t_fichier*)*(*i+1));
					(*listFichier)[*i] = (t_fichier*) malloc(sizeof(t_fichier));

					// Concaténation du nom du fichier
					nom = malloc(sizeof nom * (strlen(listDossier[j]->nom + strlen(dir->d_name) + 2)));
					sprintf(nom, "%s/%s", listDossier[j]->nom, dir->d_name);

					(*listFichier)[*i]->nom = (char*) malloc(sizeof(char) * (strlen(nom)+1));
					(*listFichier)[*i]->nom = strcpy((*listFichier)[*i]->nom, nom);

					*i = *i + 1;
					free(nom);
				}
			}
			closedir(d);
		}
	}
}

void changer_racine(t_fichier*** listFichier, int tailleList, char* racine) {
	for(int i = 0; i < tailleList; i++) {
		char* nom = NULL;
		nom = (char*) malloc(sizeof(char)*(strlen((*listFichier)[i]->nom)+1));
		nom = strcpy(nom, (*listFichier)[i]->nom);

		int tailleNouvNom = strlen(nom) - strlen(racine);

		(*listFichier)[i]->nom = realloc((*listFichier)[i]->nom,sizeof(char)* (tailleNouvNom + 1));

		for(int j = 0; j < (int) strlen(nom); j++) { // Lorsqu'on arrive au premier "/"
			if(nom[j] == '/') { //On ajoute le nom sans la racine
				(*listFichier)[i]->nom = strcpy((*listFichier)[i]->nom, nom+j+1);
				break;
			}
		}
		free(nom);
	}
}

void donner_nouvListFichier(t_fichier*** listFichier1, t_fichier*** listFichier2, int taille1, int* taille2) {
	bool estDedans = false;
	int tailleSupp = 0;

	for(int i = 0; i < taille1; i++) { // Pour chaque dossier/fichier dans listFichier1
		estDedans = false;

		for(int j = 0; j < *taille2; j++) { // On regarde s'il se trouve dans listFichier2
			if(strcmp((*listFichier1)[i]->nom, (*listFichier2)[j]->nom) == 0) {
				estDedans = true;
				break;
			}
		}

		if(!estDedans) { // S'il n'est pas dedans on l'ajoute
			tailleSupp++;

			// On ajoute une case dans la liste
			*listFichier2 = (t_fichier**) realloc(*listFichier2, sizeof(t_fichier*)*(*taille2 + tailleSupp));
			(*listFichier2)[*taille2+tailleSupp-1] = (t_fichier*) malloc(sizeof(t_fichier));

			// On insère le nom et les infos
			(*listFichier2)[*taille2+tailleSupp-1]->nom = (char*) malloc(sizeof(char) * (strlen((*listFichier1)[i]->nom)+1));
			(*listFichier2)[*taille2+tailleSupp-1]->nom = strcpy((*listFichier2)[*taille2+tailleSupp-1]->nom, (*listFichier1)[i]->nom);

			(*listFichier2)[*taille2+tailleSupp-1]->info = (*listFichier1)[i]->info;
		}
	}
	*taille2 = *taille2 + tailleSupp;
}

void desallouer_listFichier(t_fichier*** listFichier, int nbFichier) {
	for(int i = 0; i < nbFichier; i++) {
		free((*listFichier)[i]->nom);
		free((*listFichier)[i]);
	}
	free(*listFichier);
}

