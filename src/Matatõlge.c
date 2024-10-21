#include "Headers/Matatõlge.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// Funktsioon, mis kontorllib, kas antud teksti algus täpselt on sama, mis teine soovitud tekst. Tagastab 0 kui ei ole ja 1 kui on. Funktsiooni kasutusolukord: kui ollakse minemas tõlgitavas koodis üle tähtede, siis on vaja kontrollida, kas kättejõudnud kohas on mõne käsu nimi. Seda funktsiooni saab nimetatud olukorra tajumiseks kasutada.
int KasEsimesedTähed(const char* tekstis, const char* tekst)
{
    unsigned int tähtiKontrollida = strlen(tekst);
    for (unsigned int i = 0; i<tähtiKontrollida; i++)
    {
        if (tekstis[i] != tekst[i])
        {
            return 0;
        }
    }
    return 1;
}


// Funktsiooni kasutatakse juhul, kui jõutakse tõlgitavas koodis funktsioonini, mille järel on avanev sulg. Siis antakse sellele funktsioonile avanevale sulule järgneva tähe mäluaadress, misjärel funktsioon leiab kogu teksti, mis peaks avaneva ja sulgeva sulu vahele jääma. Funktsioon eraldab mälu, täidab selle sulu sisuga ja tagastab selle mälu aadressi. See mälu on vaja hiljem vabastada.
char* LeiaSuluSisu(const char* tekst)
{
    unsigned int maht = 32;
    char* mälu = malloc(maht);
    //Sulutase algab arvust 1, sest otsitakse sulgu, mis viiks sulutaseme nulli.
    unsigned int sulutase = 1;
    for (unsigned int i = 0; tekst[i]!='\0'; i++)
    {
        if (i >= maht)
        {
            maht += 32;
            mälu = realloc(mälu, maht);
            if (mälu == NULL)
            {
                perror("Ei õnnestunud mälu eraldada :(");
                exit(EXIT_FAILURE);
            }
        }
        if (tekst[i] == '(')
        {
            sulutase += 1;
        }
        else if (tekst[i] == ')')
        {
            sulutase -= 1;
            if (sulutase == 0)
            {
                mälu[i] = '\0';
                return mälu;
            }
        }
        mälu[i] = tekst[i];
    }
    perror("Ei leitud sulgevat sulgu :(");
    exit(EXIT_FAILURE);
}


// Vaja oleks mingit funktsiooni, mis tagastaks tõeväärtuse selle kohta, kas meie latexi koodi tekst kujutab endast avaldist, mille viimane tehe on murrujoon. Seda funktsiooni kasutaks selle jaoks, et aru saada, kas latexi koodis näiteks siinuse argumendi ümber on vaja panna sulud või mitte. Tõlgitavas koodis vb on kirjutatud sin(a/b), aga tegelikult latexi faili läheb murd siinuse järele ilma sulgudeta. N2: tõlgitavas koodis võib olla kirjas sin(a+b) ja selles olurkorras tuleb sulud alles jätta ka latexi koodis.


// Ma olen veendunud, et tõlkimises tühikute asjaks kasutamine on saatanast. Igast kohast, kus tühikuga tahaks tajuda argumendi algust vms, alati on võimalik mingi olukord välja mõelda, kus on mitu võimalikku tõlgendust. Seepärast las sinx/2 ja sin x/2 olla samad asjad, mis tähendavad järgmist (sin(x))/2. Siinus, kui tema järel pole sulgi, võtab oma argumendiks esimese tähemärgi ja ülejäänud kood jätkab, arvestades, et siinuse argument on see esimene temale järgnev tähemärk.


void MahtKogusTõlge(unsigned int* maht, unsigned int* kogus, char* tõlge, const char* lisatavTekst)
{
    unsigned int lisanduvKogus = strlen(lisatavTekst);
    *kogus += lisanduvKogus;
    while (*kogus > *maht)
    {
        maht += 32;
        char* tõlge = realloc(tõlge, *maht);
        if (tõlge == NULL)
        {
            perror("Ei õnnestunud mälu eraldada MahtKogusTõlge funktsioonis. :(");
            exit(EXIT_FAILURE);
        }
    }
    strcat(tõlge, lisatavTekst);
}


// Funkstioon võtab argumendiks mäluaadressi, kus on math mode'i tekst. See hakkab sealt teksti lugema ja tõlkima seda latexi koodiks. Funktsioon eraldab omale sobiva koguse mälu ja kirjutab sinna latexiks tõlgitud teksti. Tagastatakse selle mälu aadress, kus on tõlge. Kunagi pärast selle funktsiooni kasutamist on vaja selle tagastatud mäluaadressi mälu vabastada. 
char* TõlgiMathMode(const char* tekst)
{
    unsigned int tõlkeMaht = 32;
    char* tõlge = malloc(tõlkeMaht);
    tõlge[0] = '\0'; // Esimene bait tuleb panna tekstilõpumärgiks, et seda saaks kasutada funktsioonis strcat.
    unsigned int kogus = 1;


    for (unsigned int i=0; tekst[i] != '\0'; )
    {
        printf("%d: %c.\n", i, tekst[i]);
        if (KasEsimesedTähed(&(tekst[i]), "sin"))
        {
            // Neli on tõlkesse lisanduva teksti pikkus. \sin on 4 tähte pikk. Paljudes kohtades siin funktsioonis on vaja tõlkele lisada uut teksti ja iga kord ei viitsi mahtu vajadusel kasvatada, kogust suurendada, errorit checkida jne. Selle asemel kasutan funktsiooni MahtKogusTõlge, mis teeb seda kõike.
            MahtKogusTõlge(&tõlkeMaht, &kogus, tõlge, "\\sin");


            // Kolm on tõlgitavas tekstis commandi pikkus. sin on 3 tähte pikk
            i+=3;
            if (tekst[i] == '(')
            {
                MahtKogusTõlge(&tõlkeMaht, &kogus, tõlge, "\\left(");

                // suluSisu vaja millalgi vabastada. TEHTUD
                char* suluSisu = LeiaSuluSisu(&tekst[i+1]);
                // suluSisuTõlge vaja millalgi vabastada. TEHTUD
                char* suluSisuTõlge = TõlgiMathMode(suluSisu);

                MahtKogusTõlge(&tõlkeMaht, &kogus, tõlge, suluSisuTõlge);

                i += 1 + strlen(suluSisu) + 1;
                free(suluSisu);
                free(suluSisuTõlge);

                MahtKogusTõlge(&tõlkeMaht, &kogus, tõlge, "\\right)");
            }
        }
        // Siia tulevad veel muude funtksioonide kontrollid ja veel tuleb jagamisega tegeleda.
        else
        {
            printf("Praegune täht on %c\n", tekst[i]);
            char uusTäht[2] = "\0"; /* gives {\0, \0} */
            uusTäht[0] = tekst[i]; 
            i += 1;
            MahtKogusTõlge(&tõlkeMaht, &kogus, tõlge, uusTäht);
            
        }
    }
    printf("%s\n", tõlge);
    return tõlge;
}
