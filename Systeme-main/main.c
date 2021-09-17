#include "main.h"

pthread_t integ, backup, prod, update;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Appelle le module sync.
 */
void* synchroniser() {
	if(est_bloque) {
		ecrire_log("sync", "Debut de la synchronisation de diskBackUp");
	}
	else {
		ecrire_log("sync", "Debut de la synchronisation de diskProd");
	}
	synch("diskProd", "diskServeur");
	ecrire_log("sync", "Fin de synchronisation");
	return NULL;
}

/**
 * Appelle le module copy.
 */
void* copier(void* arg) {
	int tauxErrorCopy = *((int*) arg);
	if(est_bloque) {
		ecrire_log("copy", "Debut de la copie de diskBackUp");
	}
	else {
		ecrire_log("copy", "Debut de la copie de diskProd");
	}
	copier_disk("diskProd","diskServeur", tauxErrorCopy);
	ecrire_log("copy", "Fin de copie");
	return NULL;
}

/**
 * Mets à jour les fichiers du nouveau serveur toutes les 10 secondes.
 * @param arg pointeur sur le taux d'erreurs de copie
 */
void *main_update_server(void *arg) {
	usleep(donner_valeur_random(10, 1000));
	pthread_t thread1,thread2;
	int tauxErrorCopy = *((int*) arg);
	while(!a_fini) {
		sem_init(&sem1,0,0);
		sem_init(&sem2,0,0);

		pthread_create(&thread1, NULL, synchroniser, NULL);
		pthread_create(&thread2, NULL, copier, (void*) &tauxErrorCopy);

		pthread_join(thread1,NULL);
		pthread_join(thread2,NULL);

		sem_destroy(&sem1);
		sem_destroy(&sem2);

		for(int i = 0; i < 10; i++) {
			if(a_fini)
				break;
			sleep(1);
		}
	}
	return NULL;
}

/**
 * Simule l'activité du serveur de backup.
 * @param arg pointeur sur le taux d'indisponibilité du serveur de production
 */
void *main_server_backup(void *arg) {
	usleep(donner_valeur_random(10, 1000));
	int tauxCrashServ = *((int*) arg);
	while (!a_fini) {
		sleep(1);
		if(!donner_valeur_random(0,tauxCrashServ)) {
			est_bloque = false;
			V_mutex(&mutex);
		}
	}
	return NULL;
}

/**
 * Simule l'activité du serveur de production.
 * @param arg pointeur sur le taux d'indisponibilité du serveur de production
 */
void *main_server_prod(void *arg) {
	usleep(donner_valeur_random(10, 1000));
	int tauxCrashServ = *((int*) arg);
	while (!a_fini) {
		sleep(1);
		if(!donner_valeur_random(0,tauxCrashServ)) {
			est_bloque = true;
			P_mutex(&mutex);
		}
	}
	return NULL;
}

/**
 * Demande si l'utilisateur souhaite appeler le module test.
 */
void *main_nouveau_server() {
	usleep(donner_valeur_random(10, 1000));
	char r = 'r';

	printf("Initialisation du serveur nouveau serveur....  \n");
	sleep(1);
	printf("\t********* Bienvenue sur le serveur d'integration. *********\n");

	while(r != 'q') {
		do {
			printf("\n+ Vous voulez connaître le serveur courant ? (o/n) (q pour quitter) : ");
			scanf( " %c", &r );
		} while ( r!='O' && r!='N' && r!='o' && r!='n'&& r != 'q');

		if (r=='O' || r=='o'){
			tester_server();
		}
	}
	return NULL;
}

/**
 * Main, crée les threads et attend leur fin.
 * Attend la fin du thread du nouveau serveur.
 * Change un booléen qui indique la fin du programme aux autres threads.
 * Attend la fin des autres threads.
 * @param argc nombre d'arguments (1 ou 3)
 * @param argv arguments donnés (aucun ou les deux taux d'erreur)
 */
int main(int argc, char* argv[]) {
	int tauxCrashServ = INT_MAX, tauxErrorCopy = INT_MAX;

	est_bloque = false;
	srand(time(NULL));
	if(argc == 3) {
		tauxCrashServ = (int) strtol(argv[1], NULL, 10);
		tauxErrorCopy = (int) strtol(argv[2], NULL, 10);
		if(tauxCrashServ == 0) {
			tauxCrashServ = INT_MAX;
		}
		if(tauxErrorCopy == 0) {
			tauxErrorCopy = INT_MAX;
		}
	}

	// Threads serveurs
	pthread_create(&integ, NULL, main_nouveau_server, NULL);
	pthread_create(&backup, NULL, main_server_backup, (void*) &tauxCrashServ);
	pthread_create(&prod, NULL, main_server_prod, (void*) &tauxCrashServ);

	// Thread d'update pour synchroniser et copier dans diskServeur
	pthread_create(&update, NULL, main_update_server, (void*) &tauxErrorCopy);

	a_fini = false;
	pthread_join(integ, NULL);
	a_fini = true;
	printf("Fermeture en cours...");
	fflush(stdout);

	pthread_join(backup, NULL);
	pthread_join(prod, NULL);
	pthread_join(update, NULL);
	printf(" fermé\n");

	return 0;
}

