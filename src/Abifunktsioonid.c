#include <stdio.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"
#include <stdlib.h>
#include <Windows.h>
#include <direct.h>  // for _getcwd on Windows

#define MAX_PATH_LENGTH 256

extern struct KäskList käskList;
extern struct EnvironmentList environList;
extern struct TextmodeKäskList textmodeKäskList;
extern unsigned int rekursiooniTase;
extern unsigned int taandePikkus;


void prindiTaane()
{
    for (unsigned int i = 0; i<rekursiooniTase; i++)
    {
        printf("|");
        for(unsigned int j = 0; j<taandePikkus-1; j++)
        {
            printf(" ");
        }
    } 
}


void prindiTextmodeKäsuInfo(struct TextmodeKäsk* käsk)
{
    printf("%s\n", käsk->käsualgus);
    printf("%s\n", käsk->definitsioon);
    printf("%d\n", käsk->argumentideKogus);
    for (unsigned int i = 0; i<käsk->argumentideKogus; i++)
    {
        printf("Arg%d: %s, Lõpp: %s\n", i+1, käsk->argumentideNimed[i],käsk->argumentideLõpud[i]);
    }
}


void prindiVärviga(char* tekst, char* värv)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hStdout, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, dwMode);
    if (strcmp(tekst, "roheline"))
    {
        printf("\033[1;32m");
        printf("%s", tekst);
    }
    else if (strcmp(värv, "punane"))
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


void InitTextModeKäsudList()
{
    textmodeKäskList.käsud = malloc(1 * sizeof(struct Käsk)); // -> on sama mis (*list).käsud
    textmodeKäskList.kogus = 0;
    textmodeKäskList.maht = 1;
}


// Funktsioon, mis loeb kõik mathmode käsud, textmode käsud ja keskkonnad config.txt failist ja paneb need vastavatesse structidesse. Kui workingdirectorys ei ole configit, siis teeb sinna koopia exediris olevast configist.
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

    // Tahan lugeda configit workingdirectoryst. Kui workingdirectorys ei ole configit, siis teha exediril olevast config failist workingdirctoysse koopia ja ikkagi lugeda seda workingdirectoryst.
    char cwdConfigPath[MAX_PATH_LENGTH];
    snprintf(cwdConfigPath,MAX_PATH_LENGTH, "%s\\config.txt", cwd);
    FILE* configiFail = fopen(cwdConfigPath, "r");
    if (configiFail == NULL)
    {
        printf("\nTEHTI UUS CONFIG.\n\n");
        // config faili pole cwd-s. Tuleb teha sellest exediris koopia.
        FILE* configiFail = fopen(config_path, "r");
        FILE* uusCwdConfig = fopen(cwdConfigPath, "w");

        char* line = NULL;
        while ((line = read_line(configiFail)) != NULL)
        {
            fprintf(uusCwdConfig, "%s\n", line);
            free(line);
        }
        fclose(configiFail);
        fclose(uusCwdConfig);
    }


    init_käsk_list(&käskList);
    read_commands_from_config(cwdConfigPath, &käskList);

    init_environment_list(&environList);
    read_environments_from_config(cwdConfigPath, &environList);

    InitTextModeKäsudList();
    TextmodeKäsudConfigist(cwdConfigPath);
    //puts("TextmodeKäsudConfigist jooksis.");


    /*
    init_käsk_list(&käskList);
    read_commands_from_config(config_path, &käskList);

    init_environment_list(&environList);
    read_environments_from_config(config_path, &environList);

    InitTextModeKäsudList();
    TextmodeKäsudConfigist(config_path);
    puts("TextmodeKäsudConfigist jooksis.");*/
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