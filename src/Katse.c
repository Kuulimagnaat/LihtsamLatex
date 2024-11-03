#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    char* tulemus = TõlgiMathMode("limxtoa 1/x = 0");


    //int tulemus = KasAvaldiseÜmberOnSulud("((x/22)");
    printf("%s\n", tulemus);
}