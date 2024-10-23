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
unsigned int poolituskoht = 6;
const char* math_functions[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "fii", "roo", "alfa", "beeta", "epsilon", "to", "inf", "lim", NULL};
const char* math_functions_tähendused[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "varphi", "rho", "alpha", "beta", "varepsilon", "to", "infty", "lim", NULL};

// duplikeerib antud stringi kuni n baidini (tagastab pointeri uuele stringile)
char* my_strndup(const char* s, size_t n) {
    char* result;
    size_t len = strlen(s);

    if (n < len) {
        len = n;
    }

    result = malloc(len + 1); // Mälu substringi jaoks
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
    while (i < strlen(expression)) 
    {
        if (KasLugeja(&expression[i]) == 1)
        {
            puts("mind kutsuti välja");
            char* lugeja = LeiaTekstEnneTeksti(&expression[i], "/");
            char* lugejaTõlge = TõlgiMathMode(lugeja);
            char* nimetaja = LeiaNimetaja(&expression[i+strlen(lugeja)+1]);
            char* nimetajaTõlge = TõlgiMathMode(nimetaja);
            result = LiidaTekstid(result, "\\frac{");
            result = LiidaTekstid(result, lugejaTõlge);
            result = LiidaTekstid(result, "}{");
            result = LiidaTekstid(result, nimetajaTõlge);
            result = LiidaTekstid(result, "}");
            i += strlen(lugeja)+1+strlen(nimetaja);
            free(lugeja);
            free(nimetaja);
            free(lugejaTõlge);
            free(nimetajaTõlge);
            continue;
        }

        // Kontrollime kas on mõni tuntud käsk
        int func_len = 0;
        int j = 0;
        for (; math_functions[j] != NULL; j++) {
            if (strncmp(&expression[i], math_functions[j], strlen(math_functions[j])) == 0) {
                func_len = strlen(math_functions[j]);
                break;
            }
        }
        

        if (func_len != 0) {
            //Leiame funktsiooni nime
            //Kopeerime leitud funktsiooni nime, et siis saaksime seda kasutada kui eraldi "muutujat". Me ei taha modifitseerida algset *expression* stringi.
            //Samuti saame selle eluaega ise määrata, st. vabastada, millal soovime
            char* func_name = my_strndup(math_functions_tähendused[j], strlen(math_functions_tähendused[j]));

            
            // Lisame vajaliku latex süntaksi
            // !!! Siin on mäluleke imo. muutuja func_name mälu vabastatakse, aga see mälu, mille eraldab sisemine append_str, ei saa kunagi vabastatud. See lic eraldatakse, mäluaadress antakse teisele appendstr funktsioonile, mis eraldab uut mälu, aga eelmist ei vabasta. Ainult kõige viimasena eraldatud mälu vabastatakse. !!!
            char* latex_func_with_left;
            if (j>=poolituskoht)
            {
                if (strcmp(func_name, "lim") == 0)
                {

                    char* inner_expression = my_strndup(&expression[i], strlen(&expression[i]));
                    struct LimiTagastus tagastus = TõlgiLim(inner_expression); 

                    latex_func_with_left = tagastus.Tõlge;
                    i+=tagastus.TähtiLoeti;
                }
                else
                {
                    latex_func_with_left = append_str("\\", func_name);
                }
            }
            else{
                latex_func_with_left = append_str("\\", func_name);
                latex_func_with_left = append_str(latex_func_with_left, "\\left(");
            }

            //result = append_str(result, latex_func_with_left);
            result = LiidaTekstid(result, latex_func_with_left);
            if (strcmp(func_name, "lim") != 0)
            {
                i += func_len;
            } // Liigume mööda antud käsust
            free(func_name);
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
           free(latex_func_with_left);
        } 
        else 
        {
            // Kõik ülejäänud pask läheb tavaliselt
            char* single_char = my_strndup(&expression[i], 1);
            result = append_str(result, single_char);
            free(single_char);
            i++;
        }
    }
    return result;
}


char* LeiaTekstEnneTähte(const char* tekst, char täht)
{
    for (unsigned int i=0; i<strlen(tekst); i++)
    {
        if (tekst[i] == täht)
        {
            char* tekstEnne = malloc(i+1);
            memcpy(tekstEnne, tekst, i);
            tekstEnne[i] = '\0';
            return tekstEnne;
        }
    }
    return NULL;
}


char* LeiaTekstEnneTeksti(const char* tekst, const char* teksti)
{
    char* koht = strstr(tekst, teksti);
    if (koht == NULL)
    {
        char* leitud = malloc(strlen(tekst)+1);
        memcpy(leitud, tekst, strlen(tekst));
        leitud[strlen(tekst)] = '\0';
        return leitud;
    }
    char* leitud = malloc(koht-tekst+1);
    memcpy(leitud, tekst, koht-tekst);
    leitud[koht-tekst] = '\0';
    return leitud;
}


