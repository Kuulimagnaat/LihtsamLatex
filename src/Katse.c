#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    char a[] = "a";
    char* tulemus = TõlgiMathMode("a/");
    puts(tulemus);
}