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
            int onMathmode = 0;
            while ((line = read_line(file)) != NULL)
            {
                for ( unsigned int i=0; i<strlen(line); i++)
                {
                    if (KasEsimesedTähed(&line[i], " mm "))
                    {
                        onMathmode = (onMathmode+1)%2;
                        i += 4;
                        if (onMathmode)
                        {
                            fprintf(output_file, "\\[ ");
                        }
                        else if (!onMathmode)
                        {
                            fprintf(output_file, "\\] ");
                        }
                    }
                    if (onMathmode)
                    {
                        char* tõlgitav = LeiaTekstEnneTeksti(&line[i], " mm ");
                        i += strlen(tõlgitav)-1; 
                        char* tõlge = TõlgiMathMode(tõlgitav);
                        free(tõlgitav);
                        fprintf(output_file, tõlge);
                    }
                }
            }
        }
        else {
            // Write the current line from the template
            fprintf(output_file, "%s\n", line);
        }
        free(line);
    }
}