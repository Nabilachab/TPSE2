#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NB_BUS_X 5
#define NB_BUS_Y 4
#define NB_TRAJETS 10
#define MAX_TURN 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int nb_XY = 0, nb_YX = 0;
int turn_XY = 0, turn_YX = 0;
int waiting_XY = 0, waiting_YX = 0;

void sleep_random() {
    usleep((rand() % 501 + 1000) * 1000); // entre 1s et 1.5s
}

void entrer_tunnel(int direction) {
    int *waiting, *nb, *turn, *turn_opp, *nb_opposé;
    int max_attempts = 100;
    int attempts = 0;

    pthread_mutex_lock(&mutex);

    if (direction == 0) {
        waiting = &waiting_XY;
        nb = &nb_XY;
        turn = &turn_XY;
        nb_opposé = &nb_YX;
        turn_opp = &turn_YX;
    } else {
        waiting = &waiting_YX;
        nb = &nb_YX;
        turn = &turn_YX;
        nb_opposé = &nb_XY;
        turn_opp = &turn_XY;
    }

    (*waiting)++;

    while ((*nb_opposé > 0 || *turn >= MAX_TURN) && attempts < max_attempts) {
        pthread_mutex_unlock(&mutex);
        usleep(100000); // attendre 100ms
        attempts++;
        pthread_mutex_lock(&mutex);
    }

    if (attempts >= max_attempts) {
        (*waiting)--;
        pthread_mutex_unlock(&mutex);
        printf("Bus en direction %s abandonne après trop d’attente.\n", direction == 0 ? "X->Y" : "Y->X");
        return;
    }

    (*waiting)--;
    (*nb)++;
    (*turn)++;
    *turn_opp = 0;

    pthread_mutex_unlock(&mutex);
}

void sortir_tunnel(int direction) {
    pthread_mutex_lock(&mutex);
    if (direction == 0)
        nb_XY--;
    else
        nb_YX--;
    pthread_mutex_unlock(&mutex);
}

void* bus_thread(void* arg) {
    int* infos = (int*)arg;
    int id = infos[0];
    int ville_depart = infos[1];
    char* noms_villes[2] = {"X", "Y"};

    for (int i = 1; i <= NB_TRAJETS; i++) {
        // Aller
        entrer_tunnel(ville_depart);
        printf("Bus %d de %s : %s -> %s (Trajet %d)\n", id, noms_villes[ville_depart], noms_villes[ville_depart], noms_villes[1 - ville_depart], i);
        sleep_random();
        sortir_tunnel(ville_depart);

        // Retour
        entrer_tunnel(1 - ville_depart);
        printf("Bus %d de %s : %s -> %s (Trajet %d)\n", id, noms_villes[1 - ville_depart], noms_villes[1 - ville_depart], noms_villes[ville_depart], i);
        sleep_random();
        sortir_tunnel(1 - ville_depart);
    }

    return NULL;
}

int main() {
    pthread_t threads[NB_BUS_X + NB_BUS_Y];
    int infos[NB_BUS_X + NB_BUS_Y][2];

    srand(time(NULL));

    for (int i = 0; i < NB_BUS_X; i++) {
        infos[i][0] = i + 1;
        infos[i][1] = 0;
        pthread_create(&threads[i], NULL, bus_thread, infos[i]);
    }

    for (int i = 0; i < NB_BUS_Y; i++) {
        infos[NB_BUS_X + i][0] = NB_BUS_X + i + 1;
        infos[NB_BUS_X + i][1] = 1;
        pthread_create(&threads[NB_BUS_X + i], NULL, bus_thread, infos[NB_BUS_X + i]);
    }

    for (int i = 0; i < NB_BUS_X + NB_BUS_Y; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
