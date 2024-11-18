#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

struct KäskList käsk_list;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    //init_käsk_list(&käsk_list);

    char* tekst = "  uuga  ";
    char* uus = trim_whitespace(tekst);
    /*
    const char* config_path = "src/config.txt";
    read_commands_from_config(config_path, &käsk_list);

    char* tekst = "epsilon";
    char* tulemus = TõlgiMathMode(tekst);
    */
    puts(uus);

    return 0;
}