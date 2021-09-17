#include "copy.h"

/**
 * Fonction principale de copie de fichiers.
 * Récupère les infos de la synchro puis copie les fichiers manquants + actualise les modifs.
 * @param disk1 Source de la copie
 * @param disk2 Destination de la copie
 */
void copier_disk(char* disk1, char* disk2, int tauxErreur) {
	int nbDSync = -1, nbDCopy = 0, nbFSync = -1, nbFCopy = 0;
	int nbFRecu = 0, nbDRecu = 0, nbErr = 0;

	t_fichier** listDSync = NULL;
	t_fichier** listFSync = NULL;

	mode_t droits = S_IRWXG | S_IRWXO | S_IRWXU;
	int tubeSC;
	int tubeCS;
	int reponse = ERREUR;

	srand(time(NULL));

	// TUBE ---------------------------------------
	if(mkfifo("TUBEcs", droits) == -1)
		system("rm -f TUBEcs");

	// Réception nombre dossiers
	P(&sem1);
	tubeSC = open("TUBEsc", O_RDONLY);
	if(read(tubeSC,&nbDSync,sizeof(int)) <= 0)
		system("rm -f TUBEsc");
	close(tubeSC);

	// Écriture accusé de réception
	if(nbDSync >= 0)
		reponse = RECU;
	else {
		reponse = ERREUR;
	}

	V(&sem2);
	tubeCS = open("TUBEcs", O_WRONLY);
	if(write(tubeCS, &reponse, sizeof(int)) == -1)
		system("rm -f TUBEcs");
	close(tubeCS);

	if(nbDSync > 0) {
		// Réception liste dossiers
		P(&sem1);
		listDSync = malloc(sizeof(t_fichier*) * nbDSync);
		tubeSC = open("TUBEsc", O_RDONLY);
		if(read(tubeSC,listDSync,sizeof(t_fichier*) * nbDSync) <= 0)
			system("rm -f TUBEsc");
		close(tubeSC);

		// Écriture accusé de réception
		if(listDSync != NULL)
			reponse = RECU;
		else {
			reponse = ERREUR;
		}

		V(&sem2);
		tubeCS = open("TUBEcs", O_WRONLY);
		if(write(tubeCS, &reponse, sizeof(int)) == -1)
			system("rm -f TUBEcs");
		close(tubeCS);
	}

	// Réception nombre fichiers
	P(&sem1);
	tubeSC = open("TUBEsc", O_RDONLY);
	if(read(tubeSC,&nbFSync,sizeof(int)) <= 0) {
		system("rm -f TUBEsc");
	}
	close(tubeSC);

	// Écriture accusé de réception
	if(nbFSync >= 0)
		reponse = RECU;
	else {
		reponse = ERREUR;
	}

	V(&sem2);
	tubeCS = open("TUBEcs", O_WRONLY);
	if(write(tubeCS, &reponse, sizeof(int)) == -1)
		system("rm -f TUBEcs");
	close(tubeCS);

	if(nbFSync > 0) {
		// Réception liste fichiers
		P(&sem1);
		listFSync = malloc(sizeof(t_fichier*) * nbFSync);
		tubeSC = open("TUBEsc", O_RDONLY);
		if(read(tubeSC,listFSync,sizeof(t_fichier*) * nbFSync) <= 0)
			system("rm -f TUBEsc");
		close(tubeSC);

		// Écriture accusé de réception
		if(listFSync != NULL)
			reponse = RECU;
		else {
			reponse = ERREUR;
		}

		V(&sem2);
		tubeCS = open("TUBEcs", O_WRONLY);
		if(write(tubeCS, &reponse, sizeof(int)) == -1)
			system("rm -f TUBEcs");
		close(tubeCS);

		P(&sem1);
	}

	// Suppression tube
	system("rm -f TUBEcs");
	// --------------------------------------------
	t_fichier** listDCopy = NULL;
	t_fichier** listFCopy = NULL;

	// On récupère la liste des dossiers et fichiers de la destination
	donner_listDossier(disk2,&listDCopy,&nbDCopy);
	donner_listFichier(disk2,listDCopy,&listFCopy,nbDCopy,&nbFCopy);

	// On enlève la racine des chemins
	changer_racine(&listDCopy,nbDCopy,disk2);
	changer_racine(&listFCopy,nbFCopy,disk2);

	// On rajoute les dossiers et fichiers manquants selon la liste récupérée
	donner_nouvListFichier(&listDSync,&listDCopy,nbDSync,&nbDCopy);
	donner_nouvListFichier(&listFSync,&listFCopy,nbFSync,&nbFCopy);

	// On remet la racine dans les chemins
	rajouter_racine(&listDSync,nbDSync,disk1);
	rajouter_racine(&listDCopy,nbDCopy,disk2);
	rajouter_racine(&listFSync,nbFSync,disk1);
	rajouter_racine(&listFCopy,nbFCopy,disk2);

	// On met les infos des éléments
	inserer_info(&listDCopy,nbDCopy);
	inserer_info(&listFCopy,nbFCopy);
	inserer_info(&listDSync,nbDSync);
	inserer_info(&listFSync,nbFSync);

	// On effectue la copie
	copier_dossiers(listDCopy,nbDCopy,&nbDRecu,&nbErr, tauxErreur);
	copier_fichiers(listFSync,listFCopy,nbFCopy,&nbFRecu,&nbErr, tauxErreur);
	// On remet les bonnes dates
	inserer_info(&listDCopy,nbDCopy);
	copier_dates(listDSync,listDCopy,nbDCopy);

	// Écriture des stats et logs
	char* statsTxt[3] = {"Nombre de dossiers copiés","Nombre de fichiers copiés","Nombre d'erreurs"};
	char* logsTxt[3] = {" dossier(s) copié(s)", " fichier(s) copié(s)", " erreur(s) de copie"};
	int statsNum[3] = {nbDRecu,nbFRecu,nbErr};

	for(int i = 0; i < 3; i++) {
		int taille = snprintf(NULL, 0, "%d", statsNum[i]);
		char *chaine = malloc(taille + 1);
		snprintf(chaine, taille + 1, "%d", statsNum[i]);
		char* msg = malloc(sizeof(char) * (strlen(chaine) + strlen(logsTxt[i]) + 1));
		sprintf(msg,"%s%s",chaine,logsTxt[i]);
		if(statsNum[i] > 0)
			incrementer_stat(statsTxt[i],chaine);
		ecrire_log("copy",msg);
		free(chaine);
		free(msg);
	}

	// Désallocation des listes
	desallouer_listFichier(&listDCopy,nbDCopy);
	desallouer_listFichier(&listDSync,nbDSync);
	desallouer_listFichier(&listFCopy,nbFCopy);
	desallouer_listFichier(&listFSync,nbFSync);
}

