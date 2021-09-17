#!/bin/bash

function afficher_aide() {
	echo "./main.sh"
	echo "./main.sh TAUXERREUR1 TAUXERREUR2"
	echo "./main.sh [OPTION]"
	echo ""
	echo "TAUXERREUR1   : probabilité de panne du serveur de production = 1 / TAUXERREUR1"
	echo "TAUXERREUR2   : probabilité d'échec de copie = 1 / TAUXERREUR2"
	echo ""
	echo "Une seule option à la fois"
	echo "-h | --help   : affiche l'aide"
	echo "-l | --logs   : affiche les logs"
	echo "-L | --razl   : supprime les logs"
	echo "-r | --raz    : supprime les logs et les stats"
	echo "-R | --razall : supprime les logs, les stats, les dossiers serveur, le fichier compilé et valgrind.txt"
	echo "-s | --stats  : affiche les stats"
	echo "-S | --razs   : supprime les stats"
	echo "-v | --val    : appelle valgrind, écrit valgrind.txt et l'affiche"
}

function afficher_infos() {
	case $1 in
		"-h" | "--help")
			afficher_aide
			;;
		"-l" | "--logs")
			if test -r logs.txt
			then
				cat logs.txt
			else
				echo "Le fichier de logs ne peut pas être lu."
			fi
			;;
		"-L" | "--razl")
			supprimer_fichier logs.txt
			;;
		"-r" | "--raz")
			supprimer_fichier logs.txt
			supprimer_fichier stats.txt
			;;
		"-R" | "--razall")
			supprimer_fichier logs.txt
			supprimer_fichier stats.txt
			supprimer_fichier diskProd
			supprimer_fichier diskServeur
			supprimer_fichier main
			supprimer_fichier valgrind.txt
			;;
		"-s" | "--stats")
			if test -r stats.txt
			then
				cat stats.txt
			else
				echo "Le fichier de stats ne peut pas être lu."
			fi
			;;
		"-S" | "--razs")
			supprimer_fichier stats.txt
			;;
		"-v" | "--val")
			lancer_programme_valgrind
			;;
		*)
			echo "Argument invalide"
			afficher_aide
			;;
	esac
}

function ctrl_c() {
	rm -f TUBE*
	echo ""
}

function preparer_dossiers() {
	rm -f -r diskProd diskServeur
	rm -f main
	mkdir -p diskProd diskServeur

	trap ctrl_c INT
}

function lancer_programme() {
	preparer_dossiers
	gcc -Wall -Wextra -Werror copy.c log.c main.c random.c stat.c sync.c test.c -lpthread -o main && ./main "$1" "$2"
}

function lancer_programme_valgrind() {
	preparer_dossiers
	gcc -Wall -Wextra -Werror copy.c log.c main.c random.c stat.c sync.c test.c -lpthread -ggdb3 -o main && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind.txt ./main
	cat valgrind.txt
}

function supprimer_fichier() {
	if test -n "$1"
	then
		rm "$1" -r 2> /dev/null
		if test $(echo "$?") == "0"
		then
			echo "$1 supprimé"
		else
			echo "$1 n'existe pas"
		fi
	fi
}

case $# in
	0)
		lancer_programme
		;;
	1)
		afficher_infos "$1"
		;;
	2)
		lancer_programme "$1" "$2"
		;;
	*)
		afficher_aide
		;;
esac

