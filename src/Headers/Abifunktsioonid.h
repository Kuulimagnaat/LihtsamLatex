#ifndef Abifunktsioonid_h
#define Abifunktsioonid_h
#include <stdio.h>
#include "Kõigetõlge.h"

long int LeiaFailiSuurus(FILE* f);
void AmmendaConfig();
void TäidaEnvironmentList();
unsigned int MitmeTäheVõrraErineb(const char* tekst1, const char* tekst2 );
char* VõtaTekstIndeksini(char* tekst, unsigned int indeks);
char* LeiaTekstEnneTekste(char* tekst, char** lõpud, unsigned int lõppudeKogus);
void prindiVärviga(char* tekst, char* värv);
void prindiTaane();
void prindiTextmodeKäsuInfo(struct TextmodeKäsk* käsk);
//long long VõtaFailiSuurus(const wchar_t* name);

#endif