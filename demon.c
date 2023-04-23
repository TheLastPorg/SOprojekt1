#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

//Funkcja sortowania przez wstawianie

void Sortowanie(int zadania[][4], int n, char komendy[][100]) {
    int i, j, k, pom[4];
    char pom2[100][100];
    for (i = 0; i < n; i++) {
        j = i - 1;
        // kopiowanie i-tego wiersza do zmiennej pom
        for (k = 0; k < 4; k++) {
            pom[k] = zadania[i][k];
        }
        strcpy(pom2[i],komendy[i]);
        // przesuwanie wiekszych elementów w prawo
        while (j >= 0 && (zadania[j][0] > pom[0] || (zadania[j][0] == pom[0] && zadania[j][1] > pom[1]))) {
            for (k = 0; k < 4; k++) {
                zadania[j + 1][k] = zadania[j][k];
            }
            strcpy(komendy[j + 1], komendy[j]);
            j--;
        }
        // wstawienie i-tego elementu w odpowiednie miejsce
        for (k = 0; k < 4; k++) {
            zadania[j + 1][k] = pom[k];
        }
         strcpy(komendy[j + 1], pom2[i]);
    }
}


int main(int argc, char *argv[]) {
    pid_t pid, sid;

	printf("Program odpalil sie");
	
    // Pobieramy taskfile i outfile
    if(argc != 3) {
        printf("Za duzo lub za malo wprowadzonych danych!\n");
        return 1;
    }

    char *taskfile = argv[1];
    char *outfile = argv[2];


    //otwieranie pliku taskfile i outfile
    FILE *zadania = fopen(taskfile, "r");
    FILE *wypisanie = fopen(outfile, "w");

    if(zadania == NULL)
    {
        printf("Nie mozna otworzyc pliku taskfile!\n");
        return 1;
    }

    if(wypisanie == NULL)
    {
        printf("Nie mozna otworzyc pliku outfile!\n");
        return 1;
    }


    //Wczytywanie danych z pliku taskfile
    char linia[100]; //bufor
    char *wartosc; //pomocniczo zapisujemy poszczegolne wartosci
    int ilosc_zadan = 0; //pomocniczo liczymy ilosc zadan
    int zadania_tab[1000][4]; //tablica z oddzielonymi wartosciami
    char komendy[100][100]; //tablica char do przechowywania komend

    ////// linia=bufor, 100 = maksymalna dlugosc
    while (fgets(linia, 100, zadania) && ilosc_zadan < 1000)
    {
        wartosc = strtok(linia, ":");
        zadania_tab[ilosc_zadan][0] = atoi(wartosc); //atoi zamienia char na int
        wartosc = strtok(NULL, ":");
        zadania_tab[ilosc_zadan][1] = atoi(wartosc);
        wartosc = strtok(NULL, ":");
        zadania_tab[ilosc_zadan][2] = atoi(wartosc);
        strcpy(komendy[ilosc_zadan], wartosc); //zapisywanie napisu jako char w tablicy komendy, bo wczesniej zamienia sie w int i jest to bez sensu
        wartosc = strtok(NULL, ":");
        zadania_tab[ilosc_zadan][3] = atoi(wartosc);
        ilosc_zadan++;
    }



    //Sortowanie chronologiczne instrukcji

    Sortowanie(zadania_tab, ilosc_zadan, komendy);

    //ta sekcja tylko po to zeby zobaczyc jak dziala, potem sie wywali
    printf("posortowane \n");
    for(int i=0; i<ilosc_zadan; i++)
    {
        printf("komenda %s wykona sie o %d:%d z parametrem %d \n", komendy[i], zadania_tab[i][0], zadania_tab[i][1], zadania_tab[i][3]);
    }

    //Ustawienie usługi syganłu SIGINT (chyba)

    //signal(SIGINT, sigint_wywolanie); //Do SIGINT przypisujemy wywołanie funkcji sigint_wywolanie
    //^ nie kompiluje sie

    //Dalej

    pid = fork(); //Fork- funkcja uruchamia nowy proces potomny
    //ten pid = fork nie może być na tym etapie w forze, bo tworzyło się 100 procesów potomnych na raz

    int kod_wyjscia = 0;

    for(;;)
    {

	    int zadanie = 0;
	    int pom = 0;

	    

	    if (pid < 0) {
		printf("Nie udalo sie utworzyc glownego procesu.\n");
		exit(EXIT_FAILURE);
	    }
	    if (pid >= 0) {
	    
	    
	    	//Musimy sprawdzać czy już jest czas i wtedy lecimy do ifów
	    	
	    	for(;;)
	    	{
		    	time_t now = time(NULL); //pobieramy czas z systemu
		    	
		    	struct tm *local = localtime(&now);
		    	int godzina = local->tm_hour;
		    	int minuta = local->tm_min;
		    	
		    	sleep(30);
		    	printf("Godzina: %d   Minuta: %d\n", godzina, minuta);
		    	
		    	if(zadania_tab[pom][0] == godzina && zadania_tab[pom][1] == minuta)
		    	{
		    		break;
		    	}
	    	}
	    	
	    	printf("Już pora");
	    	pom++;
	    
	    	pid_t pid2 = fork(); // proces potomny wykonujący zadanie
	    	
	    	openlog("proces potomny", LOG_PID, LOG_USER); //Inicjalizacja log
	    	
	    	if (pid2 < 0) 
	    	{
			printf("Nie udalo sie utworzyc procesu potomnego dla polecenia.");
			exit(EXIT_FAILURE);
	    	}
	    	
	    	if (pid2 >= 0)
	    	{
		
			fprintf(wypisanie, "%d:%d %s %d \n", zadania_tab[zadanie][0], zadania_tab[zadanie][1], komendy[zadanie], zadania_tab[zadanie][3]);
			
			syslog(LOG_INFO, "Uruchomiono zadanie %s\n", komendy[zadanie]);
			
			if(zadania_tab[zadanie][3] == 0)
			{
				fprintf(wypisanie, "0\n");
						
				kod_wyjscia = system(komendy[zadanie]);
				
			}

			if(zadania_tab[zadanie][3] == 1)
			{
				fprintf(wypisanie, "1\n");
			}

			if(zadania_tab[zadanie][3] == 2)
			{
				fprintf(wypisanie, "2\n");
			}
			
			syslog(LOG_INFO, "Zadanie %s zakonczone z kodem wyjscia %d\n", komendy[zadanie], kod_wyjscia);
		
		}
		
		
		
		zadanie++;
		if(zadanie == ilosc_zadan)
		{
		
		closelog();
		
		exit(EXIT_SUCCESS);
		break;
		}
	    }
	    
    }
    
    closelog();

	//nie wykasowujmy jeszcze tego - może się przyda

    /* Change the file mode mask */
    umask(0);

    /* Open any logs here */
    openlog("mydaemon", LOG_PID|LOG_CONS, LOG_USER);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        syslog(LOG_ERR, "Could not create new SID for child process");
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        syslog(LOG_ERR, "Could not change working directory to /");
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon-specific initialization goes here */

    /* The Big Loop */
    while (1) {
        /* Do some task here ... */
        syslog(LOG_INFO, "Hello, world!");

        sleep(60); /* wait 60 seconds */
    }

    exit(EXIT_SUCCESS);

    return 0;
}