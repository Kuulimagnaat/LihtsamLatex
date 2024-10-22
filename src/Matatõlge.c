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


// SEDA FUNKTSIOONI VÕIB USALDADA: Käsitsi läbi katsetatud.
// Funktsiooni kasutatakse juhul, kui jõutakse tõlgitavas koodis funktsioonini, mille järel on avanev sulg. Siis antakse sellele funktsioonile avanevale sulule järgneva tähe mäluaadress, misjärel funktsioon leiab kogu teksti, mis peaks avaneva ja sulgeva sulu vahele jääma. Funktsioon eraldab mälu, täidab selle sulu sisuga ja tagastab selle mälu aadressi. See mälu on vaja hiljem vabastada.
char* LeiaSuluSisu(const char* tekst)
{
    unsigned int maht = 32;
    char* mälu = malloc(maht);
    if (mälu == NULL)
    {
        perror("Ei õnnestunud mälu eraldada :(");
        exit(EXIT_FAILURE);
    }
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
            sulutase++;
        }
        else if (tekst[i] == ')')
        {
            sulutase--;
            if (sulutase == 0)
            {
                mälu[i] = '\0';
                return mälu;
            }
        }
        mälu[i] = tekst[i];
    }

    perror("Ei leitud sulgevat sulgu :(\n");
    exit(EXIT_FAILURE);
}


// Vaja oleks mingit funktsiooni, mis tagastaks tõeväärtuse selle kohta, kas meie latexi koodi tekst kujutab endast avaldist, mille viimane tehe on murrujoon. Seda funktsiooni kasutaks selle jaoks, et aru saada, kas latexi koodis näiteks siinuse argumendi ümber on vaja panna sulud või mitte. Tõlgitavas koodis vb on kirjutatud sin(a/b), aga tegelikult latexi faili läheb murd siinuse järele ilma sulgudeta. N2: tõlgitavas koodis võib olla kirjas sin(a+b) ja selles olurkorras tuleb sulud alles jätta ka latexi koodis.


// Ma olen veendunud, et tõlkimises tühikute asjaks kasutamine on saatanast. Igast kohast, kus tühikuga tahaks tajuda argumendi algust vms, alati on võimalik mingi olukord välja mõelda, kus on mitu võimalikku tõlgendust. Seepärast las sinx/2 ja sin x/2 olla samad asjad, mis tähendavad järgmist (sin(x))/2. Siinus, kui tema järel pole sulgi, võtab oma argumendiks esimese tähemärgi ja ülejäänud kood jätkab, arvestades, et siinuse argument on see esimene temale järgnev tähemärk.


struct LimiTagastus
{
    char* Tõlge;
    unsigned int TähtiLoeti;
};


// Abimeetod, mis appendib kaks stringi, kusjuures esimene nendest on dünaamiliselt eraldatud mälu.
char* LiidaTekstid(char* eelmineMälu, const char* lisatav)
{
    char* result = realloc(eelmineMälu, strlen(eelmineMälu) + strlen(lisatav) + 1);
    if (result == NULL)
    {
        perror("Ei õnnestunud mälu eraldada :(");
        exit(EXIT_FAILURE);
    }
    strcat(result, lisatav);
    return result;
}


// Funktsioon võtab sisse mäluaadressi, kus on limi algus. Funktsioon hakkab sealt teksti lugema, eraldab endale sobiva koguse mälu ja kirjutab sellesse loetud teksti tõlke. Tagastab structi, mis sisaldab endas tõlke alguse aadressi ja seda, kui palju funktsiooni väline kood peaks lähtekoodis edasi hüppama, et jõuda tekstis sinnamaale, kus lim läbi saab.
char* TõlgiLim(const char* tekst)
{
    char* mälu = malloc(1);
    if (mälu == NULL)
    {
        perror("Ei õnnestunud mälu eraldada :(");
        exit(EXIT_FAILURE);
    }
    mälu[0] = '\0';    

    unsigned int i = 0;
    mälu = LiidaTekstid(mälu, "\\lim");
    i += 3;
    if (mälu[i] != '(') // Järelikult kasutatakse kuju limatob, kus a on mingi sümbol ja b on mingi sümbol ja to on garanteeritud olema kusagil keskel.
    {
        unsigned int toIndeks;
        for(unsigned int j=i; tekst[j]!=' '; j++)
        {
            if(strncmp(&tekst[j], "to", 2))
            {
                toIndeks = j;
            }
        }
    }
}
