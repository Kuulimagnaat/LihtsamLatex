#include <stdio.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

char* read_line(FILE* file) {
    char* line = malloc(256);
    if (!line) {
        perror("Memory allocation error :(");
        exit(EXIT_FAILURE);
    }

    int capacity = 256;
    int length = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        if (length + 1 >= capacity) {
            capacity *= 2;
            char* new_line = realloc(line, capacity);
            if (!new_line) {
                free(line);
                perror("Memory allocation error :(");
                exit(EXIT_FAILURE);
            }
            line = new_line;
        }
        line[length++] = ch;
    }

    if (length == 0 && ch == EOF) {
        free(line); // End of file, return NULL
        return NULL;
    }

    line[length] = '\0'; // Null-terminate the string
    return line;
}

// Initialize a KäskList
void init_käsk_list(struct KäskList* list) {
    list->käsud = malloc(10 * sizeof(struct Käsk)); // -> on sama mis (*list).käsud
    list->count = 0;
    list->capacity = 10;
}

// Free the memory used by KäskList
void free_käsk_list(struct KäskList* list) {
    for (size_t i = 0; i < list->count; i++) {
        free((char*)list->käsud[i].käsunimi);
        free((char*)list->käsud[i].definitsioon);
        free(list->käsud[i].argumentideTüübid);
        for (unsigned int j = 0; j < list->käsud[i].argumentideKogus; j++) {
            free((char*)list->käsud[i].argumentideNimed[j]);
        }
        free(list->käsud[i].argumentideNimed);
    }
    free(list->käsud);
}

// Add a Käsk to the list
void add_käsk(struct KäskList* list, struct Käsk käsk) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->käsud = realloc(list->käsud, list->capacity * sizeof(struct Käsk));
        if (!list->käsud) {
            perror("Memory allocation error");
            exit(EXIT_FAILURE);
        }
    }
    list->käsud[list->count++] = käsk;
}

void read_commands_from_config(const char* filepath, struct KäskList* käsk_list) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    char* line;
    while ((line = read_line(file)) != NULL) {
        // Skip empty lines or comments
        if (line[0] == '\0' || line[0] == '#') {
            free(line);
            continue;
        }

        char* arrow = strstr(line, "->");
        if (!arrow) {
            fprintf(stderr, "Invalid line in config file: %s\n", line);
            free(line);
            continue;
        }

        // Split the line into the left and right parts
        *arrow = '\0';
        char* left = line;
        char* right = arrow + 2;
        puts(left);

        // Trim whitespace
        while (*left == ' ') left++;
        while (*right == ' ') right++;

        struct Käsk käsk = {0};

        // Parse the command name and arguments from the left side
        char* open_paren = strchr(left, '(');  // Find first '('
        if (open_paren) {
            // Extract the command name before the first '('
            *open_paren = '\0'; // Null-terminate the command name
            käsk.käsunimi = strdup(left);

            // Now parse arguments
            char* current = open_paren + 1; // Start after '('
            unsigned int arg_count = 0;

            while (current && *current) {
                char* close_paren = strchr(current, ')'); // Find closing ')'
                if (!close_paren) break; // If no closing parenthesis, stop

                // Null-terminate the argument name
                *close_paren = '\0';

                // Allocate memory and store the argument name
                käsk.argumentideNimed = realloc(käsk.argumentideNimed, (arg_count + 1) * sizeof(char*));
                käsk.argumentideNimed[arg_count] = strdup(current);

                // Default argument type (can be customized later)
                käsk.argumentideTüübid = realloc(käsk.argumentideTüübid, (arg_count + 1) * sizeof(int));
                käsk.argumentideTüübid[arg_count] = 1; // Default type "long"

                // Increment argument count
                arg_count++;

                // Move past the closing parenthesis and skip any spaces
                current = close_paren + 2;
                while (*current == ' ') {
                    current++;
                }
            }

            // Set the total argument count
            käsk.argumentideKogus = arg_count;
        } else {
            // No arguments, just the command name
            käsk.käsunimi = strdup(left);
            käsk.argumentideKogus = 0;
            käsk.argumentideNimed = NULL;
            käsk.argumentideTüübid = malloc(1 * sizeof(int));
            käsk.argumentideTüübid[0] = -1; // No arguments
        }

        // Set the definition (right-hand side of the "->")
        käsk.definitsioon = strdup(right);

        // Add the parsed command to the list
        add_käsk(käsk_list, käsk);

        // Free the line buffer
        free(line);
    }

    fclose(file);
}

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