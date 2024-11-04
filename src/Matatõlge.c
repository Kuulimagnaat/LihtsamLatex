#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <math.h>


// Funktsioon, mis kontorllib, kas antud teksti algus täpselt on sama, mis teine soovitud tekst. Tagastab 0 kui ei ole ja 1 kui on. Funktsiooni kasutusolukord: kui ollakse minemas tõlgitavas koodis üle tähtede, siis on vaja kontrollida, kas kättejõudnud kohas on mõne käsu nimi. Seda funktsiooni saab nimetatud olukorra tajumiseks kasutada.
#define KasEsimesedTähedDebug 0
int KasEsimesedTähed(const char* tekstis, const char* tekst)
{
    #if KasEsimesedTähedDebug == 1
    puts("KasEsimesedTähed");
    printf("  SISSE: %s, %s\n", tekstis, tekst);
    #endif

    unsigned int tähtiKontrollida = strlen(tekst);
    for (unsigned int i = 0; i<tähtiKontrollida; i++)
    {
        if (tekstis[i] != tekst[i])
        {
            #if KasEsimesedTähedDebug == 1
            puts("  VÄLJA: 0");
            #endif
            return 0;
        }
    }

    #if KasEsimesedTähedDebug == 1
    puts("  VÄLJA: 1");
    #endif
    return 1;
}



#define TõlgiAsteDebug 1
struct TekstArv TõlgiAste(const char* tekst)
{
    #if TõlgiAsteDebug == 1
    printf("TõlgiAste\n");
    printf("  SISSE: %s\n", tekst);
    #endif
    char* argument = LeiaLühemArgument(&tekst[1]);
    unsigned int argumendiPikkus = strlen(argument);
    char* sulgudetaArgument = NULL;
    if (KasAvaldiseÜmberOnSulud(argument))
    {
        sulgudetaArgument = EemaldaEsimeneViimane(argument);
        free(argument);
    }
    else
    {
        sulgudetaArgument = argument;
    }
    char* argumendiTõlge = TõlgiMathMode(sulgudetaArgument);
    free(sulgudetaArgument);

    char* tõlge = malloc(1);
    tõlge[0] = '\0';
    tõlge = LiidaTekstid(tõlge, "^{");
    tõlge = LiidaTekstid(tõlge, argumendiTõlge);
    free(argumendiTõlge);
    tõlge = LiidaTekstid(tõlge, "}");

    struct TekstArv tagastus = {.Tekst = tõlge, .Arv = argumendiPikkus};
    #if TõlgiAsteDebug == 1
    printf("  VÄLJA: %s, %d\n", tagastus.Tekst, tagastus.Arv);
    #endif
    return tagastus;
}



// SEDA FUNKTSIOONI VÕIB USALDADA: Käsitsi läbi katsetatud.
// Funktsiooni kasutatakse juhul, kui jõutakse tõlgitavas koodis funktsioonini, mille järel on avanev sulg. Siis antakse sellele funktsioonile avanevale sulule järgneva tähe mäluaadress, misjärel funktsioon leiab kogu teksti, mis peaks avaneva ja sulgeva sulu vahele jääma. Funktsioon eraldab mälu, täidab selle sulu sisuga ja tagastab selle mälu aadressi. See mälu on vaja hiljem vabastada.
#define LeiaSuluSisuDebug 0
char* LeiaSuluSisu(const char* tekst)
{
    #if LeiaSuluSisuDebug == 1
    puts("LeiaSuluSisu");
    printf("  SISSE: %s\n", tekst);
    #endif
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
                #if LeiaSuluSisuDebug == 1
                printf("  VÄLJA: %s\n", mälu);
                #endif
                return mälu;
            }
        }
        mälu[i] = tekst[i];
    }
    perror("Ei leitud sulgevat sulgu :(\n");
    exit(EXIT_FAILURE);
}



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



char* LiidaTäht(char* eelmineMälu, char lisatav)
{
    unsigned int pikkus = strlen(eelmineMälu);
    char* uusMälu = realloc(eelmineMälu, pikkus+2);
    uusMälu[pikkus] = lisatav;
    uusMälu[pikkus+1] = '\0';
    return uusMälu;
}



// Funktsioon lisab teksti lõppu mingi arvu. Ei tööta negatiivsete arvudega, aga ma tegingi selle esmalt ühe alati positiivse counteri numbri teksti lisamiseks. Vabastab eelmise mälu ja tagastab uue mälu aadressi, kuhu on kijutatud kogu eelmise tekst, aga lõpus on veel soovitud number. 
char* LiidaArv(char* eelmineMälu, int lisatav)
{
    unsigned int numbriteKogus = log10(lisatav)+1;
    unsigned int eelmisePikkus = strlen(eelmineMälu);
    char* uusMälu = realloc(eelmineMälu, eelmisePikkus + numbriteKogus + 1);
    sprintf(&uusMälu[eelmisePikkus], "%d", lisatav);
    return uusMälu;
}



