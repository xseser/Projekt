#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  
    int who;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
    int what;
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 4

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK     1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH  5

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum {InRun, InMonitor, InWantSight, InWantPin, InWantWeapon, InSection, InFinish} state_t;
extern state_t stan;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t vClockMut;
/* zmiana stanu, obwarowana muteksem */
void changeState( state_t );
#endif