#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    struct KäskList käsk_list;
    init_käsk_list(&käsk_list);

    const char* config_path = "src/config.txt";
    read_commands_from_config(config_path, &käsk_list);

    // Print all commands
    for (size_t i = 0; i < käsk_list.count; i++) {
        printf("Command: %s\n", käsk_list.käsud[i].käsunimi);
        printf("Definition: %s\n", käsk_list.käsud[i].definitsioon);
        printf("Arguments (%u): ", käsk_list.käsud[i].argumentideKogus);
        for (unsigned int j = 0; j < käsk_list.käsud[i].argumentideKogus; j++) {
            printf("%s ", käsk_list.käsud[i].argumentideNimed[j]);
        }
        printf("\n");
        printf("Argument Types: ");
        for (unsigned int j = 0; j < käsk_list.käsud[i].argumentideKogus; j++) {
            printf("%d ", käsk_list.käsud[i].argumentideTüübid[j]);
        }
        printf("\n\n");
    }

    free_käsk_list(&käsk_list);
    return 0;
}