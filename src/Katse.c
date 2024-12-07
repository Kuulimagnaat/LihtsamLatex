#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    TäidaKäskList();
    TäidaEnvironmentList();

    struct Environment* env1 = &environList.environments[0];

    char* tekstitav = "enum\nTere, Siin tekst!\n--";
    
    struct TekstArv tulemus = TõlgiEnvironment(tekstitav, env1);
    puts("TAGASTATUD TÕLGE:");
    puts(tulemus.Tekst);
    puts("TAGASTATUD PIKKUS:");
    printf("%d\n", tulemus.Arv);

    free_environment_list(&environList);
    free_käsk_list(&käskList);
}