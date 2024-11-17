#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <math.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    char* t1 = TõlgiMathMode("ftulxxx");
    char* t2 = TõlgiMathMode("ftulxxx");
    char* t3 = TõlgiMathMode("ftulyxx");
    char* t4 = TõlgiMathMode("ftulyxy");
    char* t5 = TõlgiMathMode("ftulabbcdddddddd");

    printf("%s\n%s\n%s\n%s\n%s", t1, t2, t3, t4, t5);
}