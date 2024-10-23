#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    char a[] = "a/b";
    char* tulemus = TõlgiMathMode(&a[0]);
    //puts(tulemus);
}