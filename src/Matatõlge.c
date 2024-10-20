#include "Headers/Matatõlge.h"
#include <string.h>


// Funktsioon, mis kontorllib, kas antud tekst algus täpselt on sama, mis teine soovitud tekst. Tagastab 0 kui ei ole ja 1 kui on. Kui ollakse minemas üle tähtede, siis on vaja kontrollida, kas kättejõudnud kohas on funktsiooni nimi. Seda funktsiooni saab nimetatud olukorra tajumiseks kasutada.
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


// Funkstioon võtab argumendiks mäluaadressi, kus on math mode'i tekst. See hakkab sealt teksti lugema ja tõlkima seda latexi koodiks. Funktsioon eraldab omale sobiva koguse mälu ja kirjutab sinna latexiks tõlgitud teksti. Tagastatakse selle mälu aadress, kus on tõlge. Kunagi pärast selle funktsiooni kasutamist on vaja selle tagastatud mäluaadressi mälu vabastada. 
char* TõlgiMathMode(const char* tekst)
{
    unsigned int suluTase = 0;

    size_t pikkus = strlen(tekst);
    for (unsigned int i = 0; i<pikkus, i++)
    {
        
    }
}