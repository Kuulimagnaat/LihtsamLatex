#ifndef Matatõlge_h
#define Matatõlge_h

char* TõlgiMathMode(const char* tekst);
int KasEsimesedTähed(const char* tekstis, const char* tekst);
char* LeiaSuluSisu(const char* tekst);
void MahtKogusTõlge(unsigned int* maht, unsigned int* kogus, char* tõlge, const char* lisatavTekst);

#endif