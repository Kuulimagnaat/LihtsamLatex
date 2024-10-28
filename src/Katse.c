#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    char* tulemus = TõlgiMathMode("(a/b)/c");
    puts(tulemus);
}