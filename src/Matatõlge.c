#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <math.h>
#include <ctype.h>

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
const char* math_functions[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "lim", NULL};
const char* math_functions_tähendused[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "lim", NULL};

const char* math_functions_replace[MAX_REPLACEMENTS];
const char* math_functions_replace_tähendused[MAX_REPLACEMENTS];
int replacement_count = 0;

// Function to trim whitespace from a string
char* trim_whitespace(char* str) {
    while (isspace((unsigned char)*str)) str++;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

int load_replacements(const char* config_path) {
    FILE* config_file = fopen(config_path, "r");
    if (config_file == NULL) {
        perror("Unable to open config.txt in src directory");
        return -1;
    }

    char* line;
    while ((line = read_line(config_file)) != NULL) {
        // Check for lines with the replacement pattern
        char* arrow = strstr(line, "->");
        if (arrow) {
            *arrow = '\0';  // Null-terminate the first part to isolate the command
            char* command = trim_whitespace(line);  // This points to the command part
             char* translation = trim_whitespace(arrow + 2);  // Move past the arrow to get the translation

            if (replacement_count < MAX_REPLACEMENTS) {
                math_functions_replace[replacement_count] = strdup(command);
                math_functions_replace_tähendused[replacement_count] = strdup(translation);
                replacement_count++;
            } else {
                fprintf(stderr, "Warning: Max replacement limit reached!\n");
            }
        }
        free(line);
    }
    fclose(config_file);
    return 0;  // Success
}

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


/* Kasutaja on config.txt failis defineerinud käske. Iga käsu kirjutis tõlgitakse Käsu struktuuriks. Käsu struktuurid sisaldavad kõike vajalikku infot käsu kohta: käsu nimi, argumentide tüübid, argumentide nimed, definitsioon – täpsemalt kirjeldatud allpool. Kõigist config.txt failist saadud käsustructidest koostatakse nimekiri enne seda, kui peamine tälkimistsükkel käima läheb. Peamise tsükli käigus iga kord, kui liigutakse uuele tähele, siis käiakse üle loodud käsunimekirja ja kontrollitakse ega mõni käsk sellelt tähekohalt alga. Kui algab, siis kutsutakse välja TõlgiKäsk funktsioon, millele antakse avastatud käsu struct, et see teaks, kuidas õigesti tõlkida.
1) Käsu nimi on see, mida kasutaja tahab lähtekoodis kijrutada, et oma käsku välja kutsuda. 
2) Argumentide tüübid on nimekiri nii mitmest elemendist+1, kui palju kasutaja oma käsule definitsioonis andis. Kõige viimane element peab selles nimekrijas olema mingi negatiivne arv, märkimaks nimekirja lõppu. Ülejäänud elemendid saavad selles nimekirjas olla kas 0 või 1. 0 tähistab seda, kui järjekorras vastaval kohal olev argument on lühem: argumendi lõppu tähistab lähtekoodis tühik või suvaline tehtemärk. 1 tähistab seda, kui vastaval kohal olev argument on pikem: lähtekoodis lõpetab seda argumenti ainult tühik.
3) Argumentide nimed on nimekiri kõigist argumentide tähistustest, mida kasutaja oma argumentidele andis, et neid siduda oma käsu deklaratsiooni ja definitsiooni vahel.
4) Definitsioon on tekst, mis config.txt failis on -> märgi järel. See tekst on latexi kood, milles mõnes kohas on kasutaja argumentide tähist. */
struct Käsk
{
    const char* käsunimi;
    int* argumentideTüübid; // Selle nimekirja viimane element on mingi neg arv näitamaks nimekirja lõppu.
    const char** argumentideNimed;
    const char* definitsioon;
};


// Mõte on, et kui TõlgiMathMode funktsioonis tajutakse, et kättejõudnud kohal on mingi käsk, siis antakse sellesama sellele funktsioonile ja 
char* TõlgiKäsk(const char* tekst, struct Käsk* käsk)
{

    for (unsigned int i=0; käsk->argumentideTüübid[i] >= 0; i++)
    {
        ;
    }
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



// Funktsioon on mõeldud kasutamiseks 
int kasnEelnevatOnTäht(unsigned int n, char täht);



// Rekursiivselt tõlgime math moodi latexisse
#define TõlgiMathModeDebug 1
char* TõlgiMathMode(const char* expression) {
    #if TõlgiMathModeDebug == 1
    printf("TõlgiMathMode\n");
    printf("  SISSE: %s\n", expression);
    #endif

    char* result = malloc(1); // Empty string
    result[0] = '\0';

    int i = 0;
    while (i < strlen(expression)) {
        // Check for a number or variable
        if (KasLugeja(&expression[i]) == 1) {
            char* lugeja = LeiaLugeja(&expression[i]);
            int lugejaOnSulgudeta = 1;
            char* sulgudetaLugeja;

            // Check if the reader has parentheses
            if (KasAvaldiseÜmberOnSulud(lugeja)) {
                lugejaOnSulgudeta = 0;
                sulgudetaLugeja = EemaldaEsimeneViimane(lugeja);
            } else {
                sulgudetaLugeja = lugeja;
            }

            char* lugejaTõlge = TõlgiMathMode(sulgudetaLugeja);
            char* nimetaja = LeiaNimetaja(&expression[i + strlen(lugeja) + 1]);
            char* nimetajaTõlge = TõlgiMathMode(nimetaja);

            result = LiidaTekstid(result, "\\frac{");
            result = LiidaTekstid(result, lugejaTõlge);
            result = LiidaTekstid(result, "}{");
            result = LiidaTekstid(result, nimetajaTõlge);
            result = LiidaTekstid(result, "}");

            // Update the index
            i += strlen(lugeja) + 1 + strlen(nimetaja);

            // Free memory to prevent leaks
            free(lugeja);
            if (lugejaOnSulgudeta == 0) {
                free(sulgudetaLugeja);
            }
            free(nimetaja);
            free(lugejaTõlge);
            free(nimetajaTõlge);
            continue;
        }

        // Check for exponent
        if (expression[i] == '^') {
            struct TekstArv astmeTõlge = TõlgiAste(&expression[i]);
            result = LiidaTekstid(result, astmeTõlge.Tekst);
            free(astmeTõlge.Tekst);
            i += astmeTõlge.Arv;
            continue;
        }

        // Check for 'tul' commands
        if (KasEsimesedTähed(&expression[i], "tul")) {
            char* tõlge = malloc(1);
            tõlge[0] = '\0';

            char* argument = LeiaLühemArgument(&expression[i + 3]);
            // In the case of argument 'xxy', the translation must be '''_{xxy}
            char* alatekst = KõrvutiolevadAstmeks(argument);
            for (unsigned int j = 0; j < strlen(argument); j++) {
                tõlge = LiidaTäht(tõlge, '\'');
            }
            tõlge = LiidaTekstid(tõlge, "_{");
            tõlge = LiidaTekstid(tõlge, alatekst);
            tõlge = LiidaTekstid(tõlge, "}");

            i += 3 + strlen(argument);
            result = LiidaTekstid(result, tõlge);

            // Free allocated memory
            free(argument);
            free(alatekst);
            free(tõlge);
            continue;
        }

        // Check for known commands in one go
        int func_index = -1;
        int func_len = 0;
        int is_replacement = 0; // Flag to check if it's a replacement command

        // Check in math functions array
        for (int j = 0; math_functions[j] != NULL; j++) {
            if (strncmp(&expression[i], math_functions[j], strlen(math_functions[j])) == 0) {
                func_index = j;
                func_len = strlen(math_functions[j]);
                is_replacement = 0;
                break;
            }
        }

        // If not found in math functions, check in replacement array
        if (func_index == -1) {
            for (int j = 0; math_functions_replace[j] != NULL; j++) {
                if (strncmp(&expression[i], math_functions_replace[j], strlen(math_functions_replace[j])) == 0) {
                    func_index = j; // Keep the index for replacements
                    func_len = strlen(math_functions_replace[j]);
                    is_replacement = 1; // Found in replacements
                    break;
                }
            }
        }

        // If a function or replacement is found
        if (func_index != -1) {
            char* func_name;

            // Determine the function or replacement name
            if (is_replacement) {
                func_name = my_strndup(math_functions_replace_tähendused[func_index], strlen(math_functions_replace_tähendused[func_index]));
                result = LiidaTekstid(result, append_str("\\", func_name));
                i += func_len;
            } else {
                func_name = my_strndup(math_functions_tähendused[func_index], strlen(math_functions_tähendused[func_index]));
                
                // Handle standard math functions
                if (strcmp(func_name, "lim") == 0) {
                    char* inner_expression = my_strndup(&expression[i], strlen(&expression[i]));
                    struct LimiTagastus tagastus = TõlgiLim(inner_expression);
                    result = LiidaTekstid(result, tagastus.Tõlge);
                    i += tagastus.TähtiLoeti; // Move index forward
                    free(inner_expression); // Clean up
                } else {
                    // Add LaTeX function format
                    result = LiidaTekstid(result, append_str("\\", append_str(func_name, "\\left(")));
                    i += func_len; // Move past function
                }
            }

            free(func_name); // Clean up allocated memory

            // Check for parentheses if it's a regular function
            if (!is_replacement && expression[i] == '(') {
                puts("läks sisse");
                int start = i + 1;
                int paren_count = 1;
                i++;

                while (expression[i] != '\0' && paren_count > 0) {
                    if (expression[i] == '(') paren_count++;
                    else if (expression[i] == ')') paren_count--;
                    i++;
                }

                // Process inner expression recursively
                char* inner_expression = my_strndup(&expression[start], i - start - 1);
                char* inner_latex = TõlgiMathMode(inner_expression);

                result = LiidaTekstid(result, inner_latex);
                result = append_str(result, "\\right)");

                free(inner_expression);
                free(inner_latex);
            }
        } else {
            // Handle regular characters
            char* single_char = my_strndup(&expression[i], 1);
            result = LiidaTekstid(result, single_char);
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