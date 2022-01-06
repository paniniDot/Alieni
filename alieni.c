#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>
#include <pthread.h>

#include "DBGpthread.h"
#include "printerror.h"

#define ALIENI_INIZIALI 5
#define ALIENI_MIN_FUORI 3
#define ALIENI_MAX_IN_CASA 2
#define ALIENI_MIN_IN_CASA 1

/* mutex per coordinare l'ingresso in casa */
pthread_mutex_t mutexIngressoCasa;
/* mutex per decrementare gli alieni fuori casa quando entrano e concorrentemente crearne 2 nuovi all'occorrenza */
pthread_mutex_t mutexControlloAlieniFuori;


/* condition per bloccare l'ingresso se ci sono 2 alieni già dentro casa */
pthread_cond_t condAttesaIngresso;
/* condition per bloccare l'uscita se sono l'unico alieno o non è il mio turno */
pthread_cond_t condAttesaUscita;

intptr_t nAlieniCreati = 0;
int distributoreTicket = 0;
int turnoCorrente = 0;
int nAlieniFuori = ALIENI_INIZIALI;
int nAlieniInCasa = 0;



void *alieno( void *arg ) {

    char Alabel[128];
    int mioTurno, i, rc;
    pthread_t th;
    sprintf( Alabel, "Alieno %" PRIiPTR "", ( intptr_t )arg );

    DBGpthread_mutex_lock( &mutexIngressoCasa, Alabel );
    while( nAlieniInCasa >= ALIENI_MAX_IN_CASA ) {
        printf( "%s: Casa PIENA (%d), non posso entrare\n", Alabel, nAlieniInCasa );
        DBGpthread_cond_wait( &condAttesaIngresso, &mutexIngressoCasa, Alabel );
    }

    nAlieniInCasa++;
    DBGpthread_mutex_lock( &mutexControlloAlieniFuori, Alabel );
    nAlieniFuori--;
    DBGpthread_mutex_unlock( &mutexControlloAlieniFuori, Alabel );

    /* appena entro prendo il ticket per segnarmi in che posizione sono entrato */
    mioTurno = distributoreTicket++;
    printf( "%s: Casa NON PIENA (%d), entra e prende ticket %d\n", Alabel, nAlieniInCasa, mioTurno );

    /* entrato in casa, inizia a morire */
    printf( "%s: CODDIO sto a mori\' devo uscire\n", Alabel );

    if( nAlieniInCasa > ALIENI_MIN_IN_CASA ) {
        DBGpthread_cond_broadcast( &condAttesaIngresso, Alabel );
    }
    while( nAlieniInCasa == ALIENI_MIN_IN_CASA || mioTurno != turnoCorrente ) {
        printf( "%s: non posso uscire (alieni in casa = %d) (mio ticket = %d, ticket corrente = %d) \n", Alabel, nAlieniInCasa, mioTurno, turnoCorrente );
        DBGpthread_cond_wait( &condAttesaUscita, &mutexIngressoCasa, Alabel );
    }
    printf( "%s: posso uscire!!\n", Alabel );
    nAlieniInCasa--;
    turnoCorrente++;
    DBGpthread_cond_broadcast( &condAttesaIngresso, Alabel );
    DBGpthread_mutex_unlock( &mutexIngressoCasa, Alabel );

    DBGpthread_mutex_lock( &mutexControlloAlieniFuori, Alabel );
    if( nAlieniFuori < ALIENI_MIN_FUORI ) {
        printf( "%s: Alieni rimasti ad aspettare = %d, devo ricrearli\n", Alabel, nAlieniFuori );
        for( i=0; i<2; i++ ) {
            rc = pthread_create( &th, NULL, alieno, ( void *)nAlieniCreati++ );
            if( rc ) {
                PrintERROR_andExit( rc, "Creazione alieno" );
            }
        }
        nAlieniFuori+=2;
    }
    DBGpthread_mutex_unlock( &mutexControlloAlieniFuori, Alabel );
    pthread_exit( NULL );
}

int main( void ) {

    int rc;
    pthread_t th;
    intptr_t i;

    DBGpthread_mutex_init( &mutexIngressoCasa, NULL, "main" );
    DBGpthread_mutex_init( &mutexControlloAlieniFuori, NULL, "main" );
    DBGpthread_cond_init( &condAttesaIngresso, NULL, "main" );
    DBGpthread_cond_init( &condAttesaUscita, NULL, "main" );

    for( i=0; i<ALIENI_INIZIALI; i++ ) {
        rc = pthread_create( &th, NULL, alieno, ( void *)i );
        if( rc ) {
            PrintERROR_andExit( rc, "Creazione alieno" );
        }
    }

    pthread_exit( NULL );
    return 0;
}

