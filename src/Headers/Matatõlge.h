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


int KasEsimesedTähed(const char* tekstis, const char* tekst);
char* LeiaSuluSisu(const char* tekst);
char* LiidaTekstid(char* eelmineMälu, const char* lisatav);
char* my_strndup(const char* s, size_t n);
int is_math_function(const char* str);
char* append_str(const char* a, const char* b);
char* TõlgiMathMode(const char* expression);
char* LeiaTekstEnneTähte(const char* tekst, char täht);
struct LimiTagastus TõlgiLim(const char* tekst);
char* LeiaTekstEnneTeksti(const char* tekst, const char* teksti);
int KasLugeja(const char* tekst);
char* LeiaNimetaja(const char* tekst);
struct TekstArv TõlgiAste(const char* tekst);
char* LeiaLugeja(const char* tekst);

#endif