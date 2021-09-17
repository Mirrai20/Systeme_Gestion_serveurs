#include "random.h"

/**
 * Donne une valeur aleatoire entre a et b compris.
 * @param a l'entier qui est la borne inférieure
 * @param b l'entier qui est la borne supérieure
 * @return l'entier tiré aléatoirement
 */
int donner_valeur_random(int a, int b)
{
	return rand()%(b+1 - a) + a;
}

