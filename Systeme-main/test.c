#include "test.h"

/**
 * Indique si le serveur de production est disponible en testant la valeur d'un booléen.
 */
void tester_server() {
	if(!est_bloque){
		ecrire_log("test", "Serveur de production disponible");
		printf("\nTest server => Le serveur de production est disponible.\n");
	}
	else{
		ecrire_log("test", "Serveur de production indisponible, redirection vers serveur backup");
		incrementer_stat("Serveur de production défaillant", "1");
		printf("\nTest server => Le serveur backup est disponible.\n");
	}
}

