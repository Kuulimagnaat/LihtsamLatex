#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    TäidaKäskList();
    
    struct TekstArv tulemus = TõlgiKäsk("^2+y^2", &käskList.käsud[0]);
    for (int i = 0; i<käskList.käsud[0].argumentideKogus; i++)
    {
        printf("Tüüp: %d\n", käskList.käsud[0].argumentideTüübid[i]);
    }
    printf("%s\n", tulemus.Tekst);
    printf("%d\n", tulemus.Arv);

    return 0;
}