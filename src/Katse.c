#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    char* tõlge = TõlgiMathMode("sin(sin())+a");
    printf(tõlge);
}