#include "stat.h"

/**
 * Lit un fichier et récupère son contenu.
 * @param contenu_fichier un pointeur sur char qui contiendra le texte
 */
void lire_fichier(char **contenu_fichier) {
	long taille;
	FILE* fichier = fopen(CHEMIN, "rb");

	if(fichier) {
		fseek(fichier, 0, SEEK_END);
		taille = ftell(fichier);
		fseek(fichier, 0, SEEK_SET);
		*contenu_fichier = malloc(sizeof **contenu_fichier * (taille + 1));
		if(*contenu_fichier) {
			fread(*contenu_fichier, 1, taille, fichier);
		}
		fclose(fichier);
		(*contenu_fichier)[taille] = '\0';
	}
}

/**
 * Écrit dans un fichier.
 * @param texte la chaîne de caractères à écrire
 */
void ecrire_fichier(char *texte) {
	FILE *fichier = fopen(CHEMIN, "w");

	if(fichier) {
		fputs(texte, fichier);
		ecrire_log("stat", "Statistiques mises à jour");
		fclose(fichier);
	}
}

/**
 * Incrémente une statistique ou la met à 0.
 * @param info la chaîne de caractères contenant l'information concernée
 * @param valeur la chaîne de caractères contenant la valeur à ajouter (si 0, alors remise à 0)
 */
void incrementer_stat(char *info, char *valeur) {
	char *a_ecrire = NULL, *contenu_fichier = NULL, *curseur = NULL, *info2 = NULL, *val_apres = NULL;
	char val_avant[12];
	int taille = 0;
	long int ancienne_valeur = 0, a_ajouter = 0;

	lire_fichier(&contenu_fichier);

	// Le fichier existe et n'est pas vide.
	if(contenu_fichier && strlen(contenu_fichier)) {
		// 3 char supplémentaires ><\0
		info2 = malloc(sizeof *info2 * (strlen(info) + 3));
		sprintf(info2, ">%s<", info);

		// curseur pointe sur le début de la ligne où se trouve l'info si elle a été trouvée.
		curseur = strstr(contenu_fichier, info2);

		// L´information n´existe pas encore dans le fichier.
		if(curseur == NULL) {
			// Il suffit d'ajouter l'info à la fin du fichier.
			a_ecrire = malloc(sizeof *a_ecrire * (strlen(contenu_fichier) + strlen(info2) + strlen(valeur) + 2));
			if(a_ecrire) {
				sprintf(a_ecrire, "%s%s%s\n", contenu_fichier, info2, valeur);
				ecrire_fichier(a_ecrire);
				free(a_ecrire);
			}
		}
		else {
			// Il est nécessaire d'altérer la ligne où se situe l'info.
			sscanf(curseur + strlen(info2), "%s", val_avant);
			ancienne_valeur = strtol(val_avant, NULL, 10);
			a_ajouter = strtol(valeur, NULL, 10);
			// Si la valeur à ajouter vaut 0, il faut mettre à 0 l'information.
			ancienne_valeur = (a_ajouter != 0 ? ancienne_valeur + a_ajouter : 0);
			taille = snprintf(NULL, 0, "%ld", ancienne_valeur);
			val_apres = malloc(taille + 1);
			snprintf(val_apres, taille + 1, "%ld", ancienne_valeur);
			// Permet de retirer la ligne à modifier et les suivantes dans contenu_fichier.
			curseur[0] = 0;
			a_ecrire = malloc(sizeof *a_ecrire * (strlen(contenu_fichier) + strlen(info2) + strlen(val_apres) + 1) + strlen(curseur+strlen(info2)+strlen(val_avant)+1) + 1);
			if(a_ecrire) {
				sprintf(a_ecrire, "%s%s%s\n%s", contenu_fichier, info2, val_apres, curseur+strlen(info2)+strlen(val_avant)+1);
				ecrire_fichier(a_ecrire);
				free(a_ecrire);
			}
			free(val_apres);
		}
		free(info2);
	}
	else {
		// Il suffit de créer le fichier et d'écrire l'information avec sa valeur.
		// 4 char supplémentaires ><\n\0
		a_ecrire = malloc(sizeof *a_ecrire * (strlen(info) + 4 + strlen(valeur)));
		if(a_ecrire) {
			sprintf(a_ecrire, ">%s<%s\n", info, valeur);
			ecrire_fichier(a_ecrire);
			free(a_ecrire);
		}
	}
	free(contenu_fichier);
}

