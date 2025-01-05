#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Abifunktsioonid.h"
#include "Headers/Kõigetõlge.h"


extern struct TextmodeKäskList textmodeKäskList;


// Initialize a TextModeKäskList. Argumenti pole vaja, sest textmodekäsklist on globaalne. Mdea, miks initKäsklistil ja initEnvListil on nõutud argumenti.
void init_TextmodeKäsk_list() {
    textmodeKäskList.käsud = malloc(1 * sizeof(struct TextmodeKäsk)); // -> on sama mis (*list).käsud
    textmodeKäskList.kogus = 0;
    textmodeKäskList.maht = 1;
}



void LisaTextmodeKäsk(struct TextmodeKäsk käsk)
{
    // Kui lisatav käsk ei mahuks käsunimekirja, ss eraldatakse mälu ja tehakse nimekiri kaks korda mahukamaks.
    if (textmodeKäskList.kogus >= textmodeKäskList.maht)
    {
        textmodeKäskList.maht *= 2;
        textmodeKäskList.käsud = realloc(textmodeKäskList.käsud, textmodeKäskList.maht*sizeof(struct TextmodeKäsk));
    }
    textmodeKäskList.käsud[textmodeKäskList.kogus] = käsk;
    textmodeKäskList.kogus += 1;
}


struct TextmodeKäsk* KasTextmodeKäsk(char* tekst)
{
    for (unsigned int i = 0; i<textmodeKäskList.kogus; i++)
    {
        if (KasEsimesedTähed(tekst, textmodeKäskList.käsud[i].käsualgus))
        {
            return &textmodeKäskList.käsud[i];
        }
    }
    return NULL;
}



/*
Tlgikäsu kood peaks olema järgmine:
antud on käsk ja tekst, mis algab sellest käsust. näiteks
sumxk=0 m

Tuleb lugeda argumentide väärtused lähtekoodist nimekirja sellises järjekorars nagu need lähtekoodis on. Muuseas need väärtused ära tõlkida. Siis need on samas järjekooras nagu need käsu dfinitsiooni vasakus pooles on. 

Minna üle definitsiooni parema poole ja iga tähe peal kontrollida, kas seal algab sellesama käsu mõni argumendinimi. Kui ei alga, ss lic panna sealt tähti tõlkesse. Kui seal on mõni argumendinimi, siis tuleb teha selgeks, mitmes argument see definitsiooni vasaku poole suhtes on. Selleks tuleb minna üle antud käsu argumentide nimede nimekirja. Kui on selge, mitmenda argumendi kasutamisega on tegu, siis võtta varem koostatud argumentide väärtuste nimekijrast selle sama indeksiga element ja liita see tõlkele.
*/


// Funktsioon, millele antakse mingi tekst ja see teeb selle tekstiga pmst sama, mis tavaline tõlgikõik, ainult et läbib tsükli ainult ühe korra ja ainult esimese tähe peal. Seega kui esimene täht pole mingi käsu algus, ss lic tagastatakse tekstarv, kus tekst on see üks täht ja arv on 1. Kui aga esimesel tähel on käsk, ss tõlgitakse terve see käsk, tagastatakse käsu tõlge ja arv, kui mitu tähte tuleb tõlgitavas tekstis edasi minna. Seda funtksiooni on vaja, et textmodekäskude tõlkimisel teha tõlkimist tähthaaval, lastes kontrollida pärast iga tõlke sammu, ega polda argumendi lõpuni jõutud. Literaalselt copy-paste tõlgikõigest, aga tsükkel jookseb ainult ühe korra ja minimaalsed muudatused selleks, et tagastaks tekstarvu.
struct TekstArv TõlgiKõikSellesKohas(char* tõlgitav)
{
    struct TekstArv tulemus;

    char* tõlge = malloc(1);
    tõlge[0] = '\0';


