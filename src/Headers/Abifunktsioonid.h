#ifndef Abifunktsioonid_h
#define Abifunktsioonid_h
#include <stdio.h>

long int LeiaFailiSuurus(FILE* f);
void TäidaKäskList();
void TäidaEnvironmentList();
long int findSize(char* file_name);
unsigned int MitmeTäheVõrraErineb(const char* tekst1, const char* tekst2 );

#endif