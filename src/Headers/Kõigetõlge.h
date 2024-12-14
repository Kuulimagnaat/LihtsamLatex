#ifndef Kõigetõlge_h
#define Kõigetõlge_h
#include "Matatõlge.h"


struct TextModeKäsk
{
    const char* käsualgus;
    const char* käsulõpp;
    const char** argumentideNimed;
    char* definitsioon;
    unsigned int argumentideKogus;
};

struct TextModeKäskList
{
    struct TextModeKäsk* käsud;   // Array of Käsk
    size_t count;         // Number of commands
    size_t capacity;      // Array capacity
};

void init_TextModeKäsk_list();
void read_textModeKäsud_from_config(const char* filepath);

char* TõlgiKõik(char* tõlgitav);



#endif