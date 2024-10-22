#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"


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


// mingi list tuntud funktsioonidest
const char* math_functions[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "fii", "roo", "alfa", "beeta", "epsilon", "to", "inf", NULL};

// duplikeerib antud stringi kuni n baidini (tagastab pointeri uuele stringile)
char* my_strndup(const char* s, size_t n) {
    char* result;
    size_t len = strlen(s);

    if (n < len) {
        len = n;
    }

    result = (char*)malloc(len + 1); // Mälu substringi jaoks
    if (!result) {
        perror("Mälu ei saadud eraldada. :(\n");
        exit(EXIT_FAILURE);
    }

    strncpy(result, s, len); // Kopeerime kuni n characteri
    result[len] = '\0'; //Null-terminate
    return result;
}

// Kas antud string on listis olev funktsioon
int is_math_function(const char* str) {
    for (int i = 0; math_functions[i] != NULL; i++) {
        if (strncmp(str, math_functions[i], strlen(math_functions[i])) == 0) {
            return strlen(math_functions[i]);
        }
    }
    return 0;
}

// Abimeetod, mis appendib kaks stringi
char* append_str(const char* a, const char* b) {
    char* result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

// Rekursiivselt tõlgime math moodi latexisse
char* TõlgiMathMode(const char* expression) {
    char* result = malloc(1); // Tühi string
    result[0] = '\0';
    
    int i = 0;
    while (expression[i] != '\0') {
        // Kontrollime kas on mõni tuntud käsk
        int func_len = is_math_function(&expression[i]);
        if (func_len != 0) {
            //Leiame funktsiooni nime
            //Kopeerime leitud funktsiooni nime, et siis saaksime seda kasutada kui eraldi "muutujat". Me ei taha modifitseerida algset *expression* stringi.
            //Samuti saame selle eluaega ise määrata, st. vabastada, millal soovime
            char* func_name = my_strndup(&expression[i], func_len);

            
            // Lisame vajaliku latex süntaksi
            // !!! Siin on mäluleke imo. muutuja func_name mälu vabastatakse, aga see mälu, mille eraldab sisemine append_str, ei saa kunagi vabastatud. See lic eraldatakse, mäluaadress antakse teisele appendstr funktsioonile, mis eraldab uut mälu, aga eelmist ei vabasta. Ainult kõige viimasena eraldatud mälu vabastatakse. !!!
            char* latex_func_with_left = append_str(append_str("\\", func_name), "\\left(");
            free(func_name);

            result = append_str(result, latex_func_with_left);
            free(latex_func_with_left);

            i += func_len; // Liigume mööda antud käsust

            // Näpime seda argumenti sulgude vahel
            if (expression[i] == '(') {
                int start = i + 1;
                int paren_count = 1;
                i++;

                // Leiame sulud
                while (expression[i] != '\0' && paren_count > 0) {
                    if (expression[i] == '(') paren_count++;
                    else if (expression[i] == ')') paren_count--;
                    i++;
                }

                // Leiame rekursiivselt sisu
                char* inner_expression = my_strndup(&expression[start], i - start - 1);
                char* inner_latex = TõlgiMathMode(inner_expression);
                free(inner_expression);

                result = append_str(result, inner_latex);
                free(inner_latex);

                result = append_str(result, "\\right)");
            }
        } else {
            // Kõik ülejäänud pask läheb tavaliselt
            char* single_char = my_strndup(&expression[i], 1);
            result = append_str(result, single_char);
            free(single_char);
            i++;
        }
    }

    return result;
}


/* Funktsioon võtab sisse mäluaadressi, kus on limi algus. Funktsioon hakkab sealt teksti lugema, eraldab endale sobiva koguse mälu ja kirjutab sellesse loetud teksti tõlke. Tagastab structi, mis sisaldab endas tõlke alguse aadressi ja seda, kui palju funktsiooni väline kood peaks lähtekoodis edasi hüppama, et jõuda tekstis sinnamaale, kus lim läbi saab.
Täpsemalt on kolm võimalust, mis limi tekstile järgnev jähemärk olla saab. Iga võimaluse puhul käitutakse erinevalt.
1) limi järel on tühik. Siis limi alla teksti ei tule ja lim on üksinda. Selline lim on jadade piirväärtusel.
2) limi järel on avanev sulg. Siis limi alla läheb kõik see, mis jääb nimetatud avaneva ja seda sulgeva sulu vahele.
3) limi järel on mingi muu täht. Siis läheb limi alla kõik see, mis jääb limi ja esimese tühiku vahele.
*/
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
    if (tekst[i] == '(')
    {
        char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
        char* tõlge = TõlgiMathMode(sulusisu);
        free(sulusisu);
    }
}
