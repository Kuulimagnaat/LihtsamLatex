#include <stdio.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include <stdlib.h>
#include <Windows.h>
#include <direct.h>  // for _getcwd on Windows

#define MAX_PATH_LENGTH 256

extern struct KäskList käskList;
extern struct EnvironmentList environList;
extern struct TextmodeKäskList textmodeKäskList;


void prindiVärviga(char* tekst, char* värv)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hStdout, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, dwMode);
    if (värv == "roheline")
    {
        printf("\033[1;32m");
        printf("%s", tekst);
    }
    else if (värv == "punane")
    {
        printf("\033[31m");
        printf("%s", tekst);
    }
    printf("\033[0m");
}



// Tagastab teksti, mis eelneb indeksiga märgitud kohale.
char* VõtaTekstIndeksini(char* tekst, unsigned int indeks)
{
    char* mälu = malloc(indeks+1);
    memcpy(mälu, tekst, indeks);
    mälu[indeks] = '\0';
    return mälu;
}



long int LeiaFailiSuurus(FILE* f)
{
    fseek(f, 0, SEEK_END); // seek to end of file
    long int suurus = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET);
    return suurus;
}


// Funktsioon, mis eraldab mälu ja kopeerib sinna antud tekstist kõik selle, mis jääb enne esimest antud lõppu. Kui ükski lõpp ei klapi, ss eraldab mälu ja kopeerib sinna lic esialgse teksti. On selge, et selle funktsiooni tagastatud mälu tuleb tagastada.
char* LeiaTekstEnneTekste(char* tekst, char** lõpud, unsigned int lõppudeKogus)
{
    for (unsigned int i = 0; tekst[i]!='\0'; i++)
    {
        for (unsigned j =0; j< lõppudeKogus; j++)
        {
            if (KasEsimesedTähed(&tekst[i], lõpud[j]))
            {
                char* eelnevTekst = malloc(i+1);
                memcpy(eelnevTekst, tekst, i);
                eelnevTekst[i] = '\0';
                return eelnevTekst;
            }
        }
    }
    char* eelnevTekst = malloc(strlen(tekst)+1);
    memcpy(eelnevTekst, tekst, strlen(tekst)+1); // Tekstilõpumärk kopeeritakse ka.
    return eelnevTekst;
}



// Funktsioon, mis loeb kõik mathmode käsud, textmode käsud ja keskkonnad config.txt failist ja paneb need vastavatesse structidesse.
void AmmendaConfig()
{
    // ...\luuga\duuga\        <-- Kaust, kust programm käivitati – currend working directory.
    char cwd[MAX_PATH_LENGTH];
    // ...\uuga\buuga\a.exe        <-- Programmi exe faili directory, kus on ka programmi exe faili nimi lõpus.
    char exe_path[MAX_PATH_LENGTH];
    // ...\uuga\buuga\          <-- sama dir, aga ilma a.exe nimeta lõpus
    char exe_dir[MAX_PATH_LENGTH];

    // Get the current working directory for main.txt. Seda asukohta on vaja selleks, et sellel aadressil asuvat kasutaja kirjutatavat lähtekoodifaili avada.
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("_getcwd() error");
        return;
    }

    // Get the executable full name. Seda teksti on vaja selleks, et sellest eraldada directory, kus exe fail asub, et seda kasutada muude failide nt config.txt ja SVEN.txt avamiseks.
    if (GetModuleFileName(NULL, exe_path, MAX_PATH_LENGTH) == 0) {
        perror("GetModuleFileName() error");
        return;
    }

    // Strip the executable name to get the directory
    strcpy(exe_dir, exe_path);
    char* last_backslash = strrchr(exe_dir, '\\');
    if (last_backslash) {
        *last_backslash = '\0';  // Cut the path at the last backslash
    }


    // Koostatakse config.txt faili aadress. Sealt eraldatakse kasutaja defineeritud käsud ja need pannakse käskListi objekti.
    char config_path[MAX_PATH_LENGTH];
    snprintf(config_path, sizeof(config_path), "%s\\src\\config.txt", exe_dir);



    init_käsk_list(&käskList);
    read_commands_from_config(config_path, &käskList);

    init_environment_list(&environList);
    read_environments_from_config(config_path, &environList);
}



long int findSize(char* file_name) 
{ 
    // opening the file in read mode 
    FILE* fp = fopen(file_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    long int res = ftell(fp); 
  
    // closing the file 
    fclose(fp); 
  
    return res; 
} 



// Funktsioon, mis leiab kui mitme tähe võrra erineb teine tekst esimesest tekstist.
unsigned int MitmeTäheVõrraErineb(const char* tekst1, const char* tekst2)
{
    unsigned int loendur = 0;
    for (unsigned int i = 0; i<strlen(tekst1); i++)
    {
        if (tekst1[i] != tekst2[i])
        {
            loendur += 1;
        }
    }
    return loendur;
}