/**
 * @param listD Liste des dossiers
 * @param nbDos Taille de la liste
 * @param nbDRecu Nombre de dossiers copiés
 * @param nbErr Nombre d'erreurs lors de la copie
 */
void copier_dossiers(t_fichier** listD, int nbDos, int* nbDRecu, int* nbErr, int tauxErreur) {
	int i;
	DIR* d = NULL;

	for(i = 0; i < nbDos; i++) {
		//On vérifie si le dossier n'existe pas dans le dossier de destination
		if(!(d = opendir(listD[i]->nom))) {
			if(donner_valeur_random(0,tauxErreur)) {
				if(executer_cmd(CMD_CREATE_D,listD[i]->nom,"") == 0) // On crée le dossier
					*nbDRecu += 1;
				else
					*nbErr += 1;
			}
			else
				*nbErr += 1;
		}
		else
			closedir(d);
	}
}

/**
 * @param listSrc Liste des fichiers du serveur source
 * @param listDest Liste des fichiers du serveur destination
 * @param nbFich Taille des listes
 * @param nbFrecu Nombre de fichiers copiés
 * @param nbErr Nombre d'erreurs lors de la copie
 */
void copier_fichiers(t_fichier** listSrc, t_fichier** listDest, int nbFich, int* nbFRecu, int* nbErr, int tauxErreur) {
	int i;

	for(i = 0; i < nbFich; i++) {
		// On vérifie que le fichier n'existe pas ou qu'il n'est pas à jour
		if(access(listDest[i]->nom,F_OK) || listDest[i]->info.st_mtime != listSrc[i]->info.st_mtime) {
			if(donner_valeur_random(0,tauxErreur)) {
				if(executer_cmd(CMD_COPY_F,listSrc[i]->nom,listDest[i]->nom) == 0)
					*nbFRecu += 1;
				else
					*nbErr += 1;
			}
			else
				*nbErr += 1;
		}
	}
}

