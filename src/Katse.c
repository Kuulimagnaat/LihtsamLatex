#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    TäidaKäskList();
    char* tulemus = TõlgiKõik(" mm a\nb mm ");
    //char* tulemus = VõtaTekstIndeksini("a\nb.mm ", 4);
    puts(tulemus);
}