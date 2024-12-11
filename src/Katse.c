#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    TäidaKäskList();
    char* tulemus = TõlgiMathMode("limxtoinf");
    printf("\"%s\"", tulemus);
}