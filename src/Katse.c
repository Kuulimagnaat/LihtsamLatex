#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include "Headers/Kõigetõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;
struct TextmodeKäskList textmodeKäskList;
char* templateTekst;
unsigned int reanumber;
unsigned int rekursiooniTase;
unsigned int taandePikkus = 4;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    AmmendaConfig();

    char* tulemus = TõlgiKõik("komm on, aga mm a/b mm või\nmm\n6/4\nmm");
    puts(tulemus);
}