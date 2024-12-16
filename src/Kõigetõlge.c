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
    textmodeKäskList.kogus += 1;
    textmodeKäskList.käsud[textmodeKäskList.kogus] = käsk;
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

            struct TextmodeKäsk käsk = {0};

            char* algus = LeiaTekstEnneTeksti(vasak, "(");

            // Kui leiti ilma argumentideta textmode käsk
            if (strlen(algus) == strlen(vasak))
            {
                käsk.argumentideKogus = 0;
                käsk.argumentideNimed = NULL;
                käsk.käsualgus = algus;
                käsk.käsulõpp = NULL;
                käsk.definitsioon = parem;
                LisaTextmodeKäsk(käsk);
                free(vasak);
                free(parem);
                free(algus);
                continue;
            }

            // Kui kood jõuab siia, ss leiti tekst, millel on argumente.
            int i = strlen(algus); // Nüüd vasak[i] on esimese argumendi avav sulg.
            while (1)
            {
                
            }
        }
        free(line);
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
        struct Environment* env = KasEnvironment(&tõlgitav[i]);
        printf("\"\n%s\"\n", &tõlgitav[i]);
        printf("KAS ESIMESED ON NMMN: %d\n", KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"));
        if (env)
        {
            struct TekstArv envTõlge = TõlgiEnvironment(&tõlgitav[i], env);
            tõlge = LiidaTekstid(tõlge, envTõlge.Tekst);
            free(envTõlge.Tekst);
            i += envTõlge.Arv;
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
                puts("TAJUST NmmN");
                tõlge = LiidaTekstid(tõlge, "\n\\[\n");
                onDisplayMath = 1;
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm "))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\[");
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
                tõlge = LiidaTekstid(tõlge, "\\]");
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