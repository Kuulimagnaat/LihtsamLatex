#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

struct KäskList käsk_list;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    #define MAX_PATH_LENGTH 256

    char cwd[MAX_PATH_LENGTH];
    char exe_path[MAX_PATH_LENGTH];
    char exe_dir[MAX_PATH_LENGTH];

    // Get the current working directory for main.txt
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("_getcwd() error");
        return EXIT_FAILURE;
    }

    // Get the executable path for template.txt
    if (GetModuleFileName(NULL, exe_path, MAX_PATH_LENGTH) == 0) {
        perror("GetModuleFileName() error");
        return EXIT_FAILURE;
    }

    // Strip the executable name to get the directory
    strcpy(exe_dir, exe_path);
    char* last_backslash = strrchr(exe_dir, '\\');
    if (last_backslash) {
        *last_backslash = '\0';  // Cut the path at the last backslash
    }

    // Construct full path to config.txt in the src folder
    char config_path[MAX_PATH_LENGTH];
    snprintf(config_path, sizeof(config_path), "%s\\src\\config.txt", exe_dir);

    // Retrieve template name from config.txt
    char* template_name = get_template_name(config_path);
    if (!template_name) {
        fprintf(stderr, "Error: Template name not specified in config.txt.\n");
        return EXIT_FAILURE;
    }

    // Load custom user-defined commands
    init_käsk_list(&käsk_list);
    read_commands_from_config(config_path, &käsk_list);
    

    char* tulemus = TõlgiMathMode("sqrtx");
    
    puts(tulemus);

    return 0;
}