// mingi list tuntud funktsioonidest
unsigned int poolituskoht = 6;
const char* math_functions[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "fii", "roo", "alfa", "beeta", "epsilon", "delta", "to", "inf", "lim", NULL};
const char* math_functions_tähendused[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "varphi", "rho", "alpha", "beta", "varepsilon", "delta", "to", "infty", "lim", NULL};



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



// Võtab sisse mäluaadressi, kust algab mingi tekst. Eraldab mälu ja kirjutab sinna selle teksti ainult ilma esimese ja viimase täheta. Mõeldud selleks, et avaldiselt kujul (xxx) sulud ära võtta.
#define EemaldaEsimeneViimaneDebug 0
char* EemaldaEsimeneViimane(char* tekst)
{
#if EemaldaEsimeneViimaneDebug == 1
    printf("EemaldaEsimeneViimane\n");
    printf("  SISSE: %s.\n", tekst);
#endif
    unsigned int l = strlen(tekst);
    char* lühem = malloc(l-1);
    memcpy(lühem, &tekst[1], l-2);
    lühem[l-2] = '\0';

#if EemaldaEsimeneViimaneDebug == 1
    printf("  VÄLJA: %s.\n", lühem);
#endif

    return lühem;
}



// Selleks, et teada saada, kas avaldis on ümbritsetud mõttetute sulgudega, ei piisa kontrollimast, kas esimene täht on ( ja viimane ), sest avaldis võib olla näiteks (a+b)*(c+d). Tõepoolest, esimene täht on ( ja viimane ), aga need sulud pole avaldist ümbritsevad sulud.
#define KasAvaldiseÜmberOnSuludDebug 0
int KasAvaldiseÜmberOnSulud(const char* tekst)
{
    #if KasAvaldiseÜmberOnSuludDebug == 1
    printf("KasAvaldiseÜmberOnSulud\n");
    printf("  SISSE: %s\n", tekst);
    #endif
    if (tekst[0] != '(')
    {
        #if KasAvaldiseÜmberOnSuludDebug == 1
        printf("  VÄLJA: 0");
        #endif
        return 0;
    }
    // Järelikult esimene täht on (.
    char* sulusisu = LeiaSuluSisu(&tekst[1]);
    // Kontrollin, kas esimest sulgu sulgev sulg on teksti viimane täht.
    if (&tekst[strlen(sulusisu)+1] == &tekst[strlen(tekst)-1])
    {
        #if KasAvaldiseÜmberOnSuludDebug == 1
        printf("  VÄLJA: 1");
        #endif
        return 1;
    }
    #if KasAvaldiseÜmberOnSuludDebug == 1
    printf("  VÄLJA: 0");
    #endif
    return 0;
}


// Funktsioon, mis on mõeldud funktsiooni tul argumendi lihtsustamiseks. See peab kaks kõrvutiolevat sama tähte asendama selle tähe ruuduga, kolm kõrvutiolevat sama tähte selle tähe kuubiga jne. Näiteks xxxyy -> x^{3}y^{2} ja xxyxy -> x^{2}yxy
char* KõrvutiolevadAstmeks(const char* tekst)
{
    char* tõlge = malloc(1);
    tõlge[0] = '\0';
    unsigned int aste = 1;
    for (unsigned int i = 0; tekst[i]!='\0'; i++)
    {
        if (tekst[i+1] == tekst[i])
        {
            aste++;
            printf("%d\n", aste);
            puts("Astet kasvatati!");
            continue;
        }
        else
        {
            if (aste == 1)
            {
                tõlge = LiidaTäht(tõlge, tekst[i]);
            }
            else
            {
                //puts("Programm jõudis ^{ lisamise juurde!");
                tõlge = LiidaTäht(tõlge, tekst[i]);
                tõlge = LiidaTekstid(tõlge, "^{");
                tõlge = LiidaArv(tõlge, aste);

                tõlge = LiidaTekstid(tõlge, "}");
                aste = 1;
            }
        }
    }
    return tõlge;
}



