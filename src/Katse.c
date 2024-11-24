#include <Windows.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"

struct KäskList käskList;

int main() {
    SetConsoleOutputCP(CP_UTF8);

    struct Environment env;
    init_environment(&env);

    // Example config line to parse
    const char *config_line = "env(nimi) [body, nest, subcmds:{com1, com2, uuga}] -> \\begin{test} #content \\end{test} | (com1(ka)(arel) -> \\item ka_(arel)) (com2(a) -> a) (uuga(2) -> tekst siin 2)";
        
    // Parse the config line to populate the Käsk list
    parse_environment(config_line, &env);

    // Print all the subcommands in the environment's KäskList
    printf("Environment Name: %s\n", env.name);
    printf("Freeform Allowed? (Body): %d\n", env.body);
    printf("Nesting Allowed? (Nest): %d\n", env.nest);
    printf("Begin Define: %s\n", env.beginDefine ? env.beginDefine : "Not found");
    printf("End Define: %s\n", env.endDefine ? env.endDefine : "Not found");
    printf("Content: %s\n", env.Content ? env.Content : "Not found");
 
    for (size_t i = 0; i < env.käsk_list.count; i++) {
        printf("Subcommand %zu:\n", i + 1);
        printf("  Name: %s\n", env.käsk_list.käsud[i].käsunimi);
        printf("  LaTeX Definition: %s\n", env.käsk_list.käsud[i].definitsioon);
        printf("  Argument Count: %u\n", env.käsk_list.käsud[i].argumentideKogus);
        for (unsigned int j = 0; j < env.käsk_list.käsud[i].argumentideKogus; j++) {
            printf("    Argument %u: %s (Type: %d)\n", j + 1, env.käsk_list.käsud[i].argumentideNimed[j], env.käsk_list.käsud[i].argumentideTüübid[j]);
        }
    }

    // Free the memory used by the Environment
    free_environment(&env);

    return 0;
}