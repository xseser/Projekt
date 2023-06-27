#include "main.h"
#include "watek_glowny.h"

void mainLoopGnome(int gnomes, int dwarves, int sights, int pins)
{
    srandom(rank);
    int tag;
    int perc;
    println("Jestem nowym gnomem\n");
    bool ubieganie_agrafka;
    bool ubieganie_celownik;


    while (stan != InFinish) {
	    switch (stan) {
	        case InRun: 
		    {
		        perc = random()%100;
		        if ( perc < 25 ) {
			        perc = random()%100;
			        if ( ma_agrafke || perc < 50) {
				        println("Rozpoczynam przetwarzanie");
				        println("Ubiegam się o sekcję krytyczną związaną z celownikiem");
				        myRequestClock = vClock;
				        packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
				        pkt->who = GNOME;
				        pkt->what = SIGHT;
				        ackDwarves = 0;
				        ackGnomes = 0;
		                println("Zeruje liczbe zgod");
                        zgodyGnom.clear();
                        println("Czyszcze tablice zgod");
                        ubieganie_celownik = TRUE;
				        changeState( InWantSight );
				        for (int i=0;i<=size-1;i++)
				        if (i!=rank)
					        sendPacket( pkt, i, REQUEST);
				        free(pkt);
			        }
			        else if(ma_celownik || perc >=50) {
				        println("Ubiegam się o sekcję krytyczną związaną z agrafką");
				        myRequestClock = vClock;
				        packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
				        pkt->who = GNOME;
				        pkt->what = PIN;
				        ackDwarves = 0;
				        ackGnomes = 0;
				        println("Zeruje liczbe zgod");
                        zgodyGnom.clear();
                        println("Czyszcze tablice zgod");
                        ubieganie_agrafka = TRUE;
				        changeState( InWantPin );
				        for (int i=0;i<=size-1;i++)
				        if (i!=rank)
					        sendPacket( pkt, i, REQUEST);
				        free(pkt);
			        }
		        }
		        break;
		    }
	        case InWantPin:
		    {
			    println("Zbieram zgody na wejście do sekcji krytycznej związanej z agrafkami");
			    while ( ackDwarves + ackGnomes < dwarves + gnomes - pins);
			    println("Uzbieralem wystarczajaco zgod");
			    changeState(InSection);
			    break;
		    }
		    case InWantSight:
		    {
			    println("Zbieram zgody na wejście do sekcji krytycznej związanej z celownikami");
			    while ( ackDwarves + ackGnomes < dwarves + gnomes - sights);
			    println("Uzbieralem wystarczajaco zgod");
			    changeState(InSection);
			    break;
		    }
	        case InSection:
		    {
			    println("Jestem w sekcji krytycznej")

			    if(!ma_agrafke && ubieganie_agrafka) {
				    ma_agrafke = TRUE;
				    println("Otrzymałem agrafkę                                     === A ===");
				    ubieganie_agrafka = FALSE;
			    }
			    else if(!ma_celownik && ubieganie_celownik) {
				    ma_celownik = TRUE;
				    println("Otrzymałem celownik                                    === C ===");
				    ubieganie_celownik = FALSE;
			    }
			    sleep(2);
			    
			    if(ma_agrafke && ma_celownik){
				    ma_agrafke = FALSE;
                	ma_celownik = FALSE;
				    ma_bron = TRUE;
			        println("Utworzylem bron                                        === B ===");
			        
			        packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
			        pkt->who = GNOME;
                    while(ma_bron) {
                        for (int pid : oczekujace_bron) {
                            sendPacket( pkt, pid, ACK );
                            println("Wysylam ACK do oczekujacego %d", pid);
                            ma_bron = FALSE;
                        }
                        oczekujace_bron.clear();
                    }
                     
		            for (int i=0;i<=size-1;i++)
		                if (i!=rank)
			                sendPacket( pkt, i, RELEASE); 
		            free(pkt);
			    }
			    
			    println("Wychodzę z sekcji krytycznej");
			    changeState( InRun );
			    
                
			    break;
		    }
	        default: 
		        {break;}
        }
        sleep(SEC_IN_STATE);
    }
}

void mainLoopDwarf(int gnomes, int dwarves)
{
    srandom(rank);
    int tag;
    int perc;
    println("Jestem nowym skrzatem\n");
    
    while (stan != InFinish) {
	switch (stan) {
	    case InRun: 
		{
			perc = random()%100;
			if ( perc < 25 ) {
				println("Rozpoczynam przetwarzanie");
				println("Ubiegam się o sekcję krytyczną związaną z brońmi")
				packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
				pkt->who = DWARF;
				pkt->what = WEAPON;
				ackDwarves = 0;
				ackGnomes = 0;
				for (int i=0;i<=size-1;i++)
				if (i!=rank)
					sendPacket( pkt, i, REQUEST);
				changeState( InWantWeapon );
				free(pkt);
			}
			break;
		}
	    case InWantWeapon:
		{
			println("Zbieram zgody na wejście do sekcji krytycznej związanej z brońmi");
			while ( ackGnomes < dwarves - ackDwarves );
			println("Zgody od gnomow (%d) musza byc rowne co najmniej liczbie skrzatow (%d) - zgody od skrzatow (%d)", ackGnomes, dwarves, ackDwarves);
			println("Uzbieralem wystarczajaco zgod");
			changeState( InSection );
			break;
		}
	    case InSection:
		{
			println("Jestem w sekcji krytycznej");
			println("Zabijam szczura                                        === SZ ===");
			sleep(3);
			println("Wychodzę z sekcji krytycznej związanej z brońmi");
			
			packet_t *pkt = (packet_t *) malloc(sizeof(packet_t));
			pkt->who = DWARF;
			for (int i=0;i<=size-1;i++)
			    if (i!=rank)
				    sendPacket( pkt, i, RELEASE); 
			free(pkt);
	
			changeState( InRun );
			break;
		}
	    default: 
		{break;}
        }
        sleep(SEC_IN_STATE);
    }
}
