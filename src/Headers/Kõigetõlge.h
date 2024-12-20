#ifndef Kõigetõlge_h
#define Kõigetõlge_h
#include "Matatõlge.h"


struct TextmodeKäsk
{
    const char* käsualgus;
    const char** argumentideNimed;
    const char** argumentideLõpud;
    char* definitsioon;
    unsigned int argumentideKogus;
};

struct TextmodeKäskList
{
    struct TextmodeKäsk* käsud; // käskude nimekiri
    size_t kogus;         // kui palju hetkel nimekrijas käskusid on
    size_t maht;      // kui mitu käsku mahub nimekrija ilma et peaks uut mälu eraldada.
};

void TextmodeKäsudConfigist(char* config_path);
struct TextmodeKäsk* KasTextmodeKäsk(char* tekst);
char* TõlgiKõik(char* tõlgitav);
struct TekstArv TõlgiTextmodeKäsk(char* tekst, struct TextmodeKäsk* käsk);



#endif