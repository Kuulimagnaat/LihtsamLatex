#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Abifunktsioonid.h"
#include "Headers/Kõigetõlge.h"




char* TõlgiKõik(char* tõlgitav)
{
    char* tõlge = malloc(1);
    tõlge[0] = '\0';


    unsigned int pikkus = strlen(tõlgitav);
    // Läheb üle iga tähe kogu tõlgitavas tekstis.
    for (unsigned int i = 0; i<pikkus; )
    {
        struct Environment* env = KasEnvironment(&tõlgitav[i]);
        if (env)
        {
            struct TekstArv envTõlge = TõlgiEnvironment(&tõlgitav[i], env, 0);
            tõlge = LiidaTekstid(tõlge, envTõlge.Tekst);
            i += envTõlge.Arv;
            free(envTõlge.Tekst);
        }
        else
        if ( i == 0 && KasEsimesedTähed(&tõlgitav[i], "mm ") || KasEsimesedTähed(&tõlgitav[i], " mm ") || KasEsimesedTähed(&tõlgitav[i], "\nmm ") || KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"))
        {
            int onDisplayMath = 0;
            if (KasEsimesedTähed(&tõlgitav[i], " mm "))
            {
                tõlge = LiidaTekstid(tõlge, " $");
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "mm "))
            {
                tõlge = LiidaTekstid(tõlge, "$");
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\[\n");
                onDisplayMath = 1;
            }
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm "))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\[");
                onDisplayMath = 1;
            }
            i += (KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n") || KasEsimesedTähed(&tõlgitav[i], "\nmm ") || KasEsimesedTähed(&tõlgitav[i], " mm ") ? 4 : 3);

            unsigned int start = i;
            while (i < pikkus && !(KasEsimesedTähed(&tõlgitav[i], " mm") || KasEsimesedTähed(&tõlgitav[i], "\nmm"))) {
                i++;
            }

            char* tõlgitavMata = VõtaTekstIndeksini(&tõlgitav[start], i - start);
            char* mataTõlge = TõlgiMathMode(tõlgitavMata);
            tõlge = LiidaTekstid(tõlge, mataTõlge);

            free(mataTõlge);
            free(tõlgitavMata);

            if (onDisplayMath == 0) {
                tõlge = LiidaTekstid(tõlge, "$");
            } else {
                tõlge = LiidaTekstid(tõlge, "\\]");
            }

            i += 3;
        }
        else 
        {
            char* täht = malloc(2);
            täht[0] = tõlgitav[i];
            täht[1] = '\0';
            tõlge = LiidaTekstid(tõlge, täht);
            free(täht);
            i++;
        }
    }
    
    return tõlge;
}