    unsigned int pikkus = strlen(tõlgitav);
    // Läheb üle iga tähe kogu tõlgitavas tekstis. Literaalselt copy-paste tõlgikõigest, aga tsükkel ainult üks kord.
    for (unsigned int i = 0; i<1; )
    {
        struct TextmodeKäsk* käsk = KasTextmodeKäsk(&tõlgitav[i]);
        if (käsk) // Sama mis käsk != NULL
        {
            struct TekstArv käsuTõlge = TõlgiTextmodeKäsk(&tõlgitav[i], käsk);
            tõlge = LiidaTekstid(tõlge, käsuTõlge.Tekst);
            free(käsuTõlge.Tekst);
            i += käsuTõlge.Arv;
            
            tulemus.Arv = i;
            tulemus.Tekst = tõlge;
            return tulemus;
        }

        struct Environment* env = KasEnvironment(&tõlgitav[i]);
        if (env)
        {
            struct TekstArv envTõlge = TõlgiEnvironment(&tõlgitav[i], env, 0);
            tõlge = LiidaTekstid(tõlge, envTõlge.Tekst);
            free(envTõlge.Tekst);
            i += envTõlge.Arv;

            tulemus.Arv = i;
            tulemus.Tekst = tõlge;
            return tulemus;
        }
        if ( i == 0 && KasEsimesedTähed(&tõlgitav[i], "mm ") || KasEsimesedTähed(&tõlgitav[i], " mm ") || KasEsimesedTähed(&tõlgitav[i], "\nmm ") || KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"))
        {
            int onDisplayMath = 0;
            if (KasEsimesedTähed(&tõlgitav[i], " mm "))
            {
                tõlge = LiidaTekstid(tõlge, " $");
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "mm "))
            {
                tõlge = LiidaTekstid(tõlge, "$");
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\begin{gather*}\n");
                onDisplayMath = 1;
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm "))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\begin{gather*}");
                onDisplayMath = 1;
            }
            i += (KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n") || KasEsimesedTähed(&tõlgitav[i], "\nmm ") || KasEsimesedTähed(&tõlgitav[i], " mm ") ? 4 : 3);

            unsigned int start = i;
            while (i < pikkus && !(KasEsimesedTähed(&tõlgitav[i], " mm") || KasEsimesedTähed(&tõlgitav[i], "\nmm"))) {
                i++;
            }

            char* tõlgitavMata = VõtaTekstIndeksini(&tõlgitav[start], i - start);
            char* mataTõlge = TõlgiMathMode(tõlgitavMata);
            tõlge = LiidaTekstid(tõlge, mataTõlge);

            free(mataTõlge);
            free(tõlgitavMata);

            if (onDisplayMath == 0) {
                tõlge = LiidaTekstid(tõlge, "$");
            } else {
                tõlge = LiidaTekstid(tõlge, "\\end{gather*}");
            }

            i += 3;
            tulemus.Arv = i;
            tulemus.Tekst = tõlge;
            return tulemus;
        }
        else 
        {
            char* täht = malloc(2);
            täht[0] = tõlgitav[i];
            täht[1] = '\0';
            tõlge = LiidaTekstid(tõlge, täht);
            free(täht);
            i++;

            tulemus.Arv = i;
            tulemus.Tekst = tõlge;
            return tulemus;
        }
    }
}



struct TekstArv TõlgiTextmodeKäsk(char* tekst, struct TextmodeKäsk* käsk)
{
    char* tõlge = malloc(1);
    tõlge[0] = '\0';

    char** argumentideTõlked = malloc(käsk->argumentideKogus*sizeof(char*));

    int i = strlen(käsk->käsualgus); // Nüüd tekst[i] on lähtekoodis esimese argumendi algus
    // Algab lähtekoodi lugemine ja igale argumendile lähtekoodist vastava teksti leidmine.
    // Teeb asja argumentidekogus korda. (iga argumendi jaoks)
    for (unsigned int j = 0; j< käsk->argumentideKogus; j++)
    {
        char* argumendiTõlge = malloc(1);
        argumendiTõlge[0] = '\0';
        // Läheb üle lähtekoodi teksti tähthaaval ja kui mõni käsk tuleb ette, ss kutsub rekursiivselt välja uue textmode käsu tõlkimist. Kui lõpuks on nii, et kõik ettetulnud käsud on tõlgitud ja ette tuleb argumendilõpp, ss on argument eraldatud.
        for ( ; KasEsimesedTähed(&tekst[i], käsk->argumentideLõpud[j]) == 0; )
        {
            struct TekstArv kohaTõlge = TõlgiKõikSellesKohas(&tekst[i]);
            
            argumendiTõlge = LiidaTekstid(argumendiTõlge, kohaTõlge.Tekst);
            i += kohaTõlge.Arv;
        }
        // Kui kood jõuab siia ss järelikult on käsud ületatud ja jõuti argumednilõõpu juurde.
        argumentideTõlked[j] = argumendiTõlge;
        i += strlen(käsk->argumentideLõpud[j]); // Nüüd tekst[i] on järgmise argumendi esimesel kohal.
    }

