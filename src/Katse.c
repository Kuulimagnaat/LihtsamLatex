#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

struct KäskList käsk_list;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    init_käsk_list(&käsk_list);
    

    const char* config_path = "src/config.txt";
    read_commands_from_config(config_path, &käsk_list);

    char* tekst = "limx=3 45 a/b";
    struct TekstArv tulemus = TõlgiKäsk(tekst, &käsk_list.käsud[0]);
    //char* tulemus = TõlgiMathMode(tekst);
    
    puts(tulemus.Tekst);

    return 0;
}