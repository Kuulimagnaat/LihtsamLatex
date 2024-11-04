#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>


/* Dynamic line reading function */
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


int main()
{   
    const char* math_functions[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "fii", "roo", "alfa", "beeta", "epsilon", "delta", "to", "inf", "lim", NULL};
    const char* math_functions_tähendused[] = {"sin", "cos", "tan", "log", "ln", "sqrt", "varphi", "rho", "alpha", "beta", "varepsilon", "delta", "to", "infty", "lim", NULL};

    int func_len = 0;
    int j = 0;
    for (; math_functions[j] != NULL; j++) {
        if (strncmp("a", math_functions[j], strlen(math_functions[j])) == 0) {
            func_len = strlen(math_functions[j]);
            break;
        }
    }

    printf("%d\n", func_len);

    SetConsoleOutputCP(CP_UTF8);
    
    // Open the input .txt file for reading
    FILE *file = fopen("src/main.txt", "r");
    if (file == NULL) {
        perror("Unable to open input file");
        return EXIT_FAILURE;
    }

    // Open the template file for reading
    FILE *template_file = fopen("templates/defaultTemplate.txt", "r");
    if (template_file == NULL) {
        perror("Unable to open template file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Open the output .tex file for writing
    FILE *output_file = fopen("output/output.tex", "w");
    if (output_file == NULL) {
        perror("Error creating output file");
        fclose(file);
        fclose(template_file);
        return EXIT_FAILURE;
    }

    char* line; // Pointer for the line
    while ((line = read_line(template_file)) != NULL) {
        // Check if the line contains a placeholder for content
        if (strcmp(line, "{{content}}") == 0) {
            // Process the main.txt content
            while ((line = read_line(file)) != NULL)
            {
                for ( unsigned int i=0; i<strlen(line); i++)
                {
                    if (i==0 && KasEsimesedTähed(&line[i], "mm "))
                    {
                        i += 3;
                        fprintf(output_file, "\\[ ");
                        char* tõlgitav = LeiaTekstEnneTeksti(&line[i], " mm");
                        i += strlen(tõlgitav)-1; 
                        char* tõlge = TõlgiMathMode(tõlgitav);
                        free(tõlgitav);
                        fprintf(output_file, tõlge);
                        fprintf(output_file, "\\] ");
                        i += 4;
                    }
                    else if (KasEsimesedTähed(&line[i], " mm "))
                    {
                        i += 4;
                        fprintf(output_file, " $");
                        char* tõlgitav = LeiaTekstEnneTeksti(&line[i], " mm ");
                        i += strlen(tõlgitav)-1; 
                        char* tõlge = TõlgiMathMode(tõlgitav);
                        free(tõlgitav);
                        fprintf(output_file, tõlge);
                        fprintf(output_file, "$ ");
                        i += 4;
                    }
                    else
                    {
                        char* Täht = malloc(2);
                        Täht[0] = line[i];
                        Täht[1] = '\0';
                        fprintf(output_file, Täht);
                        free(Täht);
                    }
                }
                fprintf(output_file, "\n");
            }
        }
        else {
            // Write the current line from the template
            fprintf(output_file, "%s\n", line);
        }
        free(line);
    }
}