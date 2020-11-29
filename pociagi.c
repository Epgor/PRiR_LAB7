#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "/usr/lib/x86_64-linux-gnu/openmpi/include/mpi.h"

#define REZERWA 500
//#define LADUJ 1
//#define NIE_LADUJ 0
//definicja stanow samolotu
#define LOTNISKO 1
#define START 2
#define LOT 3
#define KONIEC_LOTU 4
#define KATASTROFA 5
#define TANKUJ 5000


int paliwo = 5000;
int LADUJ=1, NIE_LADUJ=0;
int liczba_procesow;
int nr_procesu;
int ilosc_samolotow;
int ilosc_pasow=4;
int ilosc_zajetych_pasow=0;
int tag=1;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;

void Wyslij(int nr_samolotu, int stan) 
//wyslij do lotniska swoj stan
{
    wyslij[0]=nr_samolotu;
    wyslij[1]=stan;
    MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}

void Lotnisko(int liczba_procesow)
{
    int nr_samolotu,status;
    ilosc_samolotow = liczba_procesow -1;
    printf("Halo, Witam serdecznie, tu wieża kontrolna lotniska \n");
    if(rand()%2==1){
        printf("Mamy piekna pogode sprzyjajaca podnniebnym podbojom\n");
    }
    else{
        printf("Niestety pogoda nie sprzyja dzisiejszym lotom\n");
    }
    printf("Zyczymy Panstwu, przyjemnej podrozy \n \n \n");
    printf("Dysponujemy %d pasami startowymi\n", ilosc_pasow);
    sleep(2);
    while(ilosc_pasow<=ilosc_samolotow){
        MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
        //odbieram od kogokolwiek
        nr_samolotu=odbierz[0];
        status=odbierz[1];
        if(status==1){
            printf("Samolot %d stoi na lotnisku, przynajmniej nie bedzie katastrofy\n", nr_samolotu);
        }
        if(status==2){
            printf("Samolot %d pozwolenie na start z pasu nr %d\n", nr_samolotu, ilosc_zajetych_pasow);
            ilosc_zajetych_pasow--;
        }
        if(status==3){
            printf("Samolot %d LATA\n", nr_samolotu);
        }
        if(status==4){
            if(ilosc_zajetych_pasow<ilosc_pasow){
                ilosc_zajetych_pasow++;
                MPI_Send(&LADUJ, 1, MPI_INT, nr_samolotu, tag, MPI_COMM_WORLD);
                }
                else{
                MPI_Send(&NIE_LADUJ, 1, MPI_INT, nr_samolotu, tag, MPI_COMM_WORLD);
                }
        }
        if(status==5){
            ilosc_samolotow--;
            printf("Ilosc samolotow %d\n", ilosc_samolotow);
            }
            }
            //zamykam while
            printf("Program zakonczyl dzialanie:)\n");
}


void Samolot(){
                int  stan,suma,i;
                stan=LOT;
                //to chyba jedyny rozsadny stan z jakiego warto startowac
                while(1){
                    if(stan==1){
                        if(rand()%2==1){
                            stan=START;
                            paliwo=TANKUJ;
                            printf("Prosze o pozwolenie na start, samolot %d\n",nr_procesu);
                            Wyslij(nr_procesu,stan);
                            }
                            else{
                                Wyslij(nr_procesu,stan);
                                }
                    }
                    else if(stan==2){
                        printf("Wystartowalem, samolot %d\n",nr_procesu);
                        stan=LOT;
                        Wyslij(nr_procesu,stan);
                    }
                    else if(stan==3){
                        paliwo-=rand()%500;
                        // spalilem troche paliwa
                        if(paliwo<=REZERWA){
                            stan=KONIEC_LOTU;
                            printf("prosze o pozwolenie na ladowanie\n");
                            Wyslij(nr_procesu,stan);
                        }
                        else{
                            for(i=0; rand()%10000;i++);
                        }
                    }
                    else if(stan==4){
                        int temp;
                        MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
                        if(temp==LADUJ){
                            stan=LOTNISKO;
                            printf("Wyladowalem, samolot %d\n", nr_procesu);
                        }
                        else{
                            paliwo-=rand()%500;
                            if(paliwo>0){
                                Wyslij(nr_procesu,stan);
                            }
                            else{
                                stan=KATASTROFA;
                                printf("rozbilem sie\n");
                                Wyslij(nr_procesu,stan);
                                return; 
                            }
                        }
                    }
                }
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
    MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
    srand(time(NULL));
    if(nr_procesu == 0) //Lotnisko
        Lotnisko(liczba_procesow);
    else //samoloty (petal odpowiada ze samoloty (czyli procesy inne niz 0))
        Samolot();
    MPI_Finalize();
    return 0;
} 
