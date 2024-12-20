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

    struct TextmodeKäsk* tulemus = KasTextmodeKäsk("//");
    puts("TULEMUS:");
    puts(tulemus->käsualgus);
}