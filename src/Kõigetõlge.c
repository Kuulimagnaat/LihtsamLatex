#include <stdio.h>
//#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"

char* TõlgiKõik(char* tõlgitav)
{
    unsigned int pikkus = strlen(tõlgitav);
    // Läheb üle iga tähe kogu tõlgitavas tekstis.
    for (unsigned int i = 0; i<pikkus; i++)
    {
        if (KasEnvironment(&tõlgitav[i]))
        {

        }
    }
}