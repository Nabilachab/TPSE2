/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>


#define NB_BUS_X 5
#define NB_BUS_Y 4
#define NB_TRAJETS 10
#define MAX_TURN 3

// Déclaration du mutex pour les sections critiques
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int nb_XY = 0, nb_YX = 0; // Nombre de bus actuellement dans le tunnel dans chaque sens
int turn_XY = 0, turn_YX = 0; // Nombre de bus consécutifs ayant accédé au tunnel
int waiting_XY = 0, waiting_YX = 0; // Nombre de bus en attente

// Fonction qui simule un temps de trajet aléatoire entre 1 et 1.5 secondes
void sleep_random() {
    usleep((rand() % 501 + 1000) * 1000); // usleep prend des microsecondes
}

// Fonction pour demander l'accès au tunnel
void entrer_tunnel(int direction) {
    // Déclaration de pointeurs vers les variables associées à la direction
    int opposé, *waiting, *nb, *turn, *turn_opp;

    pthread_mutex_lock(&mutex); // Entrée en section critique

    if (direction == 0) { // Direction X->Y
        waiting = &waiting_XY;
        nb = &nb_XY;
        turn = &turn_XY;
        opposé = nb_YX;
        turn_opp = &turn_YX;
    } else { // Direction Y->X
        waiting = &waiting_YX;
        nb = &nb_YX;
        turn = &turn_YX;
        opposé = nb_XY;
        turn_opp = &turn_XY;
    }

    (*waiting)++; // Le bus signale qu'il attend

    // Tant qu'il y a des bus dans le sens opposé ou que la limite de tour est atteinte
    while (opposé > 0 || (*turn) >= MAX_TURN) {
        pthread_mutex_unlock(&mutex); // Sortie temporaire de la section critique
        usleep(100000); // Attente active (100ms)
        pthread_mutex_lock(&mutex); // Rentrer pour re-vérifier
        if (direction == 0) opposé = nb_YX;
        else opposé = nb_XY;
    }

    (*waiting)--; // Le bus ne fait plus partie des attentes
    (*nb)++;      // Le bus entre dans le tunnel
    (*turn)++;    // Incrément du compteur de tour pour son sens
    *turn_opp = 0; // Réinitialisation du tour de l'autre sens

    pthread_mutex_unlock(&mutex); // Sortie de la section critique
}

// Fonction pour signaler la sortie du tunnel
void sortir_tunnel(int direction) {
    pthread_mutex_lock(&mutex); // Entrée en section critique

    if (direction == 0) nb_XY--; // Sortie dans le sens X->Y
    else nb_YX--;               // Sortie dans le sens Y->X

    pthread_mutex_unlock(&mutex); // Sortie de la section critique
}

// Fonction principale exécutée par chaque thread bus
void* bus_thread(void* arg) {
    int id = ((int*)arg)[0]; // ID du bus
    int ville_depart = ((int*)arg)[1]; // Ville de départ : 0 = X, 1 = Y
    char* noms_villes[2] = {"X", "Y"};

    for (int i = 1; i <= NB_TRAJETS; i++) {
        // Trajet aller
        entrer_tunnel(ville_depart);
        printf("Bus %d de %s : %s -> %s (Trajet %d)\n", id, noms_villes[ville_depart], noms_villes[ville_depart], noms_villes[1 - ville_depart], i);
        sleep_random();
        sortir_tunnel(ville_depart);

        // Trajet retour
        entrer_tunnel(1 - ville_depart);
        printf("Bus %d de %s : %s -> %s (Trajet %d)\n", id, noms_villes[1 - ville_depart], noms_villes[1 - ville_depart], noms_villes[ville_depart], i);
        sleep_random();
        sortir_tunnel(1 - ville_depart);
    }

    return NULL;
}

// Fonction principale du programme
int main() {
    pthread_t threads[NB_BUS_X + NB_BUS_Y]; // Tableau des threads
    int infos[NB_BUS_X + NB_BUS_Y][2];      // Infos : ID et ville de départ


    srand(time(NULL)); // Initialisation du générateur aléatoire

    // Création des threads pour les bus de la ville X
    for (int i = 0; i < NB_BUS_X; i++) {
        infos[i][0] = i + 1; // ID
        infos[i][1] = 0;     // Ville X
        pthread_create(&threads[i], NULL, bus_thread, infos[i]);
    }

    // Création des threads pour les bus de la ville Y
    for (int i = 0; i < NB_BUS_Y; i++) {
        infos[NB_BUS_X + i][0] = NB_BUS_X + i + 1; // ID
        infos[NB_BUS_X + i][1] = 1;               // Ville Y
        pthread_create(&threads[NB_BUS_X + i], NULL, bus_thread, infos[NB_BUS_X + i]);
    }

    // Attente de la fin de tous les threads
    for (int i = 0; i < NB_BUS_X + NB_BUS_Y; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