// Rekursiivselt tõlgime math moodi latexisse
#define TõlgiMathModeDebug 1
char* TõlgiMathMode(const char* expression)
{
    #if TõlgiMathModeDebug == 1
    printf("TõlgiMathMode\n");
    printf("  SISSE: %s\n", expression);
    #endif
    //puts(expression);
    char* result = malloc(1); // Tühi string
    result[0] = '\0';
    
    int i = 0;
    while (i < strlen(expression)) 
    {
        if (KasLugeja(&expression[i]) == 1)
        {
            char* lugeja = LeiaLugeja(&expression[i]);
            int lugejaOnSulgudeta = 1;
            char* sulgudetaLugeja;
            if (KasAvaldiseÜmberOnSulud(lugeja))
            {
                lugejaOnSulgudeta = 0;
                sulgudetaLugeja = EemaldaEsimeneViimane(lugeja);
            }
            // Siin kui sulgusid ei ole, mida eemaldada lugjealt enne selle tõlkimist, siis tahaksin panna sulgudetaLugeja väärtuseks sellesama lugeja mäluaadressi. See on probleemne, sest hiljem kui vabastatakse lugeja ja ss sulgudetaLugeja, ss vabastatakse sama mäluaadressi mälu 2 korda. Selle vältimiseks tegin muutuja lugejaOnSulgudeta, mis omab väärtust 0, kui lugeja on erinev sulgudeta lugejast ja 1, kui lugeja ja sulgudetaLugeja tekstid omavad sama mäluaadressi. Kui on 1, ss mälu vabastatakse ainult ühe korra.
            else
            {
                sulgudetaLugeja = lugeja;
            }
                
            char* lugejaTõlge = TõlgiMathMode(sulgudetaLugeja);
            char* nimetaja = LeiaNimetaja(&expression[i+strlen(lugeja)+1]);
            char* nimetajaTõlge = TõlgiMathMode(nimetaja);
            result = LiidaTekstid(result, "\\frac{");
            result = LiidaTekstid(result, lugejaTõlge);
            result = LiidaTekstid(result, "}{");
            result = LiidaTekstid(result, nimetajaTõlge);
            result = LiidaTekstid(result, "}");
            i += strlen(lugeja)+1+strlen(nimetaja);

            free(lugeja);
            //printf("Kas lugeja on sulgudeta: %d\n", lugejaOnSulgudeta);
            // Kui lugejaOnSulgudeta on väärtusega 1, siis on lugeja ja sulgudetaLugeja samal mäluaadressil, mistõttu ei tohi sulgudetaLugjejat vabastada, sest ss vabastaks sama mälu kaks korda. SulgudetaLugeja tuleb vabastada ainult siis, kui lugejaOnSulgudeta väärtus on 0, st esialgne lugeja on kujul (xxx) ja sulgudetalugeja jaoks loodi uus mälu, kus on tekst kujul xxx. 
            if (lugejaOnSulgudeta == 0)
            {
                free(sulgudetaLugeja);
            }
            //printf("Result on järgmine: %s\n", result);
            free(nimetaja);
            free(lugejaTõlge);
            free(nimetajaTõlge);
            continue;
        }
        if (expression[i] == '^')
        {
            struct TekstArv astmeTõlge = TõlgiAste(&expression[i]);
            result = LiidaTekstid(result, astmeTõlge.Tekst);
            free(astmeTõlge.Tekst);
            i += astmeTõlge.Arv;
            continue;
        }

        if (KasEsimesedTähed(&expression[i], "tul"))
        {
            char* tõlge = malloc(1);
            tõlge[0] = '\0';

            char* argument = LeiaLühemArgument(&expression[i+3]);
            // argumendi xxy puhul peab tõlge olema '''_{xxy}
            char* alatekst = KõrvutiolevadAstmeks(argument);
            for (unsigned int j = 0; j<strlen(argument); j++)
            {
                tõlge = LiidaTäht(tõlge, '\'');
            }
            tõlge = LiidaTekstid(tõlge, "_{");
            tõlge = LiidaTekstid(tõlge, alatekst);
            tõlge = LiidaTekstid(tõlge, "}");

            i += 3+strlen(argument);
            result = LiidaTekstid(result, tõlge);
            free(argument);
            free(alatekst);
            free(tõlge);
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
            //printf("Resulti lisatakse tavaline täht %s.\n", single_char);
            result = append_str(result, single_char);
            free(single_char);
            i++;
        }
    }
    #if TõlgiMathModeDebug == 1
    printf("  VÄLJA: %s\n", result);
    #endif
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
3) limi järel on mingi muu täht. Siis läheb limi alla kõik see, mis jääb limi ja esimese tühiku vahele. limlima/btoc (a+b)/c+d*/
struct LimiTagastus TõlgiLim(char* tekst)
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



