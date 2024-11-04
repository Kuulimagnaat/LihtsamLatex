#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <direct.h>  // for _getcwd on Windows

#define MAX_PATH_LENGTH 256

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

/* Function to find the first .txt file in the current directory */
int find_first_txt_file(char* txt_file_path) {
    WIN32_FIND_DATA find_file_data;
    HANDLE h_find = FindFirstFile("*.txt", &find_file_data);

    if (h_find == INVALID_HANDLE_VALUE) {
        printf("No .txt files found in the current directory.\n");
        return 0;  // No .txt file found
    } else {
        // Store the name of the first .txt file found
        strcpy(txt_file_path, find_file_data.cFileName);
        FindClose(h_find);
        return 1;  // Success
    }
}

/* Function to strip .txt extension from the filename */
void strip_txt_extension(char* filename) {
    char* dot = strrchr(filename, '.');
    if (dot && strcmp(dot, ".txt") == 0) {
        *dot = '\0';  // Remove the .txt extension
    }
}

int main()
{   
    SetConsoleOutputCP(CP_UTF8);

    char cwd[MAX_PATH_LENGTH];
    char exe_path[MAX_PATH_LENGTH];
    char exe_dir[MAX_PATH_LENGTH];
    char txt_file_path[MAX_PATH_LENGTH];

    // Get the current working directory
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("_getcwd() error");
        return EXIT_FAILURE;
    }

    // Find the first .txt file in the current directory
    if (!find_first_txt_file(txt_file_path)) {
        printf("No .txt files found in the current directory.\n");
        return EXIT_FAILURE;
    }
    
    // Open the input .txt file for reading
    FILE *file = fopen(txt_file_path, "r");
    if (file == NULL) {
        perror("Unable to open the .txt file found in the current directory");
        return EXIT_FAILURE;
    }

    strip_txt_extension(txt_file_path);

    // Create output filenames based on the .txt filename
    char output_tex_file[MAX_PATH_LENGTH];
    char output_pdf_file[MAX_PATH_LENGTH];
    snprintf(output_tex_file, sizeof(output_tex_file), "%s.tex", txt_file_path);
    snprintf(output_pdf_file, sizeof(output_pdf_file), "%s.pdf", txt_file_path);

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
    
    // Construct full path to template.txt
    char template_path[MAX_PATH_LENGTH];
    snprintf(template_path, sizeof(template_path), "%s\\templates\\defaultTemplate.txt", exe_dir);

    // Open the template file for reading from the executable's templates folder
    FILE *template_file = fopen(template_path, "r");
    if (template_file == NULL) {
        perror("Unable to open defaultTemplate.txt in the executable's templates directory");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Open the output .tex file for writing
    FILE *output_file = fopen(output_tex_file, "w");
    if (output_file == NULL) {
        perror("Error creating output .tex file");
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

    // Close all files
    fclose(file);
    fclose(template_file);
    fclose(output_file);

    // Compile output.tex to a .pdf using pdflatex
    char compile_command[MAX_PATH_LENGTH];
    snprintf(compile_command, sizeof(compile_command), "pdflatex %s", output_tex_file);
    if (system(compile_command) != 0) {
        perror("Error compiling .tex file with pdflatex");
        return EXIT_FAILURE;
    }

    return 0;
}