/* Funktsioon võtab sisse mäluaadressi, kus on limi algus. Funktsioon hakkab sealt teksti lugema, eraldab endale sobiva koguse mälu ja kirjutab sellesse loetud teksti tõlke. Tagastab structi, mis sisaldab endas tõlke alguse aadressi ja seda, kui palju funktsiooni väline kood peaks lähtekoodis edasi hüppama, et jõuda tekstis sinnamaale, kus lim läbi saab.
Täpsemalt on kolm võimalust, mis limi tekstile järgnev jähemärk olla saab. Iga võimaluse puhul käitutakse erinevalt.
1) limi järel on tühik. Siis limi alla teksti ei tule ja lim on üksinda. Selline lim on jadade piirväärtusel.
2) limi järel on avanev sulg. Siis limi alla läheb kõik see, mis jääb nimetatud avaneva ja seda sulgeva sulu vahele.
3) limi järel on mingi muu täht. Siis läheb limi alla kõik see, mis jääb limi ja esimese tühiku vahele. limlima/btoc (a+b)/c+d
*/
struct LimiTagastus TõlgiLim(const char* tekst)
{
    struct LimiTagastus tagastus = {.TähtiLoeti=0, .Tõlge=NULL};

    char* mälu = malloc(1);
    if (mälu == NULL)
    {
        perror("Ei õnnestunud mälu eraldada :(");
        exit(EXIT_FAILURE);
    }
    mälu[0] = '\0';    

    mälu = LiidaTekstid(mälu, "\\lim");
    if (tekst[3] == '(')
    {
        char* sulusisu = LeiaSuluSisu(&tekst[4]);
        char* tõlge = TõlgiMathMode(sulusisu);
        mälu = LiidaTekstid(mälu, "_{");
        mälu = LiidaTekstid(mälu, tõlge);
        free(tõlge);
        mälu = LiidaTekstid(mälu, "}");
        tagastus.TähtiLoeti = 4+strlen(sulusisu)+1;
        tagastus.Tõlge = mälu;
        free(sulusisu);
        return tagastus;
    }
    else if (tekst[3] == ' ')
    {
        mälu = LiidaTekstid(mälu, " ");
        tagastus.TähtiLoeti = 4;
        tagastus.Tõlge = mälu;
        return tagastus;
    }
    // Tegeleda hiljem põhjalikumalt üldisema juhu jaoks, kus limi all võib omakorda lim olla ja esimene tühik tajutakse ära sisemise limi omaks, seega loetake edasi otsimaks järgmist tühikut, mis lõpetaks välimise limi.
    else
    {
        char* limiAlla = LeiaTekstEnneTähte(&tekst[3], ' ');
        if (limiAlla == NULL)
        {
            free(limiAlla);
            limiAlla = &tekst[3];
        }
        char* tõlge = TõlgiMathMode(limiAlla);
        mälu = LiidaTekstid(mälu, "_{");
        mälu = LiidaTekstid(mälu, tõlge);
        free(tõlge);
        mälu = LiidaTekstid(mälu, "}");
        tagastus.TähtiLoeti = 3+strlen(limiAlla)+1;
        free(limiAlla);
        tagastus.Tõlge = mälu;
        return tagastus;
    }
}



/* Murruga on lugu selline.
sin(x+y)/2  -->  (sin(x+y))/2, mitte sin((x+y)/2)
Sulgude omistamine käib vasakult paremale. Sin on enne murrujoont ja seega sulud on siinuse omad ja seega vähim lugeja on sin(x+y).
Küsimus on, kuidas seda tajuda.
MÕTE 1.  Saaks teha funktsiooni, mis ütleb kas tõlgitava teksti kohal [i] algab mingi murru lugeja. Sel juhul tõlgimathmode sees pannakse resulti \\frac{ ja antakse &tekst[i] funktsioonile tõlgifrac, mis loeb sisse lugeja ja nimetaja ja tagastab ülejäänud tõlke, mis on kujul a}{b}. Samas tõlgimathmodes astutakse vist ka nimetatud avaldises sin(x+y)/2 ka ( märgi peale ja sel puhul murru lugeja tajuja funktsioon ütleks ka selle kohta, et algab murd. 
MÕTE 2.  Veel üks võimalus oleks mitte tajuda ette, kas tekstikoht on mingi nimetaja, vaid rahulikult jõuda lugemisega lõpuks murrujooneni ja siis hakata uurima, mis oleks pidanud nimetaja olema. Ma arvan, et see on lugeja väljaselgitamise koha pealt veidike lihtsam, aga miinus on see, et siis peab juba kirjutatud resulti tagantjärele muutma niimoodi, et \frac{ oleks selle sees täpselt õiges kohas.*/


// Funktsioon tagastab mäluaadressi, millel on antud teksti esimene mittetühik. Kui juba esimene täht pole tühik, siis tagastab sellesama mäluaadressi, mis sisse anti. Tagastab aadressi NULL, kui teksti lõpuni ei olnud midagi muud peale tühikute.
char* TähtPealeTühikuid(const char* tekst)
{
    for (unsigned int i=0; i<strlen(tekst); i++)
    {
        if (tekst[i] != ' ')
        {
            return &tekst[i];
        }
    }
    return NULL;
}