    //puts("VÄLJUS ESIMESEST LOOPIST");
    // Teksti määran allpool.
    struct TekstArv tagastus = {.Arv = i, .Tekst=NULL};

    
    i = 0;
    // Läheb üle definitsiooni iga tähe. Ja paneb tõlkesse definitsiooni tähe või argumendi tõlke.
    for ( ; i<strlen(käsk->definitsioon); )
    {
        // Läheb iga tähe peal üle selle käsu iga argumendinime.
        for (unsigned int j = 0; j<käsk->argumentideKogus; j++)
        {
            // Kas sellel kohal on mingi argumendinimi?
            if (KasEsimesedTähed(&käsk->definitsioon[i], käsk->argumentideNimed[j]))
            {
                // Kui on, siis mitte lisada definitsooni tähte, vaid argumedni tõlget.
                tõlge = LiidaTekstid(tõlge, argumentideTõlked[j]);
                i += strlen(käsk->argumentideNimed[j]);
            }
            else
            {
                // Kui ei ole argumendinime, ss pannakse ns defintitsioobni täht tõlkessse.
                tõlge = LiidaTäht(tõlge, käsk->definitsioon[i]);
                i++;
            }
        }
    }


    tagastus.Tekst = tõlge;
    return tagastus;
}


void TextmodeKäsudConfigist(char* config_path)
{
    FILE* file = fopen(config_path, "r");
    if (!file) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    char* line;
    int onTextmodeKäskudeJuures = 0;
    while ((line = read_line(file)) != NULL) 
    {
        if (KasEsimesedTähed(line, "TEXTMODE KÄSUD"))
        {
            free(line);
            onTextmodeKäskudeJuures = 1;
            continue;
        }
        if (KasEsimesedTähed(line, "KESKKONNAD") || KasEsimesedTähed(line, "MATHMODE KÄSUD"))
        {
            free(line);
            onTextmodeKäskudeJuures = 0;
            continue;
        }
        // Skip empty lines or comments
        if (line[0] == '\0' || line[0] == '#')
        {
            free(line);
            continue;
        }
        if (onTextmodeKäskudeJuures)
        {
            // Teskst enne " -> "
            char* vasak = LeiaTekstEnneTeksti(line, " -> ");

            // Terve see kood on debuginfo väljund. Mitte vaadata.
            
            if (strlen(vasak) == strlen(line))
            {
                printf("Textmode käskude lugemise juures configist on kahtlande rida.\n");
                char* prinditav = malloc(1);
                prinditav[0] = '\0';
                prinditav = LiidaTekstid(prinditav, "\"");
                prinditav = LiidaTekstid(prinditav, line);
                prinditav = LiidaTekstid(prinditav, "\"\n");
                prindiVärviga(prinditav, "punane");
                puts("Sellelt realt ei leitud sõne \" -> \"");
                free(prinditav);
            }

            // Tekst pärast " -> "
            char* parem = strdup(&line[strlen(vasak)+4]);

            struct TextmodeKäsk käsk = {.käsualgus = NULL, .argumentideKogus=0, .argumentideNimed=malloc(sizeof(char*)), .argumentideLõpud=malloc(sizeof(char*)), .definitsioon=NULL};

            char* algus = LeiaTekstEnneTeksti(vasak, "(");
            käsk.käsualgus = algus;

            // Kui leiti ilma argumentideta textmode käsk
            if (strlen(algus) == strlen(vasak))
            {
                käsk.argumentideNimed = NULL;
                käsk.argumentideLõpud = NULL;
                käsk.argumentideKogus = 0;
                käsk.definitsioon = parem;
                LisaTextmodeKäsk(käsk);

                continue;
            }

            // Kui kood jõuab siia, ss leiti tekst, millel on argumente.
            int i = strlen(algus); // Nüüd vasak[i] on esimese argumendi avav sulg.
            while (vasak[i] != '\0')
            {
                i++; // Nüüd vasak[i] on argumendi esimene täht
                char* argumendiNimi = LeiaTekstEnneTeksti(&vasak[i], ")");
                i += strlen(argumendiNimi); // Nüüd vasak[i] on argumenti sulgev sulg.
                //puts(&vasak[i]);
                i++; // vasak[i] on argumendilõpu esimene täht
                char* argumendiLõpp = LeiaTekstEnneTeksti(&vasak[i], "(");
                i+=strlen(argumendiLõpp);
                if (strlen(argumendiLõpp) == 0)
                {
                    argumendiLõpp = LiidaTekstid(argumendiLõpp, "\n");
                }


                käsk.argumentideNimed = realloc(käsk.argumentideNimed, (käsk.argumentideKogus+1)*sizeof(char*));
                käsk.argumentideLõpud = realloc(käsk.argumentideLõpud, (käsk.argumentideKogus+1)*sizeof(char*));
                käsk.argumentideNimed[käsk.argumentideKogus] = argumendiNimi;
                käsk.argumentideLõpud[käsk.argumentideKogus] = argumendiLõpp;
                käsk.argumentideKogus += 1;
                //printf("Argumentide kogus: %d\n",  käsk.argumentideKogus);
                // Nüüd vasak[i] on argumenti sulgev sulg.
            }

            käsk.definitsioon = parem;
            //printf("Siin on käsu info:\n");
            //puts(käsk.käsualgus);
            //puts(käsk.argumentideLõpud[0]);
            //puts(käsk.argumentideNimed[0]);
            //puts(käsk.definitsioon);
            LisaTextmodeKäsk(käsk);
        }
        free(line);
    }
    fclose(file);
}



