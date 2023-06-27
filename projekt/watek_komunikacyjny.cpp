#include "main.h"
#include "watek_komunikacyjny.h"
#include <algorithm>

void *startComGnome(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
	auto index = zgodyGnom.end();
	bool receivedReleaseFromDwarf = FALSE;
	
	packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
	pkt->who = GNOME;
    
    while ( stan != InFinish ) {
	    
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        switch ( status.MPI_TAG ) {
	        case REQUEST:
	        {
                if(pakiet.what == PIN) {
                    if(stan == InWantPin && pakiet.ts > myRequestClock){
                        if(pakiet.who == DWARF){
                            if(std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src) == zgodyGnom.end()) {
                                ackDwarves++;
                                zgodyGnom.push_back(pakiet.src);
                                debug("Dostałem REQ, ktory traktuje jak ACK od %d, mam już %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                            }
                        }
                        if(pakiet.who == GNOME){
                            if(std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src) == zgodyGnom.end()) {
                                ackGnomes++;
                                zgodyGnom.push_back(pakiet.src);
                                debug("Dostałem REQ, ktory traktuje jak ACK od %d, mam już %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                            }
                        }
                    }
                    println("Czy blokuje agrafke? %s", ma_agrafke ? "tak" : "nie");
                    if(stan != InWantPin && !ma_agrafke && stan != InSection){
                        sendPacket( pkt, status.MPI_SOURCE, ACK );
                    }
                }
                if(pakiet.what == SIGHT){
                    if(stan == InWantSight && pakiet.ts > myRequestClock){
                        if(pakiet.who == DWARF){
                            if(std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src) == zgodyGnom.end()) {
                                ackDwarves++;
                                zgodyGnom.push_back(pakiet.src);
                                debug("Dostałem REQ, ktory traktuje jak ACK od %d, mam już %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                            }
                        }
                        if(pakiet.who == GNOME){
                            if(std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src) == zgodyGnom.end()) {
                                ackGnomes++;
                                zgodyGnom.push_back(pakiet.src);
                                debug("Dostałem REQ, ktory traktuje jak ACK od %d, mam już %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                            }
                        }
                    }
                    println("Czy blokuje celownik? %s", ma_celownik ? "tak" : "nie");
                    if(stan != InWantSight && !ma_celownik && stan != InSection){
                        sendPacket( pkt, status.MPI_SOURCE, ACK );
                    }
                }
                if(pakiet.what == WEAPON){
                    println("Czy mam bron? %s", ma_bron ? "tak" : "nie");
                    if(ma_bron){
                        sendPacket( pkt, status.MPI_SOURCE, ACK );
                        ma_bron = FALSE;
                    }
                    else {
                        oczekujace_bron.push_back(pakiet.src);
                        println("Dodaje %d do oczekujacych na bron", pakiet.src);
                    }
                }
                break;
	        }
	        case ACK:
	        {
	            if(pakiet.who == DWARF){
	                if(std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src) == zgodyGnom.end()) {
                        ackDwarves++;
                        zgodyGnom.push_back(pakiet.src);
                        debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                    }
                }
                else if(pakiet.who == GNOME){
                    if(std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src) == zgodyGnom.end()) {
                        ackGnomes++;
                        zgodyGnom.push_back(pakiet.src);
                        debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                    }
                }
                else debug("Nie udalo sie odebrac ACK");
	            break;
	        }
	        case RELEASE:
	        {
	            if( pakiet.who == DWARF ) {
	                index = std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src);
                    if (index != zgodyGnom.end()) {
                        zgodyGnom.erase(index);
                        ackDwarves--;
                        println("Usuwam ACK od skrzata %d, mam teraz %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                    }
                    receivedReleaseFromDwarf = TRUE;
                } else if ( pakiet.who == GNOME && receivedReleaseFromDwarf) {
                    if(std::find(zgodyGnom.begin(), zgodyGnom.end(), pakiet.src) == zgodyGnom.end()) {
                        ackGnomes++;
                        zgodyGnom.push_back(pakiet.src);
                        debug("Dostałem RELEASE, ktory traktuje jak ACK od %d, mam już %d", status.MPI_SOURCE, ackDwarves+ackGnomes);
                    }
                    receivedReleaseFromDwarf = FALSE;
                }
                break;
	        }
	        default:
	            break;
        }
        pthread_mutex_lock(&vClockMut);
        if (vClock < pakiet.ts){vClock = pakiet.ts;}
        vClock++;
        pthread_mutex_unlock( &vClockMut );
    }
}

void *startComDwarf(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
	std::vector<int> oczekujace_agrafki;
	std::vector<int> oczekujace_celowniki;
	std::vector<int> zgody;
	
	packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
	pkt->who = DWARF;
	
    while ( stan != InFinish ) {
    
        if( stan == InRun ) {
            for (int pid : oczekujace_agrafki) {
                sendPacket( pkt, pid, ACK );
                println("Wysylam ACK do oczekujacego %d", pid);
            }
            for (int pid : oczekujace_celowniki) {
                sendPacket( pkt, pid, ACK );
                println("Wysylam ACK do oczekujacego %d", pid);
            }
            zgody.clear();
            oczekujace_agrafki.clear();
            oczekujace_celowniki.clear();
        }
    
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
        switch ( status.MPI_TAG ) {
	        case REQUEST:
	            {
                if(pakiet.what == PIN){
                    if(stan == InWantWeapon){
                        sendPacket( pkt, status.MPI_SOURCE, ACK );
                    }else{
                        oczekujace_agrafki.push_back(pakiet.src);
                    }
                }
                if(pakiet.what == SIGHT){
                    if(stan == InWantWeapon){
                        sendPacket( pkt, status.MPI_SOURCE, ACK );
                    }else{
                        oczekujace_celowniki.push_back(pakiet.src);
                    }
                }
                if(pakiet.what == WEAPON){
                    if(stan != InWantWeapon && stan != InSection){
                        sendPacket( pkt, status.MPI_SOURCE, ACK );
                    }
                }
	            break;
	        }
	        case ACK: 
	        {
	            if(pakiet.who == DWARF) {
	                if(std::find(zgody.begin(), zgody.end(), pakiet.src) == zgody.end()) {
                        ackDwarves++;
                        zgody.push_back(pakiet.src);
                        debug("Dostałem ACK od %d, mam już %d od skrzatow i %d od gnomow", status.MPI_SOURCE, ackDwarves, ackGnomes);
                    }
                }
                else if(pakiet.who == GNOME){
                    if(std::find(zgody.begin(), zgody.end(), pakiet.src) == zgody.end()) {
                        ackGnomes++;
                        zgody.push_back(pakiet.src);
                        debug("Dostałem ACK od %d, mam już %d od skrzatow i %d od gnomow", status.MPI_SOURCE, ackDwarves, ackGnomes);
                    }
                }
                else debug("Nie udalo sie odebrac ACK");
	            break;
	        }
	        default:
	            break;
        }
        pthread_mutex_lock(&vClockMut);
        if (vClock < pakiet.ts){vClock = pakiet.ts;}
        vClock++;
        pthread_mutex_unlock( &vClockMut );
    }
}
