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
            struct TekstArv envTõlge = TõlgiEnvironment(&tõlgitav[i], env);
            tõlge = LiidaTekstid(tõlge, envTõlge.Tekst);
            free(envTõlge.Tekst);
            i += envTõlge.Arv;
        }
        else
        if ( i == 0 && KasEsimesedTähed(&tõlgitav[i], "mm ") || KasEsimesedTähed(&tõlgitav[i], " mm ") || KasEsimesedTähed(&tõlgitav[i], "\nmm "))
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
            else if (KasEsimesedTähed(&tõlgitav[i], "\nmm "))
            {
                tõlge = LiidaTekstid(tõlge, "\n\\[ ");
                onDisplayMath = 1;
            }
            i+=4;
            char* tõlgitavMata = NULL;
            for (unsigned int j = i; j < pikkus; j++)
            {
                if (KasEsimesedTähed(&tõlgitav[j], " mm") || KasEsimesedTähed(&tõlgitav[j], "\nmm"))
                {
                    char* tõlgitavMata = VõtaTekstIndeksini(&tõlgitav[i], j-i);

                    char* mataTõlge = TõlgiMathMode(tõlgitavMata);
                    tõlge = LiidaTekstid(tõlge, mataTõlge);
                    free(mataTõlge);
                    

                    i += j-i;
                    if (onDisplayMath == 0)
                    {
                        tõlge = LiidaTekstid(tõlge, "$");
                    }
                    else if(onDisplayMath == 1)
                    {
                        tõlge = LiidaTekstid(tõlge, "\\]");
                    }

                    i += 3;
                }
                
            }
            
            

            
            free (tõlgitavMata);
            
            
        }


        else
        {
            char* täht = malloc(2);
            täht[0] = tõlgitav[i];
            täht[1] = '\0';
            tõlge = LiidaTekstid(tõlge, täht);
            i++;
        }
    }
    
    return tõlge;
}