char* LeiaNimetaja(const char* tekst) // nin(x)/sin(x + 4)abc     va(4 sin(x)x)/sin(x + 4)abc
{
    for (unsigned int i = 0; tekst[i]!='/';)
    {
        if (tekst[i] == ' ' || tekst[i]=='\0')
        {
            char* nimetajaMälu = malloc(i+1);
            memcpy(nimetajaMälu, tekst, i);
            nimetajaMälu[i] = '\0';
            return nimetajaMälu;
        }
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            unsigned int pikkus = strlen(sulusisu);
            free(sulusisu);
            i += pikkus+2;
        }
        else
        {
            i++;
        }
    }
}


int KasLugeja(const char* tekst) // nin(x)/sin(x + 4)abc     va(4 sin(x)x)/sin(x + 4)abc
{
    for (unsigned int i = 0; tekst[i]!='/';)
    {
        if (tekst[i] == ' ')
        {
            return 0;
        }

        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            unsigned int pikkus = strlen(sulusisu);
            free(sulusisu);
            i += pikkus+2;
        } else {
            i++;
        }
    }
    return 1;
    /*
    // Esimeseks kontrollib, ega / otse vaadeldava koha kõrval pole. Kõrvalolemist tuleb kontrollida nii, et tühikuid ei arvestataks. avaldises "a /b" on / a kõrval
    // Teiseks kontrollib, ega küsitaval kohal pole avanev sulg. Siis otsib vastava sulgeva sulu ja kui selle taga on /, siis on murd.
    if (*(TähtPealeTühikuid(&tekst[1])) == '/')
    {
        return 1;
    }
    if (tekst[0] == '(')
    {
        
        char* suluSisu = LeiaSuluSisu(&tekst[1]);
        unsigned int pikkus = strlen(suluSisu);
        if (*(TähtPealeTühikuid(&tekst[pikkus+2])) == '/')
        {
            return 1;
        }
    }
    // Kolmandaks kontrollib, ega tegu pole funktsiooniga, mille järel on sulud ja nende järel omakorda /. Näiteks avaldises 1+sin(x+b)/2 on sin(x+b) lugeja.
    for (unsigned int i=0; i<poolituskoht; i++)
    {
        if (KasEsimesedTähed(tekst, math_functions[i]))
        {
            if (tekst[strlen(math_functions[i])] == '(')
            {
                char* suluSisu = LeiaSuluSisu(&tekst[strlen(math_functions[i])+1]);
                unsigned int pikkus = strlen(suluSisu);
                if (*(TähtPealeTühikuid(&tekst[strlen(math_functions[i])+pikkus+2])) == '/') // trust.
                {
                    return 1;
                }
            }
        }
    }
    // Viimasena kontrollib ega kontrollitaval kohal pole funktsiooni, mille järel ei pea sulge olema nt alfa.
    for (unsigned int i=poolituskoht; math_functions[i]!= NULL; i++)
    {
        if (KasEsimesedTähed(tekst, math_functions[i]))
        {
            if (*(TähtPealeTühikuid(&tekst[strlen(math_functions[i])])) == '/')
            {
                    return 1;
            }
        }
    }
    return 0;
    */
}


/*Funktsioon võtab sisse murdu tähistava kaldkriipsu mäluaadressi tõlgitavas tekstis. Funktsioon vaatab sellest tagasipoole ja selgitab välja lugeja. Selleks, et kogemata liiga palju tagasi ei vaataks, on funktsioonil vaja ka teist argumenti, mis on tõlgitava teksti alguse mäluaadress. Funktsioon vaatab ka mälus edasipoole ja selgitab välja nimetaja. */


struct TekstArv TõlgiFrac(const char* tekst)
{
    // V/tta kõik alates algusest kuni / märgini.   va(4 sin(x)x)/sin(x + 4)abc   /sin(x)  sin(x+y)/2  
    char* lugeja = LeiaTekstEnneTeksti(tekst, "/");
    
}

/*
int KasOnLugeja(const char*)
{

}
*/


/* Funktsioon võtab argumendiks mäluaadressi, kust algab tekst, mille esimesed tähed on sum. Funktsioon loeb seda teksti, eraldab endale sobiva koguse mälu ja kirjutab sinna latexiks tõlgitud teksti. Funktsioon tagastab structi, milles on tõlke mälu aadress ja üks arv, mis ütleb, kui palju tõlgimathmode peab tälgitavas koodis edasi hüppama, et jõuda kohta, mis on TõlgiSum tõlgitud alast möödas.
Täpsematl töötab TõlgiSum järgmiselt. On mitu võimalust, mis saab olla esimene tähemärk sum tähtede järel.
1) sumi järel on avanev sulg. Siis sum paneb enda alaindeksiks selle sulu sisu. Kui avanevat sulgu sulgeva sulu järel on kohe veel üks avanev sulg, siis selle sisu läheb ülaindeksiks.
2) sumi järel on tühik. siis eeldab sum, et tahetakse kahte argumenti, mis on omavahel eraldatud tühikutega.

*/
//struct TekstArv TõlgiSum(const char* tekst)