#ifndef Kõigetõlge_h
#define Kõigetõlge_h
#include "Matatõlge.h"


struct TextmodeKäsk
{
    const char* käsualgus;
    const char* käsulõpp;
    const char** argumentideNimed;
    char* definitsioon;
    unsigned int argumentideKogus;
};

struct TextmodeKäskList
{
    struct TextmodeKäsk* käsud; // käskude nimekiri
    size_t kogus;         // kui palju hetkel nimekrijas käskusid on
    size_t maht;      // kui mitu käsku mahub nimekrija ilma et peaks uut mälu eraldada.
};

void init_TextModeKäsk_list();
void read_textModeKäsud_from_config(const char* filepath);

char* TõlgiKõik(char* tõlgitav);



#endif