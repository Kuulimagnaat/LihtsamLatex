#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Abifunktsioonid.h"

struct KäskList käsk_list;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    const char a[] = "C:\\Users\\Kaarel\\Documents\\Ülikooli repod\\mimma 4. kt.txt";
    FILE* file = fopen(a, "r");
    if (file == NULL)
    {
        puts("Ei tööta.");
    }

    char exe_path[256];
    if (GetModuleFileName(NULL, exe_path, 256) == 0)
    {
        perror("GetModuleFileName() error");
        return EXIT_FAILURE;
    }
    puts(exe_path);
/*
    while (1)
    {
        long int tulemus = LeiaFailiSuurus(file);
    
        printf("%d\n", tulemus);
    }
*/

    return 0;
}