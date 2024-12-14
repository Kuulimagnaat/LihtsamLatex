#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Abifunktsioonid.h"
#include "Headers/Kõigetõlge.h"


extern struct TextModeKäskList textModeKäskList;


// Initialize a TextModeKäskList
void init_TextModeKäsk_list() {
    textModeKäskList.käsud = malloc(1 * sizeof(struct Käsk)); // -> on sama mis (*list).käsud
    textModeKäskList.count = 0;
    textModeKäskList.capacity = 1;
}


void read_textModeKäsud_from_config(char* config_path)
{
    FILE* file = fopen(config_path, "r");
    if (!file) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    char* line;
    int onMathmodeKäskudeJuures = 0;
    // Loetakse config faili ridahaaval. Hiljem võiks teha nagu lähtekoodifailigagi et enne lugeda pikka sõnesse ja ss seda analüüsida.
    while ((line = read_line(file)) != NULL) 
    {
        if (KasEsimesedTähed(line, "MATHMODE KÄSUD"))
        {
            onMathmodeKäskudeJuures = 1;
            continue;
        }
        if (KasEsimesedTähed(line, "KESKKONNAD") || KasEsimesedTähed(line, "TEXTMODE KÄSUD"))
        {
            onMathmodeKäskudeJuures = 0;
            continue;
        }
        // Skip empty lines or comments and environment lines
        if (line[0] == '\0' || line[0] == '#' || KasEsimesedTähed(line, "env"))
        {
            free(line);
            continue;
        }
        if (onMathmodeKäskudeJuures)
        {
            char* NooleAsuk = strstr(line, "->");
            if (NooleAsuk == NULL)
            {
                //fprintf(stderr, "Noolt ei leitud: %s\n", line);
                free(line);
                continue;
            }
            // Kui kood jõuab siiani, on teada, et sellel real on käsu definitsioon, st leidub ->.
            // Split the line into the left and right parts
            // Left ja right on dünaamiliselt eraldatud. Seesama mälu läheb loetava structi klassimuutujatesse, seega seda ei tohi siin vabastada.
            char* left = LeiaTekstEnneTeksti(line, "->");
            char* right = strdup(&line[strlen(left)+2]);
            // Trim whitespace
            left = trim_whitespace(left);
            right = trim_whitespace(right);
            struct Käsk käsk = {0};

            käsk.definitsioon = right;
            // Parse the command name and arguments from the left side
            // 5) Käsu nimi
            if (strstr(left, "(") || strstr(left, "["))
            {
                // Järelikult leiti argumendiga käsk, st tuleb käsu structis palju väärtusi määrata.
                char* nimi = NULL;
                char* t1 = LeiaTekstEnneTeksti(left, "(");
                char* t2 = LeiaTekstEnneTeksti(left, "[");
                if (strlen(t1)<strlen(t2))
                {
                    nimi = t1;
                    free(t2);
                }
                else
                {
                    nimi = t2;
                    free(t1);
                }

                käsk.käsunimi = nimi;
                for (unsigned int i = strlen(nimi); i<strlen(left); )
                {
                    // 1) Argumentide kogus
                    if (left[i] == '(' || left[i]=='[')
                    {
                        char* argument = LeiaSuluSisu(&left[i+1]);
                        // 1) Argumentide kogus
                        käsk.argumentideKogus += 1;
                        // 2) Argumentide nimed
                        käsk.argumentideNimed = realloc(käsk.argumentideNimed, käsk.argumentideKogus*sizeof(char*));
                        käsk.argumentideNimed[käsk.argumentideKogus-1] = argument;
                        // 3) Argumentide tüübid
                        käsk.argumentideTüübid = realloc(käsk.argumentideTüübid, käsk.argumentideKogus*sizeof(char*));
                        
                        if (left[i] == '(')
                        {
                            käsk.argumentideTüübid[käsk.argumentideKogus-1] = 1;
                        }
                        else if (left[i] == '[')
                        {
                            käsk.argumentideTüübid[käsk.argumentideKogus-1] = 0;
                        }
                        i += strlen(argument);
                    }
                    else 
                    {
                        i++;
                    }
                }
            }
            else
            {  
                // Järelikult leiti käsk,  millel pole argumenti. Paljud väärtused käsu structis määratakse nulliks.
                käsk.käsunimi = strdup(left);
                käsk.argumentideKogus = 0;
                käsk.argumentideNimed = NULL;
                käsk.argumentideTüübid = NULL;
            }

            // Add the parsed command to the list
            add_käsk(käsk_list, käsk);
        }

        // Free the line buffer
        free(line);
    }

    fclose(file);
}




char* TõlgiKõik(char* tõlgitav)
{
    char* tõlge = malloc(1);
    tõlge[0] = '\0';


    unsigned int pikkus = strlen(tõlgitav);
    // Läheb üle iga tähe kogu tõlgitavas tekstis.
    for (unsigned int i = 0; i<pikkus; )
    {
        struct Environment* env = KasEnvironment(&tõlgitav[i]);
        printf("\"\n%s\"\n", &tõlgitav[i]);
        printf("KAS ESIMESED ON NMMN: %d\n", KasEsimesedTähed(&tõlgitav[i], "\nmm\r\n"));
        if (env)
        {
            struct TekstArv envTõlge = TõlgiEnvironment(&tõlgitav[i], env);
            tõlge = LiidaTekstid(tõlge, envTõlge.Tekst);
            free(envTõlge.Tekst);
            i += envTõlge.Arv;
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
                puts("TAJUST NmmN");
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