#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    char* tõlge = TõlgiMathMode2("cos(x+sin(ln(y)))-cd");
    printf(tõlge);
}