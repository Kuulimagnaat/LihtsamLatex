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

/* Function to retrieve the template name from config.txt */
char* get_template_name(const char* config_path) {
    FILE* config_file = fopen(config_path, "r");
    if (config_file == NULL) {
        perror("Unable to open config.txt in src directory");
        return NULL;
    }

    char* line;
    char* template_name = NULL;
    while ((line = read_line(config_file)) != NULL) {
        if (strncmp(line, "template =", 10) == 0) {
            template_name = strdup(line + 11); // Extract template name after "template = "
            free(line);
            break;
        }
        free(line);
    }
    fclose(config_file);
    return template_name;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

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

    // Construct full path to the template file in the templates folder
    char template_path[MAX_PATH_LENGTH];
    snprintf(template_path, sizeof(template_path), "%s\\templates\\%s.txt", exe_dir, template_name);

    // Open the specified template file
    FILE* template_file = fopen(template_path, "r");
    if (template_file == NULL) {
        fprintf(stderr, "Unable to open template file: %s\n", template_path);
        free(template_name);
        return EXIT_FAILURE;
    }

    char main_txt_file[MAX_PATH_LENGTH];
    if (!find_first_txt_file(main_txt_file)) {
        fclose(template_file);
        free(template_name);
        return EXIT_FAILURE;
    }

    // Construct full path to the found .txt file
    char main_path[MAX_PATH_LENGTH];
    snprintf(main_path, sizeof(main_path), "%s\\%s", cwd, main_txt_file);

    // Open the input .txt file for reading
    FILE* file = fopen(main_path, "r");
    if (file == NULL) {
        perror("Unable to open the main .txt file in the current directory");
        fclose(template_file);
        free(template_name);
        return EXIT_FAILURE;
    }

    // Strip the .txt extension from the found file to use for output naming
    char output_base_name[MAX_PATH_LENGTH];
    strncpy(output_base_name, main_txt_file, sizeof(output_base_name));
    strip_txt_extension(output_base_name); // This will remove the .txt part

    // Construct the output .tex and .pdf file names based on the found .txt file
    char output_tex_path[MAX_PATH_LENGTH];
    snprintf(output_tex_path, sizeof(output_tex_path), "%s\\%s.tex", cwd, output_base_name);
    
    char output_pdf_path[MAX_PATH_LENGTH];
    snprintf(output_pdf_path, sizeof(output_pdf_path), "%s\\%s.pdf", cwd, output_base_name);

    // Open the output .tex file for writing
    FILE* output_file = fopen(output_tex_path, "w");
    if (output_file == NULL) {
        perror("Error creating output.tex file");
        fclose(file);
        fclose(template_file);
        free(template_name);
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
    snprintf(compile_command, sizeof(compile_command), "pdflatex -output-directory=\"%s\" \"%s\"", cwd, output_tex_path);
    if (system(compile_command) != 0) {
        perror("Error compiling .tex file with pdflatex");
        return EXIT_FAILURE;
    }

    // Free the template name memory
    free(template_name);

    return 0;
}