/**
 * @param listSrc Liste dont on doit copier les dates
 * @param listDest Liste où on va remplacer les dates
 * @param tailleListe Taille des listes
 */ 
void copier_dates(t_fichier** listSrc, t_fichier** listDest, int tailleListe) {
	int i;

	for(i = 0; i < tailleListe; i++) {
		// On vérifie que le dossier n'a pas la même date que dans la source
		if(listDest[i]->info.st_mtime != listSrc[i]->info.st_mtime) {
			executer_cmd(CMD_DATE,listSrc[i]->nom,listDest[i]->nom);
		}
	}
}

/**
 * @param liste Liste où on veut rajouter la racine à tous les chemins
 * @param taille Taille de la liste
 * @param racine Nom de la racine
 */ 
void rajouter_racine(t_fichier*** liste, int taille, char* racine) {
	int i;

	for(i = 0; i < taille; i++) {
		char* nom = malloc(sizeof(char) * (strlen(racine) + strlen((*liste)[i]->nom) + 2));
		sprintf(nom,"%s/%s",racine,(*liste)[i]->nom);

		(*liste)[i]->nom = realloc((*liste)[i]->nom,sizeof(char) * (strlen(nom) + 1));
		strcpy((*liste)[i]->nom,nom);

		free(nom);
	}
}

/**
 * @param chemin Chemin où l'on doit trouver le dossier parent
 * @return Chemin du dossier parent
 */ 
char* donner_dossier_parent(char* chemin) {
	int i;
	int pos;
	char* nom = NULL;

	// On recherche la position du dernier slash
	for(i = strlen(chemin); i >= 0; i--) {
		if(chemin[i] == '/') {
			pos = i;
			break;
		}
	}

	nom = malloc(sizeof(char) * (pos + 1));

	// On copie la chaine de caractère jusqu'à ce slash
	for(i = 0; i < pos; i++)
		nom[i] = chemin[i];
	nom[i] = '\0';

	return nom;
}

/**
 * @param nomCmd Command à exécuter
 * @param arg1 Argument 1 de la commande
 * @param arg2 Argument 2 de la commande
 * @return Si la commande a réussi ou non
 */ 
int executer_cmd(char* nomCmd, char* arg1, char* arg2) {
	char* cmd = NULL;
	char* dosParent = NULL;
	int numErr;
	DIR* d = NULL;

	// On prend le chemin du dossier parent du dossier/fichier à créer/actualiser
	if(strcmp(nomCmd,CMD_CREATE_D) == 0)
		dosParent = donner_dossier_parent(arg1);
	else
		dosParent = donner_dossier_parent(arg2);

	// On vérifie que le dossier existe
	if((d = opendir(dosParent))) {
		cmd = malloc(sizeof(char) * (strlen(nomCmd) + strlen(arg1) + strlen(arg2) + 2));
		sprintf(cmd,"%s%s %s",nomCmd,arg1,arg2);

		numErr = system(cmd);

		free(cmd);
		closedir(d);
	}
	else
		numErr = -1;

	free(dosParent);

	return numErr;
}

