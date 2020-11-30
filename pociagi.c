#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "/usr/lib/x86_64-linux-gnu/openmpi/include/mpi.h"

#define REZERWA 500

#define STACJA 1
#define START 2
#define TRASA 3
#define KONIEC_TRASY 4
#define KATASTROFA 5
#define SERWIS 6

#define TANKUJ 5000


int paliwo = 5000;
int LADUJ=1, NIE_LADUJ=0;
int liczba_procesow;
int nr_procesu;
int ilosc_pociagow;
int ilosc_torow=4;
int ilosc_zajetych_torow=0;
int tag=1;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;

void Wyslij(int nr_pociagu, int stan) 
//wyslij do lotniska swoj stan
{
    wyslij[0]=nr_pociagu;
    wyslij[1]=stan;
    MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}

void Stacja(int liczba_procesow)
{
    int nr_pociagu,status;
    ilosc_pociagow = liczba_procesow -1;
    printf("Halo, Witam serdecznie, tu stacja kolejowa \n");
    if(rand()%2==1){
        printf("Mamy piekna pogode sprzyjajaca trasom\n");
    }
    else{
        printf("Niestety pogoda nie sprzyja trasom\n");
    }
    printf("Zyczymy Panstwu, przyjemnej podrozy \n \n \n");
    printf("Dysponujemy %d torami\n", ilosc_torow);
    sleep(2);
    while(ilosc_torow<=ilosc_pociagow){
        MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
        //odbieram od kogokolwiek
        nr_pociagu=odbierz[0];
        status=odbierz[1];
        if(status==1){
            printf("Pociag %d stoi na peronie \n", nr_pociagu);
        }
        if(status==2){
            printf("Pociag %d pozwolenie na start z toru nr %d\n", nr_pociagu, ilosc_zajetych_torow);
            ilosc_zajetych_torow--;
        }
        if(status==3){
            printf("Pociag %d w trasie\n", nr_pociagu);
        }
        if(status==4){
            if(ilosc_zajetych_torow<ilosc_torow){
                ilosc_zajetych_torow++;
                MPI_Send(&LADUJ, 1, MPI_INT, nr_pociagu, tag, MPI_COMM_WORLD);
                }
                else{
                MPI_Send(&NIE_LADUJ, 1, MPI_INT, nr_pociagu, tag, MPI_COMM_WORLD);
                }
        }
        if(status==5){
            ilosc_pociagow--;
            printf("Ilosc pociagow %d\n", ilosc_pociagow);
        }
        if(status==6){
            ilosc_pociagow++;
            printf("Ilosc pociagow %d\n", ilosc_pociagow);
        }
    }
            //zamykam while
            printf("Program zakonczyl dzialanie\n");
}


void Pociag(){
                int  stan,suma,i;
                stan=TRASA;
                while(1){
                    if(stan==1){
                        if(rand()%2==1){
                            stan=START;
                            paliwo=TANKUJ;
                            printf("Prosze o pozwolenie na start, Pociag %d\n",nr_procesu);
                            Wyslij(nr_procesu,stan);
                            }
                            else{
                                Wyslij(nr_procesu,stan);
                                }
                    }
                    else if(stan==2){
                        printf("Wystartowalem, Pociag %d\n",nr_procesu);
                        stan=TRASA;
                        Wyslij(nr_procesu,stan);
                    }
                    else if(stan==3){
                        paliwo-=rand()%500;
                        // spalilem troche paliwa
                        if(paliwo<=REZERWA){
                            stan=KONIEC_TRASY;
                            printf("prosze o pozwolenie na wjazd na stacje\n");
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
                            stan=STACJA;
                            printf("Wyladowalem, Pociag %d\n", nr_procesu);
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
                    else if(stan==5){
                        //naprawianie pociagów
                            stan=SERWIS;
                            printf(" Pociag %d, w serwisie\n", nr_procesu);

                            paliwo=TANKUJ;

                        }
                    
                    else if(stan==6){

                            stan=TRASA;
                            printf("Powrót na trasę, Pociag %d\n", nr_procesu);

                            Wyslij(nr_procesu,stan);

                        }
                    }
                
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
    MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
    srand(time(NULL));
    if(nr_procesu == 0) //STACJA
        Stacja(liczba_procesow);
    else //pociagi (petal odpowiada ze pociagi (czyli procesy inne niz 0))
        Pociag();
    MPI_Finalize();
    return 0;
} 
