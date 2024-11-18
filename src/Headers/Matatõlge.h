#ifndef Matatõlge_h
#define Matatõlge_h

struct LimiTagastus
{
    char* Tõlge;
    unsigned int TähtiLoeti;
};

struct TekstArv
{
    char* Tekst;
    unsigned int Arv;
};



/* Kasutaja on config.txt failis defineerinud käske. Iga käsu kirjutis tõlgitakse Käsu struktuuriks. Käsu struktuurid sisaldavad kõike vajalikku infot käsu kohta: käsu nimi, argumentide tüübid, argumentide nimed, definitsioon – täpsemalt kirjeldatud allpool. Kõigist config.txt failist saadud käsustructidest koostatakse nimekiri enne seda, kui peamine tälkimistsükkel käima läheb. Peamise tsükli käigus iga kord, kui liigutakse uuele tähele, siis käiakse üle loodud käsunimekirja ja kontrollitakse ega mõni käsk sellelt tähekohalt alga. Kui algab, siis kutsutakse välja TõlgiKäsk funktsioon, millele antakse avastatud käsu struct, et see teaks, kuidas õigesti tõlkida.
1) Käsu nimi on see, mida kasutaja tahab lähtekoodis kijrutada, et oma käsku välja kutsuda. 
2) Argumentide tüübid on nimekiri nii mitmest elemendist+1, kui palju kasutaja oma käsule definitsioonis andis. Kõige viimane element peab selles nimekrijas olema mingi negatiivne arv, märkimaks nimekirja lõppu. Ülejäänud elemendid saavad selles nimekirjas olla kas 0 või 1. 0 tähistab seda, kui järjekorras vastaval kohal olev argument on lühem: argumendi lõppu tähistab lähtekoodis tühik või suvaline tehtemärk. 1 tähistab seda, kui vastaval kohal olev argument on pikem: lähtekoodis lõpetab seda argumenti ainult tühik.
3) Argumentide nimed on nimekiri kõigist argumentide tähistustest, mida kasutaja oma argumentidele andis, et neid siduda oma käsu deklaratsiooni ja definitsiooni vahel.
4) Definitsioon on tekst, mis config.txt failis on -> märgi järel. See tekst on latexi kood, milles mõnes kohas on kasutaja argumentide tähist. */
struct Käsk
{
    const char* käsunimi;
    int* argumentideTüübid;
    const char** argumentideNimed;
    const char* definitsioon;
    unsigned int argumentideKogus;
};



#define MAX_REPLACEMENTS 100 

extern const char* math_functions_replace[MAX_REPLACEMENTS];
extern const char* math_functions_replace_tähendused[MAX_REPLACEMENTS];
extern int replacement_count;

int load_replacements(const char* config_path);

//char* read_line(FILE* file);
int KasEsimesedTähed(const char* tekstis, const char* tekst);
char* LeiaSuluSisu(const char* tekst);
char* LiidaTekstid(char* eelmineMälu, const char* lisatav);
char* my_strndup(const char* s, size_t n);
int is_math_function(const char* str);
char* append_str(const char* a, const char* b);
char* TõlgiMathMode(const char* expression);
char* LeiaTekstEnneTähte(const char* tekst, char täht);
struct LimiTagastus TõlgiLim(char* tekst);
char* LeiaTekstEnneTeksti(const char* tekst, const char* teksti);
int KasLugeja(const char* tekst);
char* LeiaNimetaja(const char* tekst);
struct TekstArv TõlgiAste(const char* tekst);
char* LeiaLugeja(const char* tekst);
char* EemaldaEsimeneViimane(char* tekst);
int KasAvaldiseÜmberOnSulud(const char* tekst);
char* LeiaArgument(const char* tekst);
char* LeiaLühemArgument(const char* tekst);
char* KõrvutiolevadAstmeks(const char* tekst);
char* LiidaTäht(char* eelmineMälu, char lisatav);
char* LiidaArv(char* eelmineMälu, int lisatav);
int kasnEelnevatOnTäht(unsigned int n, char täht);
char* TõlgiKäsk(const char* tekst, struct Käsk* käsk);
#endif