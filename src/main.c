#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>  // Täpitähtedega printimiseks ja exe faili asukoha jaoks
#include <direct.h>  // for _getcwd on Windows
#include "Headers/Abifunktsioonid.h"
#include "Headers/Kõigetõlge.h"

#define MAX_PATH_LENGTH 256

struct KäskList käskList;
struct EnvironmentList environList;
int reanumber = 1;
struct TextmodeKäskList textmodeKäskList;

//struct KeskkonnaNimekiri keskkonnaNimek;

//Leiab fili viimase modification time'i
FILETIME getFileModTime(const char* file_path) {
    FILETIME modification_time = {0, 0};
    HANDLE file_handle = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Unable to open file for getting modification time: %s\n", file_path);
        return modification_time;
    }

    // Get the modification time
    if (!GetFileTime(file_handle, NULL, NULL, &modification_time)) {
        fprintf(stderr, "Error: Unable to retrieve file modification time for: %s\n", file_path);
    }

    CloseHandle(file_handle);
    return modification_time;
}

int compare_filetime(FILETIME ft1, FILETIME ft2) {
    return CompareFileTime(&ft1, &ft2);
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

    // ...\luuga\duuga\kasutajafailinimi.txt
    char main_path[MAX_PATH_LENGTH];


int main() {
    SetConsoleOutputCP(CP_UTF8);

    // ...\luuga\duuga\        <-- Kaust, kust programm käivitati – currend working directory.
    char cwd[MAX_PATH_LENGTH];
    // ...\uuga\buuga\a.exe        <-- Programmi exe faili directory, kus on ka programmi exe faili nimi lõpus.
    char exe_path[MAX_PATH_LENGTH];
    // ...\uuga\buuga\          <-- sama dir, aga ilma a.exe nimeta lõpus
    char exe_dir[MAX_PATH_LENGTH];
    // ...\uuga\buuga\src\config.txt    <-- Programmi config faili asukoht failipuus.
    char config_path[MAX_PATH_LENGTH];
    // ...\uuga\buuga\src\templatefailinimi.txt
    char template_path[MAX_PATH_LENGTH];


    // Get the current working directory for main.txt. Seda asukohta on vaja selleks, et sellel aadressil asuvat kasutaja kirjutatavat lähtekoodifaili avada.
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("_getcwd() error");
        return EXIT_FAILURE;
    }

    // Get the executable full name. Seda teksti on vaja selleks, et sellest eraldada directory, kus exe fail asub, et seda kasutada muude failide nt config.txt ja SVEN.txt avamiseks.
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

    // Koostatakse programmi exe faili aadressi abil aadress, kus asub config.txt. 
    snprintf(config_path, sizeof(config_path), "%s\\src\\config.txt", exe_dir);


    // Debugimiseks:
    //print_environment_info(&environList.environments[0]);

    // config.txt failist eraldatakse template faili nimi, millest koostatakse aadress, mis viitab soovitavale template failile.
    char* template_name = get_template_name(config_path);
    if (!template_name) {
        fprintf(stderr, "Error: Template name not specified in config.txt.\n");
        return EXIT_FAILURE;
    }

    // Construct full path to the template file in the templates folder
    snprintf(template_path, sizeof(template_path), "%s\\templates\\%s.txt", exe_dir, template_name);

    // Otsitakse funktsiooni abil working directoryst esimene tekstifail, mis on see, kust loetakse kasutaja krijtuatud latexiks tõlgitavat teksti.
    char main_txt_file[MAX_PATH_LENGTH];
    if (!find_first_txt_file(main_txt_file)) {
        free(template_name);
        return EXIT_FAILURE;
    }

    // Construct full path to the found .txt file
    snprintf(main_path, sizeof(main_path), "%s\\%s", cwd, main_txt_file);

    FILETIME last_mod_time = {0, 0};
    while (1)
    {
        Sleep(300);
        FILETIME current_mod_time = getFileModTime(main_path);
        
        if (compare_filetime(current_mod_time, last_mod_time) != 0) 
        {
            last_mod_time = current_mod_time;

            // Täidetakse käsklist ja envlist.
            AmmendaConfig();


            // config.txt failist eraldatakse template faili nimi, millest koostatakse aadress, mis viitab soovitavale template failile.
            char* template_name = get_template_name(config_path);
            if (!template_name) {
                fprintf(stderr, "Koodi ei kompileerita, sest config.txt failis on template fail määramata.\n");
            }

            // Construct full path to the template file in the templates folder
            snprintf(template_path, sizeof(template_path), "%s\\templates\\%s.txt", exe_dir, template_name);

            // Open the specified template file
            FILE* template_file = fopen(template_path, "r");
            if (template_file == NULL) {
                fprintf(stderr, "Unable to open template file: %s\n", template_path);
                free(template_name);
                return EXIT_FAILURE;
            }

            // Otsitakse funktsiooni abil working directoryst esimene tekstifail, mis on see, kust loetakse kasutaja krijtuatud latexiks tõlgitavat teksti.
            char main_txt_file[MAX_PATH_LENGTH];
            if (!find_first_txt_file(main_txt_file)) {
                fclose(template_file);
                free(template_name);
                return EXIT_FAILURE;
            }


            char* koguTekst = 0;
            long length;
            FILE* koguFail = fopen(main_path, "rb");

            if (koguFail)
            {
                fseek(koguFail, 0, SEEK_END);
                length = ftell(koguFail);
                fseek(koguFail, 0, SEEK_SET);
                
                koguTekst = malloc(length + 1);  // +1 for null terminator
                if (koguTekst)
                {
                    fread(koguTekst, 1, length, koguFail);
                    koguTekst[length] = '\0';  // Ensure null termination
                }
                fclose(koguFail);
            }


            // Open the input .txt file for reading
            FILE* file = fopen(main_path, "rb");
            if (file == NULL) {
                perror("Unable to open the main .txt file in the current directory");
                fclose(template_file);
                free(template_name);
                return EXIT_FAILURE;
            }


            // Strip the .txt extension from the found file to use for naming of output tex and pdf files.
            char output_base_name[MAX_PATH_LENGTH];
            strncpy(output_base_name, main_txt_file, sizeof(output_base_name));
            strip_txt_extension(output_base_name); // This will remove the .txt part

            // Construct the output .tex and .pdf file names based on the found .txt file
            char output_tex_path[MAX_PATH_LENGTH];
            snprintf(output_tex_path, sizeof(output_tex_path), "%s\\%s.tex", cwd, output_base_name);
            
            char output_pdf_path[MAX_PATH_LENGTH];
            snprintf(output_pdf_path, sizeof(output_pdf_path), "%s\\%s.pdf", cwd, output_base_name);

            // Open the output .tex file for writing
            FILE* output_file = fopen(output_tex_path, "wb");
            if (output_file == NULL) {
                perror("Error creating output.tex file");
                fclose(file);
                fclose(template_file);
                free(template_name);
                return EXIT_FAILURE;
            }

            char* line; // Pointer for the line
            int skip_lines = 0;
            while ((line = read_line(template_file)) != NULL) {
                // Check if the line contains a placeholder for content
                if (strcmp(line, "{{content}}") == 0) {
                    
                    // Process the main.txt content
                    char* tõlge = TõlgiKõik(koguTekst);
                    fprintf(output_file, "%s\n", tõlge);
                    free(tõlge);
                    puts("Faili tõlkimine toimis.");
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
            snprintf(compile_command, sizeof(compile_command), "pdflatex -output-directory=\"%s\" \"%s\"", cwd, output_tex_path); //-quiet
            if (system(compile_command) != 0) {
                perror("Error compiling .tex file with pdflatex");
            }

            // Free the template name memory
            free(template_name);
            
            free_environment_list(&environList);
            free_käsk_list(&käskList);
        }
    }

    return 0;
}