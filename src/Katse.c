#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    TäidaEnvironmentList();
    print_environment_info(&environList.environments[0]);

    TäidaKäskList();
    for (unsigned int i = 0; i<käskList.käsud[0].argumentideKogus; i++)
    {
        printf("Tüüp %d: %d\n", i, käskList.käsud[0].argumentideTüübid[i]);
    }

    return 0;
}