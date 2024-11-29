#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"

struct KäskList käskList;
struct EnvironmentList environList;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    
    int tulemus = MitmeTäheVõrraErineb("lim", "liaxtoinf");
    printf("%d", tulemus);

    return 0;
}