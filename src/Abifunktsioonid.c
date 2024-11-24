#include <stdio.h>
#include "Headers/Abifunktsioonid.h"


long int LeiaFailiSuurus(FILE* f)
{
    fseek(f, 0, SEEK_END); // seek to end of file
    long int suurus = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET);
    return suurus;
}