#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"

struct KäskList käskList;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    TäidaKäskList();
    for (unsigned int i = 0; i<käskList.count; i++)
    {
        puts(käskList.käsud[i].käsunimi);
        puts(käskList.käsud[i].definitsioon);
    }

    return 0;
}