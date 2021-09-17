#include "log.h"

/**
 * Écriture d'un log selon le module.
 * @param module un char qui contiendra le module donné
 * @param message char possédant le contenu du message
 */
void ecrire_log(char* module, char* message) {
	char c = '\0';
	char date[18];
	FILE* fichier = fopen("logs.txt", "a");
	time_t date_nombre;
	struct tm* date_struct;

	if(fichier) {
		// strlen permet de vérifier la taille du message.
		if(strlen(message) > 100){ 
			c = message[100];
			message[100] = '\0';
		}

		time(&date_nombre);
		date_struct = localtime(&date_nombre);
		strftime(date, 18, "%d/%m/%y-%H:%M:%S", date_struct);
		fprintf(fichier, "[%s][%s] : %s\n", date, module, message);
		fclose(fichier);

		if(c != '\0'){ 
			message[100] = c;
			ecrire_log(module, message+100);
		}
	}
}

