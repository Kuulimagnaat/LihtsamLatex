#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    char esialgne[] = "uugabuuga";
    char* tulemus = EemaldaEsimeneViimane(esialgne);
    printf("\"%s\"", tulemus);
    return 0;
}