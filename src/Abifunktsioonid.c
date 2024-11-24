#include <stdio.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Matatõlge.h"
#include <stdlib.h>
#include <Windows.h>
#include <direct.h>  // for _getcwd on Windows

#define MAX_PATH_LENGTH 256

extern struct KäskList käskList;


long int LeiaFailiSuurus(FILE* f)
{
    fseek(f, 0, SEEK_END); // seek to end of file
    long int suurus = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET);
    return suurus;
}


void TäidaKäskList()
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
        return;
    }

    // Get the executable full name. Seda teksti on vaja selleks, et sellest eraldada directory, kus exe fail asub, et seda kasutada muude failide nt config.txt ja SVEN.txt avamiseks.
    if (GetModuleFileName(NULL, exe_path, MAX_PATH_LENGTH) == 0) {
        perror("GetModuleFileName() error");
        return;
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

    init_käsk_list(&käskList);
    read_commands_from_config(config_path, &käskList);
}



long int findSize(char* file_name) 
{ 
    // opening the file in read mode 
    FILE* fp = fopen(file_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("File Not Found!\n"); 
        return -1; 
    } 
  
    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    long int res = ftell(fp); 
  
    // closing the file 
    fclose(fp); 
  
    return res; 
} 