#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;
struct TextmodeKäskList textmodeKäskList;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    AmmendaConfig();

    struct Käsk* a = &käskList.käsud[0];
    struct TekstArv tulemus = TõlgiKäsk("summ k=0", a);
    puts(tulemus.Tekst);
}