/*
Tlgikäsu kood peaks olema järgmine:
antud on käsk ja tekst, mis algab sellest käsust. näiteks
sumxk=0 m

Tuleb lugeda argumentide väärtused lähtekoodist nimekirja sellises järjekorars nagu need lähtekoodis on. Siis need on samas järjekooras nagu need käsu dfinitsiooni vasakus pooles on. 

Minna üle definitsiooni parema poole ja iga tähe peal kontrollida, kas seal algab sellesama käsu mõni argumendinimi. Kui ei alga, ss lic panna sealt tähti tõlkesse. Kui seal on mõni argumendinimi, siis tuleb teha selgeks, mitmes argument see definitsiooni vasaku poole suhtes on. Selleks tuleb minna üle antud käsu argumentide nimede nimekirja. Kui on selge, mitmenda argumendi kasutamisega on tegu, siis võtta varem koostatud argumentide väärtuste nimekijrast selle sama indeksiga element ja liita see tõlkele.
*/


char* TõlgiKõik(char* tõlgitav)
{
    char* tõlge = malloc(1);
    tõlge[0] = '\0';


    unsigned int pikkus = strlen(tõlgitav);
    // Läheb üle iga tähe kogu tõlgitavas tekstis.
    for (unsigned int i = 0; i<pikkus; )
    {
        struct TextmodeKäsk* käsk = KasTextmodeKäsk(&tõlgitav[i]);
        if (käsk) // Sama mis käsk != NULL
        {
            struct TekstArv käsuTõlge = TõlgiTextmodeKäsk(&tõlgitav[i], käsk);
            tõlge = LiidaTekstid(tõlge, käsuTõlge.Tekst);
            free(käsuTõlge.Tekst);
            i += käsuTõlge.Arv;
        }

        struct Environment* env = KasEnvironment(&tõlgitav[i]);
        if (env)
        {
            struct TekstArv envTõlge = TõlgiEnvironment(&tõlgitav[i], env, 0);
            tõlge = LiidaTekstid(tõlge, envTõlge.Tekst);
            i += envTõlge.Arv;
            free(envTõlge.Tekst);
        }
        else
        if ( i == 0 && KasEsimesedTähed(&tõlgitav[i], "mm ") || KasEsimesedTähed(&tõlgitav[i], " mm ") || KasEsimesedTähed(&tõlgitav[i], "\nmm ") || KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"))
        {
            int onDisplayMath = 0;
            if (KasEsimesedTähed(&tõlgitav[i], " mm "))
            {
                tõlge = LiidaTekstid(tõlge, " $");
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "mm "))
            {
                tõlge = LiidaTekstid(tõlge, "$");
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\begin{gather*}\n");
                onDisplayMath = 1;
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm "))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\begin{gather*}");
                onDisplayMath = 1;
            }
            i += (KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n") || KasEsimesedTähed(&tõlgitav[i], "\nmm ") || KasEsimesedTähed(&tõlgitav[i], " mm ") ? 4 : 3);

            unsigned int start = i;
            while (i < pikkus && !(KasEsimesedTähed(&tõlgitav[i], " mm") || KasEsimesedTähed(&tõlgitav[i], "\nmm"))) {
                i++;
            }

            char* tõlgitavMata = VõtaTekstIndeksini(&tõlgitav[start], i - start);
            char* mataTõlge = TõlgiMathMode(tõlgitavMata);
            tõlge = LiidaTekstid(tõlge, mataTõlge);

            free(mataTõlge);
            free(tõlgitavMata);

            if (onDisplayMath == 0) {
                tõlge = LiidaTekstid(tõlge, "$");
            } else {
                tõlge = LiidaTekstid(tõlge, "\\end{gather*}");
            }

            i += 3;
        }
        else 
        {
            char* täht = malloc(2);
            täht[0] = tõlgitav[i];
            täht[1] = '\0';
            tõlge = LiidaTekstid(tõlge, täht);
            free(täht);
            i++;
        }
    }
    
    return tõlge;
}