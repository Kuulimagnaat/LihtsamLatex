#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    const char* argNim[] = {"uuga", "buuga"};
    int argTüü[] = {1,1};
    struct Käsk käsk = 
    {
        .argumentideKogus=2,
        .argumentideNimed=argNim,
        .argumentideTüübid=argTüü,
        .definitsioon="\\frac{uuga}{buuga}",
        .käsunimi="murd"
    };

    char* tekst = "a+b+murdx+2 3";
    char* tulemus = TõlgiKäsk(tekst, &käsk);
    printf("%s\n", tulemus);
}