#define LeiaNimetajaDebug 0
char* LeiaNimetaja(const char* tekst) // sin(x)/sin(x + 4)abc     va(4 sin(x)x)/sin(x + 4)abc
{
    #if LeiaNimetajaDebug == 1
    printf("LeiaNimetaja\n");
    printf("  SISSE: %s.\n", tekst);
    #endif
    for (unsigned int i = 0; tekst[i]!='/';)
    {
        if (tekst[i] == ' ' || tekst[i]=='\0')
        {
            char* nimetajaMälu = malloc(i+1);
            memcpy(nimetajaMälu, tekst, i);
            nimetajaMälu[i] = '\0';

            #if LeiaNimetajaDebug == 1
            printf("  VÄLJA: %s.\n", nimetajaMälu);
            #endif
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



#define LeiaLugejaDebug 0
char* LeiaLugeja(const char* tekst)
{
    #if LeiaLugejaDebug == 1
    printf("LeiaLugeja\n");
    printf("  SISSE: %s\n", tekst);
    #endif

    unsigned int i=0;
    for ( ; tekst[i] != '/'; )
    {
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            unsigned int sulupikkus = strlen(sulusisu);
            free(sulusisu);
            i+=sulupikkus+2;
            continue;
        }
        i++;
    }
    char* lugeja = malloc(i+1);
    memcpy(lugeja, tekst, i+1);
    lugeja[i] = '\0';
    #if LeiaLugejaDebug == 1
    printf("  VÄLJA: %s\n", lugeja);
    #endif
    return lugeja;
}



#define KasLugejaDebug 0
int KasLugeja(const char* tekst) // nin(x)/sin(x + 4)abc     va(4 sin(x)x)/sin(x + 4)abc
{
    #if KasLugejaDebug == 1
    puts("KasLugeja");
    printf("  SISSE: %s\n", tekst);
    #endif
    for (unsigned int i = 0; tekst[i]!='/';)
    {
        if(tekst[i]=='\0')
        {
            #if KasLugejaDebug == 1
            puts("  VÄLJA: 0");
            #endif
            return 0;
        }
        else if (tekst[i] == ' ')
        {
            #if KasLugejaDebug == 1
            puts("  VÄLJA: 0");
            #endif
            return 0;
        }
        else if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            unsigned int pikkus = strlen(sulusisu);
            free(sulusisu);
            i += pikkus+2;
        } else {
            i++;
        }
    }
    #if KasLugejaDebug == 1
    puts("  VÄLJA: 1");
    #endif
    return 1;
}



// Argumendi võtmine käib nii, et tagastatakse tekst enne esimest tühikut välja arvatud juhul kui see tühik on mingite sulgude sees. See ei ole aga ainus viis argumenti võtta, sest näitekst astmed tahavad veidi teistsugust argumendivõtmist. Teksti ax^2+bx+c peab tõlgendama nii, et ax^(2)+bx+c, mitte ax^(2+bx+c) nagu esimene variant tõlgendaks. Seega on olemas kaks argumendivõtmise funtksiooni. Üks on tavaline LeiaArgument, teine on LeiaLühemArgument selline, mis lõpetab argumendivõtu mitte ainult tühiku peale vaid ka tehete +-* peale. Jagamismärki selles nimekirjas pole, sest jagamise argumendid selgitatakse välja enne astmete omi, mistõttu jagamismärki ei tule kunagi astme argumendi otsimisel ette.
#define LeiaArgumentDebug 0
char* LeiaArgument(const char* tekst)
{
    #if LeiaArgumentDebug == 1
    printf("LeiaArgument\n");
    printf("  SISSE: %s\n", tekst);
    #endif
    for (unsigned int i=0; ; )
    {
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            i += strlen(sulusisu)+2;
            free(sulusisu);
            continue;
        }
        if  (tekst[i] == ' ' || tekst[i]=='\0')
        {
            char* argument = malloc(i+1);
            memcpy(argument, tekst, i);
            argument[i]='\0';

            #if LeiaArgumentDebug == 1
            printf("  VÄLJA: %s\n", a.Tekst);
            #endif
            return argument;
        }
        i++;
    }
}

// Identne eelmise funktsooniga, lic argumendi võtmise lõpetamise tingimusi on rohkem.
#define LeiaLühemArgumentDebug 0
char* LeiaLühemArgument(const char* tekst)
{
    #if LeiaLühemArgumentDebug == 1
    printf("LeiaLühemArgument\n");
    printf("  SISSE: %s\n", tekst);
    #endif
    for (unsigned int i=0; ; )
    {
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            i += strlen(sulusisu)+2;
            free(sulusisu);
            continue;
        }
        if  (tekst[i] == '+' || tekst[i] == '-' || tekst[i] == '*' || tekst[i] == ' ' || tekst[i]=='\0')
        {
            char* argument = malloc(i+1);
            memcpy(argument, tekst, i);
            argument[i]='\0';

            #if LeiaLühemArgumentDebug == 1
            printf("  VÄLJA: %s\n", argument);
            #endif
            return argument;
        }
        i++;
    }
}