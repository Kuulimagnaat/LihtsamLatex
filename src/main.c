#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>  // Täpitähtedega printimiseks ja exe faili asukoha jaoks
#include <direct.h>  // for _getcwd on Windows
#include "Headers/Abifunktsioonid.h"

#define MAX_PATH_LENGTH 256

struct KäskList käskList;
struct EnvironmentList environList;
int reanumber = 1;

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
            puts("Tajuti faili modifikatsiooni aja muutumist.");
            last_mod_time = current_mod_time;
            init_käsk_list(&käskList);
            read_commands_from_config(config_path, &käskList);


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


            // Open the input .txt file for reading
            FILE* file = fopen(main_path, "r");
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

                    unsigned int peabOlemaInlineMath = 0;
                    unsigned int onJubaMathMode = 0;
                    while ((line = read_line(file)) != NULL)
                    {
                        /* CHECK IF ENVIRONMENT STARTS ON THIS LINE */
                        struct Environment* env = KasEnvironment(line);
                        if (env) {
                            printf("Detected Environment: %s\n", env->name);
                            TõlgiEnvironment(env, file);
                        }


                        if (!onJubaMathMode) 
                        {
                            peabOlemaInlineMath = 0;
                        }
                        // Uut rida alustades sõltub edasine tegevus sellest, kas math mode on eelmisest reast jäänud lõpetamata ja ollakse mathmode'is või eiu ole math mode.
                        // Kui on math mode eelmisest reast käimas ja ootab lõpetamist
                        if (onJubaMathMode)
                        {
                            fprintf(output_file, "\\\\");
                            char* leitudMata = LeiaTekstEnneTeksti(line, " mm");
                            // On kaks võimalust, mis võib olla: 
                            // Päriselt leitakse mm-tekst. Selle kontrollimiseks kontrollitakse, kas leitud tekstile järgnevad tähed on " mm".
                            if (KasEsimesedTähed(&line[strlen(leitudMata)], " mm"))
                            {
                                char* tõlge = TõlgiMathMode(leitudMata);
                                fprintf(output_file, "%s", tõlge);
                                free(tõlge);
                                if (peabOlemaInlineMath)
                                {
                                    fprintf(output_file, "$");
                                }
                                else
                                {
                                    fprintf(output_file, "\\]");
                                }
                                onJubaMathMode = 0;
                            }
                            // Järelikult ei leitud mm-teksti ja mathmode peab edasi kestma.
                            else
                            {
                                char* tõlge = TõlgiMathMode(leitudMata);
                                fprintf(output_file, "%s", tõlge);
                                free(tõlge);
                            }
                        }
                        // Kui mathmode mode pole käimas.
                        else
                        {
                            for (unsigned int i = 0; line[i]!='\0'; )
                            {
                                // Kui mingil kohal tabatakse " mm " või kui ollakse täiesti rea alguses ja seal on "mm ", siis peab sealt math mode algama.
                                if (KasEsimesedTähed(&line[i], " mm ") || i==0 && KasEsimesedTähed(&line[i], "mm "))
                                {
                                    char* leitudMata = NULL;
                                    // Proovitakse kohe selle mata lõpetavat mm-i leida.
                                    if (line[i] == 'm')
                                    {
                                        leitudMata = LeiaTekstEnneTeksti(&line[i+3], " mm");

                                        // Saab olla, et leitakse lõpetav mm, aga võib ka seda mitte leida. Kui ei leitud lõpetajat, siis tuleb panna realugemistsükliväline muutuja selliseks, et see ütleks, et mathmode peab edasi kestma.
                                        if (!KasEsimesedTähed(&line[i+3+strlen(leitudMata)], " mm"))
                                        {
                                            onJubaMathMode = 1;
                                            i += 3 + strlen(leitudMata);
                                        }
                                        else 
                                        {
                                            i += 3 + strlen(leitudMata) + 3;
                                        }
                                    }
                                    else
                                    {
                                        leitudMata = LeiaTekstEnneTeksti(&line[i+4], " mm");

                                        // Saab olla, et leitakse lõpetav mm, aga võib ka seda mitte leida. Kui ei leitud lõpetajat, siis tuleb panna realugemistsükliväline muutuja selliseks, et see ütleks, et mathmode peab edasi kestma.
                                        if (!KasEsimesedTähed(&line[i+4+strlen(leitudMata)], " mm"))
                                        {
                                            onJubaMathMode = 1;
                                            i += 4 + strlen(leitudMata);
                                        }
                                        else 
                                        {
                                            i += 4 + strlen(leitudMata) + 3;
                                        }
                                    }

                                    // Olenemata sellest, kas lõpetav mm leitakse v mitte, on vaja anda leitud tekst math mode tõlkesse.
                                    char* tõlge = TõlgiMathMode(leitudMata);

                                    // Kuna kolm taset välimine else haru ütleb, et siin mathmode pole veel käimas, siis tuleb lisada alguse ja lõpu märgid.
                                    if (peabOlemaInlineMath)
                                    {
                                        fprintf(output_file, " $");
                                    }
                                    else
                                    {
                                        fprintf(output_file, "\\[");
                                    }
                                    fprintf(output_file, "%s", tõlge);
                                    free(tõlge);
                                    // Kui onJubaMath mode on jäetud üheks muutmata, siis järelikult leiti lõpp sellelt realt, mistõttu tuleb ka faili kirjutada kohe lõpp.
                                    if (!onJubaMathMode)
                                    {
                                        if (peabOlemaInlineMath)
                                        {
                                            fprintf(output_file, "$");
                                        }
                                        else
                                        {
                                            fprintf(output_file, "\\]");
                                        }
                                    }
                                }
                                // Muul juhul, kui ei tajutud kohal i mathmode algust, siis käesolev täht lic lükatakse faili. Lisaks, kui see täht ei ole tühik, siis suvaline tulev math mode sellel real peab olema inline, mistõttu üks ridadelugemistsükliväline muutuja pannakse vastavaks.
                                else
                                {
                                    if (line[i] != ' ')
                                    {
                                        peabOlemaInlineMath = 1;
                                    }

                                    char* täht = malloc(2);
                                    täht[0] = line[i];
                                    täht[1] = '\0';
                                    fprintf(output_file, "%s", täht);
                                    i++;
                                }
                            }
                            // Soov on panna rea lõppu uuereamärk, aga mitte siis, kui on math mode.
                            if (onJubaMathMode == 0)
                            {
                                fprintf(output_file, "%s", "\n");
                            }
                        }
                        // puts("Reanumbrit kasvatati.");
                        reanumber += 1;
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
            snprintf(compile_command, sizeof(compile_command), "pdflatex -quiet -output-directory=\"%s\" \"%s\"", cwd, output_tex_path);
            if (system(compile_command) != 0) {
                perror("Error compiling .tex file with pdflatex");
            }

            // Free the template name memory
            free(template_name);

            free_käsk_list(&käskList);
        }
    }

    return 0;
}