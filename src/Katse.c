#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    char* lõpud[] = {"uuga", "buuga", "luuga", "duuga"};
    char tekst[] = "Tere!, minu nimi on Kaarel luuga ja ma olen äge vend";
    char* tulemus = LeiaTekstEnneTekste(tekst, lõpud, 4);
    printf("\"%s\"\n", tulemus);
}