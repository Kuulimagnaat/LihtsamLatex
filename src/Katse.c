#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;
struct TextmodeKäskList textmodeKäskList;

unsigned int rekursiooniTase;
unsigned int taandePikkus = 4;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    AmmendaConfig();

    char* tulemus = TõlgiKõik("teemapealk Sissejuhatus\n\nTundub, et kood [[main.txt]] bruh Pärisori [[pärisori]].\nPikk kood. Bruh Lõpp.\n");
    puts(tulemus);
}