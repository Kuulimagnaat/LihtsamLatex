#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <Windows.h>
#include <direct.h>  // for _getcwd on Windows

#define MAX_PATH_LENGTH 256

struct KäskList käsk_list;

//struct KeskkonnaNimekiri keskkonnaNimek;

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

void free_replacements() {
    for (int i = 0; i < replacement_count; i++) {
        free((char*)math_functions_replace[i]);
        free((char*)math_functions_replace_tähendused[i]);
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // Tsükkel, mis kogu aeg jooksutab programmi uuesti ja uuesti, et pidevalt kompileerida;
    while(1)
    {
        // ...\luuga\duuga\        <-- Kaust, kust programm käivitati – currend working directory.
        char cwd[MAX_PATH_LENGTH];
        // ...\uuga\buuga\a.exe        <-- Programmi exe faili directory, kus on ka programmi exe faili nimi lõpus.
        char exe_path[MAX_PATH_LENGTH];
        // ...\uuga\buuga\          <-- sama dir, aga ilma a.exe nimeta lõpus
        char exe_dir[MAX_PATH_LENGTH];

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


        // Koostatakse programmi exe faili aadressi abil aadress, kus asub config.txt. Seejärel sealt eraldatakse kasutaja defineeritud käsud, mida hakatakse hoidma KäskListi objektis.
        char config_path[MAX_PATH_LENGTH];
        snprintf(config_path, sizeof(config_path), "%s\\src\\config.txt", exe_dir);

        init_käsk_list(&käsk_list);
        read_commands_from_config(config_path, &käsk_list);
        //loeConfigistKeskkonnad(config_path, &keskkonnaNimek);


        // config.txt failist eraldatakse template faili nimi, millest koostatakse aadress, mis viitab soovitavale template failile. Template fail avatakse, et sealt hiljem hakata tex-faili päist ja jalust lugema.
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

        // Otsitakse funktsiooni abil working directoryst esimene tekstifail, mis on see, kust loetakse kasutaja krijtuatud latexiks tõlgitavat teksti. Seejärel avatakse see fail.
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
                    }
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
            return EXIT_FAILURE;
        }

        // Free the template name memory
        free(template_name);
        free_replacements();
        free_käsk_list(&käsk_list);

        Sleep(3000);
    }
    

    return 0;
}