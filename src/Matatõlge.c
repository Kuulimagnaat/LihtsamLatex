#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <math.h>
#include <ctype.h>
#include "Headers/Abifunktsioonid.h"
#include "Windows.h"

extern struct KäskList käskList;
extern struct EnvironmentList environList;
extern int reanumber;

// Muutuja, mis hoiab endas seda infot, kui sügaval rekursiooniga ollakse. Võimaldab printida sügavusele vastavalt tühkuid debug sõnumite ette, et oleks kenam.
unsigned int rekursiooniTase;
// See kui mitu tühikut on taande pikkus.
unsigned int taandePikkus = 3;

void prindiTaane()
{
    for (unsigned int i = 0; i<rekursiooniTase*taandePikkus; i++)
    {
        printf(" ");
    } 
}


void prindiVärviga(char* tekst, char* värv)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hStdout, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, dwMode);
    if (värv == "roheline")
    {
        printf("\033[1;32m");
        printf("%s", tekst);
    }
    else if (värv == "punane")
    {
        printf("\033[31m");
        printf("%s", tekst);
    }
    printf("\033[0m");
}


/* ENVIRONMENT ASJAD */
// Function to extract a substring between two delimiters.
// Leitud kusagilt internetist, sen mingi maagia aga töötab.
void extract_between(const char *source, const char *start, const char *end, char *result, int max_len) {
    const char *start_ptr = strstr(source, start);
    if (!start_ptr) {
        fprintf(stderr, "Error: Could not find '%s' in input.\n", start);
        return;
    }
    start_ptr += strlen(start); // Move past the start delimiter

    const char *end_ptr = strstr(start_ptr, end);
    if (!end_ptr) {
        fprintf(stderr, "Error: Could not find '%s' after '%s' in input.\n", end, start);
        return;
    }

    int length = end_ptr - start_ptr;
    if (length >= max_len) {
        fprintf(stderr, "Error: Substring exceeds maximum length of %d characters.\n", max_len);
        return;
    }

    strncpy(result, start_ptr, length);
    result[length] = '\0';
}

// Function to parse flags from the part inside square brackets (that are not the subcommands)
void parse_flags_in_brackets(const char* config_line, struct Environment* env) {
    const char *start = strchr(config_line, '[');
    const char *end = strchr(config_line, ']');
    
    if (start && end && end > start) {
        // Extract the content inside the square brackets
        char flags_part[end - start];  // Allocate space for the part inside []
        strncpy(flags_part, start + 1, end - start - 1);  // Copy the content (without [ and ])
        flags_part[end - start - 1] = '\0';  // Null-terminate the string
        
        // Trim any leading or trailing spaces
        char* trimmed_flags = trim_whitespace(flags_part);

        // Check if 'body' is present and set the flag accordingly
        if (strstr(trimmed_flags, "body") != NULL) {
            env->body = 1;
        } else {
            env->body = 0;
        }

        // Check if 'nest' is present and set the flag accordingly
        if (strstr(trimmed_flags, "nest") != NULL) {
            env->nest = 1;
        } else {
            env->nest = 0;
        }
    } else {
        // If no square brackets, keep default values (both flags are 0)
        env->body = 0;
        env->nest = 0;
    }
}

// Daddy function (does everything)
void parse_environment(const char *config_line, struct Environment* env) {

    /* GET THE CONTENT FIELD */
    const char* begin_pos = strstr(config_line, "\\begin{");
    const char* end_pos = strstr(config_line, "\\end{");

    if (begin_pos && end_pos) {
        const char* end_end_pos = strchr(end_pos, '}');
        if (end_end_pos) {
            // Extract the entire content from \begin{...} to the closing }
            const char* content_start = begin_pos;
            const char* content_end = end_end_pos + 1; // Include the final '}'

            size_t content_len = content_end - content_start;
            char* content = malloc(content_len + 1);
            strncpy(content, content_start, content_len);
            content[content_len] = '\0'; // Null-terminate
            env->Content = content;
        } else {
            env->Content = NULL; // No valid ending found
        }
    } else {
        env->Content = NULL; // Missing \begin{} or \end{}
    }

    /* GET THE ENVIRONMENT BEGIN AND END DEFINITIONS */
    begin_pos = strstr(config_line, "\\begin{");
    end_pos = strstr(config_line, "\\end{");

    if (begin_pos && end_pos) {
        // Extract the part inside \begin{}
        begin_pos += 7;  // Skip over "\\begin{"
        const char* begin_end_pos = strchr(begin_pos, '}');
        if (begin_end_pos) {
            size_t begin_len = begin_end_pos - begin_pos;
            char* begin_content = malloc(begin_len + 1);
            strncpy(begin_content, begin_pos, begin_len);
            begin_content[begin_len] = '\0';
            env->beginDefine = begin_content;
        }

        // Extract the part inside \end{}
        end_pos += 5;  // Skip over "\\end{"
        const char* end_end_pos = strchr(end_pos, '}');
        if (end_end_pos) {
            size_t end_len = end_end_pos - end_pos;
            char* end_content = malloc(end_len + 1);
            strncpy(end_content, end_pos, end_len);
            end_content[end_len] = '\0';
            env->endDefine = end_content;
        }
    }
    
    /* GET THE ENVIRONMENT NAME */
    const char* start = strstr(config_line, "env(");
    if (start) {
        start += 4; // Move past 'env('
        const char* end = strchr(start, ')');
        if (end) {
            size_t name_len = end - start;
            char name[name_len + 1];
            strncpy(name, start, name_len);
            name[name_len] = '\0';  // Null-terminate the string
            env->name = strdup(name); // Allocate and assign the environment name
        }
    }
    
    /* THE BODY, NEST DETECTION PART STARTS HERE */
    parse_flags_in_brackets(config_line, env);

    /* THE SUBCOMMAND DETECTION PART STARTS HERE */
    const char *definitions_start = strchr(config_line, '|');
    if (!definitions_start) {
        printf("No subcommand definitions found.\n");
        return;
    }
    definitions_start++; // Move past the '|'

    int subcommand_count = 0;
    const char *current_pos = definitions_start;

    // Loop through each subcommand definition
    while ((current_pos = strchr(current_pos, '(')) != NULL) {
        const char *start_pos = current_pos + 1; // Skip the opening '('
        int open_parens = 1;                    // Start with one open parenthesis

        // Find the matching closing parenthesis for this subcommand
        while (open_parens > 0) {
            current_pos++;
            if (*current_pos == '(') {
                open_parens++; // Found another open parenthesis
            } else if (*current_pos == ')') {
                open_parens--; // Found a closing parenthesis
            }
        }

        // Extract the full subcommand definition (excluding '(' and ')')
        size_t def_length = current_pos - start_pos; // Exclude the closing ')'
        char subcommand_def[def_length + 1];
        strncpy(subcommand_def, start_pos, def_length);
        subcommand_def[def_length] = '\0';

        // Split the subcommand definition by "->"
        char *arrow_pos = strstr(subcommand_def, "->");
        if (!arrow_pos) {
            printf("Error: Missing '->' in subcommand definition: %s\n", subcommand_def);
            continue;
        }

        // Split into two parts
        *arrow_pos = '\0'; // Terminate the first part
        char *subcommand_name_and_args = subcommand_def;       // Part before '->'
        char *latex_definition = arrow_pos + 2;               // Part after '->'

        // Trim whitespace
        subcommand_name_and_args = trim_whitespace(subcommand_name_and_args);
        latex_definition = trim_whitespace(latex_definition);

        // Extract command name (before the first '(')
        const char *arg_start = strchr(subcommand_name_and_args, '(');
        char command_name[50]; // Assuming command names are up to 49 characters
        if (arg_start) {
            size_t name_len = arg_start - subcommand_name_and_args;
            strncpy(command_name, subcommand_name_and_args, name_len);
            command_name[name_len] = '\0'; // Null-terminate
        } else {
            strcpy(command_name, subcommand_name_and_args); // No arguments, just copy the name
        }

        // Extract arguments from the left-hand side
        const char *arg_pos = strchr(subcommand_name_and_args, '(');
        int arg_count = 0;
        char args[10][50]; // Assuming a maximum of 10 arguments, each up to 49 chars
        while (arg_pos) {
            arg_pos++; // Skip the opening '('
            const char *arg_end = strchr(arg_pos, ')');
            if (!arg_end) {
                printf("Error: Unmatched parentheses in subcommand definition: %s\n", subcommand_name_and_args);
                break;
            }

            size_t arg_len = arg_end - arg_pos;
            strncpy(args[arg_count], arg_pos, arg_len);
            args[arg_count][arg_len] = '\0'; // Null-terminate the argument
            arg_count++;

            // Look for the next argument
            arg_pos = strchr(arg_end + 1, '(');
        }

        struct Käsk subcommand = {0};
        subcommand.käsunimi = strdup(command_name);
        subcommand.argumentideKogus = arg_count;
        subcommand.definitsioon = strdup(latex_definition);

        // Allocate and fill argument types (default to 1 for all arguments)
        subcommand.argumentideTüübid = (int*)malloc(sizeof(int) * (arg_count + 1));
        for (int i = 0; i < arg_count; i++) {
            subcommand.argumentideTüübid[i] = 1;  // Default type is 1
        }
        subcommand.argumentideTüübid[arg_count] = -1;  // End marker

        // Allocate and fill argument names
        subcommand.argumentideNimed = (const char**)malloc(sizeof(const char*) * arg_count);
        for (int i = 0; i < arg_count; i++) {
            subcommand.argumentideNimed[i] = strdup(args[i]);
        }

        add_käsk(&(env->käsk_list), subcommand);

        current_pos++; // Move past the current subcommand
    }
}

// Initialize an Environment struct
void init_environment(struct Environment* env) {
    env->name = "DEFAULT (NAMING FAILED)"; // Set the environment name
    env->body = 0;
    env->nest = 0;
    env->beginDefine = NULL;
    env->endDefine = NULL;
    env->Content = NULL;
    init_käsk_list(&(env->käsk_list)); // Initialize the KäskList within the environment
}

// Free the memory used by an Environment struct
void free_environment(struct Environment* env) {
    free_käsk_list(&(env->käsk_list)); // Free the KäskList
}




// Funktsioon, mis kontorllib, kas antud teksti algus täpselt on sama, mis teine soovitud tekst. Tagastab 0 kui ei ole ja 1 kui on. Funktsiooni kasutusolukord: kui ollakse minemas tõlgitavas koodis üle tähtede, siis on vaja kontrollida, kas kättejõudnud kohas on mõne käsu nimi. Seda funktsiooni saab nimetatud olukorra tajumiseks kasutada.
#define KasEsimesedTähedDebug 0
int KasEsimesedTähed(const char* tekstis, const char* tekst)
{
    #if KasEsimesedTähedDebug == 1
    prindiTaane();
    printf("KasEsimesedTähed\n");
    prindiTaane();
    printf("SISSE: %s, %s\n", tekstis, tekst);
    rekursiooniTase += 1;
    #endif

    unsigned int tähtiKontrollida = strlen(tekst);
    for (unsigned int i = 0; i<tähtiKontrollida; i++)
    {
        if (tekstis[i] != tekst[i])
        {
            #if KasEsimesedTähedDebug == 1
            rekursiooniTase -= 1;
            prindiTaane();
            printf("VÄLJA: 0\n");
            #endif
            return 0;
        }
    }

    #if KasEsimesedTähedDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: 1\n");
    #endif
    return 1;
}



#define TõlgiAsteDebug 0
struct TekstArv TõlgiAste(const char* tekst)
{
    #if TõlgiAsteDebug == 1
    prindiTaane();
    printf("TõlgiAste\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
    #endif
    char* argument = LeiaLühemArgument(&tekst[1]);
    unsigned int argumendiPikkus = strlen(argument);
    char* sulgudetaArgument = NULL;
    if (KasAvaldiseÜmberOnSulud(argument))
    {
        sulgudetaArgument = EemaldaEsimeneViimane(argument);
        free(argument);
    }
    else
    {
        sulgudetaArgument = argument;
    }
    char* argumendiTõlge = TõlgiMathMode(sulgudetaArgument);
    free(sulgudetaArgument);

    char* tõlge = malloc(1);
    tõlge[0] = '\0';
    tõlge = LiidaTekstid(tõlge, "^{");
    tõlge = LiidaTekstid(tõlge, argumendiTõlge);
    free(argumendiTõlge);
    tõlge = LiidaTekstid(tõlge, "}");

    struct TekstArv tagastus = {.Tekst = tõlge, .Arv = argumendiPikkus};
    #if TõlgiAsteDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: %s, %d\n", tagastus.Tekst, tagastus.Arv);
    #endif
    return tagastus;
}



// SEDA FUNKTSIOONI VÕIB USALDADA: Käsitsi läbi katsetatud.
// Funktsiooni kasutatakse juhul, kui jõutakse tõlgitavas koodis funktsioonini, mille järel on avanev sulg. Siis antakse sellele funktsioonile avanevale sulule järgneva tähe mäluaadress, misjärel funktsioon leiab kogu teksti, mis peaks avaneva ja sulgeva sulu vahele jääma. Funktsioon eraldab mälu, täidab selle sulu sisuga ja tagastab selle mälu aadressi. See mälu on vaja hiljem vabastada.
#define LeiaSuluSisuDebug 0
char* LeiaSuluSisu(const char* tekst)
{
    #if LeiaSuluSisuDebug == 1
    prindiTaane();
    printf("LeiaSuluSisu\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
    #endif
    unsigned int maht = 32;
    char* mälu = malloc(maht);
    if (mälu == NULL)
    {
        perror("Ei õnnestunud mälu eraldada :(");
        exit(EXIT_FAILURE);
    }

    //Sulutase algab arvust 1, sest otsitakse sulgu, mis viiks sulutaseme nulli.
    unsigned int sulutase = 1;

    for (unsigned int i = 0; tekst[i]!='\0'; i++)
    {
        if (i >= maht)
        {
            maht += 32;
            mälu = realloc(mälu, maht);
            if (mälu == NULL)
            {
                perror("Ei õnnestunud mälu eraldada :(");
                exit(EXIT_FAILURE);
            }
        }
        if (tekst[i] == '(' || tekst[i] == '[' || tekst[i] == '{')
        {
            sulutase++;
        }
        else if (tekst[i] == ')' || tekst[i] == ']' || tekst[i] == '}')
        {
            sulutase--;
            if (sulutase == 0)
            {
                mälu[i] = '\0';
                #if LeiaSuluSisuDebug == 1
                rekursiooniTase -= 1;
                prindiTaane();
                printf("VÄLJA: %s\n", mälu);
                #endif
                return mälu;
            }
        }
        mälu[i] = tekst[i];
    }
    perror("Ei leitud sulgevat sulgu :(\n");
    exit(EXIT_FAILURE);
}



// Abimeetod, mis appendib kaks stringi, kusjuures esimene nendest on dünaamiliselt eraldatud mälu.
#define LiidaTekstidDebug 0
char* LiidaTekstid(char* eelmineMälu, const char* lisatav)
{
    #if LiidaTekstidDebug == 1
    prindiTaane();
    printf("LiidaTekstid\n");
    prindiTaane();
    printf("SISSE: \"%s\", \"%s\"\n", eelmineMälu, lisatav);
    rekursiooniTase += 1;
    #endif
    char* result = realloc(eelmineMälu, strlen(eelmineMälu) + strlen(lisatav) + 1);
    if (result == NULL)
    {
        perror("Ei õnnestunud mälu eraldada :(");
        exit(EXIT_FAILURE);
    }
    strcat(result, lisatav);
    #if LiidaTekstidDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: %s\n", result);
    #endif
    return result;
}



char* LiidaTäht(char* eelmineMälu, char lisatav)
{
    unsigned int pikkus = strlen(eelmineMälu);
    char* uusMälu = realloc(eelmineMälu, pikkus+2);
    uusMälu[pikkus] = lisatav;
    uusMälu[pikkus+1] = '\0';
    return uusMälu;
}



// Funktsioon lisab teksti lõppu mingi arvu. Ei tööta negatiivsete arvudega, aga ma tegingi selle esmalt ühe alati positiivse counteri numbri teksti lisamiseks. Vabastab eelmise mälu ja tagastab uue mälu aadressi, kuhu on kijutatud kogu eelmise tekst, aga lõpus on veel soovitud number. 
char* LiidaArv(char* eelmineMälu, int lisatav)
{
    unsigned int numbriteKogus = log10(lisatav)+1;
    unsigned int eelmisePikkus = strlen(eelmineMälu);
    char* uusMälu = realloc(eelmineMälu, eelmisePikkus + numbriteKogus + 1);
    sprintf(&uusMälu[eelmisePikkus], "%d", lisatav);
    return uusMälu;
}


// Eraldab uut mälu, kuhu pannakse argumendiga antud tekst, millelt on eest ja tagant tühikud eemladatud. Tagastab selle mälu aadressi.
#define trim_whitespaceDebug 0
char* trim_whitespace(char* str) {
    #if trim_whitespaceDebug == 1
    prindiTaane();
    printf("trim_whitespace\n");
    prindiTaane();
    printf("SISSE: \"%s\"", str);
    rekursiooniTase += 1;
    #endif


    unsigned int algusindeks = 0;
    for ( ; isspace(str[algusindeks]); algusindeks++ );
    unsigned int lõpuindeks = strlen(str)-1;
    for ( ; isspace(str[lõpuindeks]); lõpuindeks-- );

    char* tekst = malloc(lõpuindeks-algusindeks+2);
    memcpy(tekst, &str[algusindeks], lõpuindeks-algusindeks+1);
    tekst[lõpuindeks-algusindeks+1] = '\0';
    return tekst;


    #if trim_whitespaceDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: \"%s\"", str);
    #endif
    return str;
}

// Initialize a KäskList
void init_käsk_list(struct KäskList* list) {
    list->käsud = malloc(1 * sizeof(struct Käsk)); // -> on sama mis (*list).käsud
    list->count = 0;
    list->capacity = 1;
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
void add_käsk(struct KäskList* list, struct Käsk käsk)
{

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


/*
void loeConfigistKeskkonnad(const char* filepath, struct KeskkonnaNimekiri* keskkonnaNimek)
{
    FILE* file = fopen(filepath, "r");
    if (!file) 
    {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }



    char* line;
    while ((line = read_line(file)) != NULL) 
    {
        int onKeskkonnaLugemine = 0;
        // Igal real, kui ei ole kohe alguses "keskkond", siis lic edasi minna järgmisele reale. Kõik, millest hoolitakse on see, mis on "keskkonna" taga.
        if (!KasEsimesedTähed(line, "keskkond"))
        {
            continue;
        }
        // Kui kood jõuab siia, ss järelikult on käesoleval real esimene sõna "keskkond".


        char* left = line;
        char* right = arrow + 2;

        // Trim whitespace
        left = trim_whitespace(left);
        right = trim_whitespace(right);

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
*/


void read_commands_from_config(const char* filepath, struct KäskList* käsk_list)
{
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    char* line;
    while ((line = read_line(file)) != NULL) 
    {
        // Skip empty lines or comments and environment lines
        if (line[0] == '\0' || line[0] == '#' || KasEsimesedTähed(line, "env"))
        {
            free(line);
            continue;
        }

        char* NooleAsuk = strstr(line, "->");
        if (NooleAsuk == NULL)
        {
            //fprintf(stderr, "Noolt ei leitud: %s\n", line);
            free(line);
            continue;
        }
        // Kui kood jõuab siiani, on teada, et sellel real on käsu definitsioon, st leidub ->.
        // Split the line into the left and right parts
        // Left ja right on dünaamiliselt eraldatud. Seesama mälu läheb loetava structi klassimuutujatesse, seega seda ei tohi siin vabastada.
        char* left = LeiaTekstEnneTeksti(line, "->");
        char* right = strdup(&line[strlen(left)+2]);
        // Trim whitespace
        left = trim_whitespace(left);
        right = trim_whitespace(right);
        struct Käsk käsk = {0};

        käsk.definitsioon = right;
        // Parse the command name and arguments from the left side
        // 5) Käsu nimi
        if (strstr(left, "(") || strstr(left, "["))
        {
            // Järelikult leiti argumendiga käsk, st tuleb käsu structis palju väärtusi määrata.
            char* nimi = NULL;
            char* t1 = LeiaTekstEnneTeksti(left, "(");
            char* t2 = LeiaTekstEnneTeksti(left, "[");
            if (strlen(t1)<strlen(t2))
            {
                nimi = t1;
                free(t2);
            }
            else
            {
                nimi = t2;
                free(t1);
            }

            käsk.käsunimi = nimi;
            for (unsigned int i = strlen(nimi); i<strlen(left); )
            {
                // 1) Argumentide kogus
                if (left[i] == '(' || left[i]=='[')
                {
                    char* argument = LeiaSuluSisu(&left[i+1]);
                    // 1) Argumentide kogus
                    käsk.argumentideKogus += 1;
                    // 2) Argumentide nimed
                    käsk.argumentideNimed = realloc(käsk.argumentideNimed, käsk.argumentideKogus*sizeof(char*));
                    käsk.argumentideNimed[käsk.argumentideKogus-1] = argument;
                    // 3) Argumentide tüübid
                    käsk.argumentideTüübid = realloc(käsk.argumentideTüübid, käsk.argumentideKogus*sizeof(char*));
                    
                    if (left[i] == '(')
                    {
                        käsk.argumentideTüübid[käsk.argumentideKogus-1] = 0;
                    }
                    else if (left[i] == '[')
                    {
                        käsk.argumentideTüübid[käsk.argumentideKogus-1] = 1;
                    }
                    i += strlen(argument);
                }
                else 
                {
                    i++;
                }
            }
        }
        else
        {  
            // Järelikult leiti käsk,  millel pole argumenti. Paljud väärtused käsu structis määratakse nulliks.
            käsk.käsunimi = strdup(left);
            käsk.argumentideKogus = 0;
            käsk.argumentideNimed = NULL;
            käsk.argumentideTüübid = NULL;
        }

        /* char* open_paren = strchr(left, '(');  // Find first '('
        if (open_paren) {
            // Extract the command name before the first '('
            *open_paren = '\0'; // Null-terminate the command name
            käsk.käsunimi = strdup(left);

            // Now parse arguments
            char* current = open_paren + 1; // Start after '('
            unsigned int arg_count = 0;

            
         // while (current && *current) fucking abomination
            while (*current != '\0')
            {
                char* close_paren = strchr(current, ')'); // Find closing ')'
                if (!close_paren) break; // If no closing parenthesis, stop

                // Null-terminate the argument name
                *close_paren = '\0';

                // Allocate memory and store the argument name
                käsk.argumentideNimed = realloc(käsk.argumentideNimed, (arg_count + 1) * sizeof(char*));
                käsk.argumentideNimed[arg_count] = strdup(current);
                //puts(käsk.argumentideNimed[arg_count]);

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
            käsk.argumentideTüübid = NULL;
        }
        */

        // Set the definition (right-hand side of the "->")

        //puts("KOOD SIIN!");
        // Add the parsed command to the list
        add_käsk(käsk_list, käsk);

        // Free the line buffer
        free(line);
    }

    fclose(file);
}



// duplikeerib antud stringi kuni n baidini (tagastab pointeri uuele stringile)
char* my_strndup(const char* s, size_t n) {
    char* result;
    size_t len = strlen(s);

    if (n < len) {
        len = n;
    }

    result = malloc(len + 1); // Mälu substringi jaoks
    if (!result) {
        perror("Mälu ei saadud eraldada. :(\n");
        exit(EXIT_FAILURE);
    }

    strncpy(result, s, len); // Kopeerime kuni n characteri
    result[len] = '\0'; //Null-terminate
    return result;
}



// Kui TõlgiMathMode funktsioonis tajutakse, et kättejõudnud kohal on mingi käsk, siis seal kohas antakse selle koha aadress ja tajutud käsule vastav struct selele funtksiooile, et see saaks tõlkida seda kohta. 
#define TõlgiKäskDebug 1
struct TekstArv TõlgiKäsk(const char* tekst, struct Käsk* käsk)
{
    #if TõlgiKäskDebug == 1
    prindiTaane();
    printf("TõlgiKäsk\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
    #endif
    /*
    Teoreetiliselt command võib olla selline, et kaks argumenti pole järjest. Näiteks oleks definitsioon selline:
    uuga(arg1)buuga(arg2) -> \frac{arg1}{arg2}
    Sel juhul saaks lähtekoodis kirjutada niimoodi: uugaxbuuga4, millest saab \frac{x}{4}.
    Kas frac on ainus omalaadne, kus üks argument on käsu enda nimest eespool? Oletame, et ei ole. Ss võiks vabalt ka mitu argumenti käsu enda nimest ettepoole panna, aga ss peaks lugema nii palju tulevikku, et aru saada, kas käesoleval kohal on mingi mitmeargumendilise käsu esimene argument.
    // Ütleme, et ei tohi nii olla ja ütleme, et ei tohi isegi olla käsu definitsioonid sellised: uuga(arg1)buuga(arg2). Postuleerime, et kõik argumendid peavad olema kõige lõpus igal commandil ja loodame selle peale, et me ei taha tulevikus midagi muud fracilaadset programmi lisada. frac on ainus omalaadne.

    // Kõik argumendid selles nimekirjas ja hiljem see nimekiri ise on vaja vabastada.*/
    char** argumentideTõlked = malloc(käsk->argumentideKogus*sizeof(char*));
    unsigned int i = strlen(käsk->käsunimi);
    // Nii mitu korda tuleb argumenti otsida.
    for (unsigned int j=0; j<käsk->argumentideKogus; j++)
    {
        //puts("Kood jõudis siia!");
        char* argument = NULL;
        if (käsk->argumentideTüübid[j] == 0)
        {
            //puts("Kood jõudis siia");
            argument = LeiaLühemArgument(&tekst[i]);
        }
        else if (käsk->argumentideTüübid[j] == 1)
        {
            argument = LeiaArgument(&tekst[i]);
        }
        argumentideTõlked[j] = TõlgiMathMode(argument);
        i += strlen(argument)+1;
        free(argument);
    }

    unsigned int pikkus = i;
    // Kui kood siia jõuab, on iga argumendi tõlge nimekirjas argumentideTõlked. Nüüd on vaja käia üle käsu structis oleva definitsiooni ja asendada muutujanimed vastavate tõlgetega. Saadav asi ongi käsu tõlge.


    char* tõlge = malloc(1);
    tõlge[0] = '\0';
    

    // Läheb üle definitsiooni tähtede
    for (unsigned int i = 0; käsk->definitsioon[i]!='\0';)
    {
        // Läheb iga tähe puhul üle kõigi argumentide nimede (need, mis definitsioonist loeti)
        unsigned int j=0;
        for (; j < käsk->argumentideKogus; j++)
        {
            if (KasEsimesedTähed(&(käsk->definitsioon[i]), käsk->argumentideNimed[j]))
            {
                tõlge = LiidaTekstid(tõlge, argumentideTõlked[j]);
                // Tuleb liita argumenditõlge kohal j tõlkele.
                i += strlen(käsk->argumentideNimed[j]);
                break;
            }
        }
        if (j == käsk->argumentideKogus)
        {
            // Tuleb liita definitsiooni täht tõlkele
            char täht[2] = {käsk->definitsioon[i], '\0'};
            tõlge = LiidaTekstid(tõlge, täht);
            i++;            
        }
    }

    for (unsigned int i =0; i < käsk->argumentideKogus; i++)
    {
        free(argumentideTõlked[i]);
    }
    free(argumentideTõlked);
    
    struct TekstArv tagastus = {.Arv=pikkus, .Tekst=tõlge};

    #if TõlgiKäskDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: %d, %s\n", tagastus.Arv, tagastus.Tekst );
    #endif
    return tagastus;
}



// Abimeetod, mis appendib kaks stringi
char* append_str(const char* a, const char* b) {
    char* result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}



// Võtab sisse mäluaadressi, kust algab mingi tekst. Eraldab mälu ja kirjutab sinna selle teksti ainult ilma esimese ja viimase täheta. Mõeldud selleks, et avaldiselt kujul (xxx) sulud ära võtta.
#define EemaldaEsimeneViimaneDebug 0
char* EemaldaEsimeneViimane(char* tekst)
{
    #if EemaldaEsimeneViimaneDebug == 1
    prindiTaane();
    printf("TõlgiKäsk\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
    #endif
    unsigned int l = strlen(tekst);
    char* lühem = malloc(l-1);
    memcpy(lühem, &tekst[1], l-2);
    lühem[l-2] = '\0';

    #if EemaldaEsimeneViimaneDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: %s\n", lühem);
    #endif

    return lühem;
}



// Selleks, et teada saada, kas avaldis on ümbritsetud mõttetute sulgudega, ei piisa kontrollimast, kas esimene täht on ( ja viimane ), sest avaldis võib olla näiteks (a+b)*(c+d). Tõepoolest, esimene täht on ( ja viimane ), aga need sulud pole avaldist ümbritsevad sulud.
#define KasAvaldiseÜmberOnSuludDebug 0
int KasAvaldiseÜmberOnSulud(const char* tekst)
{
    #if KasAvaldiseÜmberOnSuludDebug == 1
    prindiTaane();
    printf("KasAvaldisePmberOnSulud\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
    #endif
    if (tekst[0] != '(')
    {
        #if KasAvaldiseÜmberOnSuludDebug == 1
        rekursiooniTase -= 1;
        prindiTaane();
        printf("VÄLJA: 0");
        #endif
        return 0;
    }
    // Järelikult esimene täht on (.
    char* sulusisu = LeiaSuluSisu(&tekst[1]);
    // Kontrollin, kas esimest sulgu sulgev sulg on teksti viimane täht.
    if (&tekst[strlen(sulusisu)+1] == &tekst[strlen(tekst)-1])
    {
        #if KasAvaldiseÜmberOnSuludDebug == 1
        rekursiooniTase -= 1;
        prindiTaane();
        printf("VÄLJA: 1");
        #endif
        return 1;
    }
    #if KasAvaldiseÜmberOnSuludDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: 0");
    #endif
    return 0;
}



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
            char* line = realloc(line, capacity);
            if (!line) {
                free(line);
                perror("Memory allocation error :(");
                exit(EXIT_FAILURE);
            }
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


// Vahel on soov anda mingi käsu argumendiks mingi kaheargumendilise käsu väljakutse. Näiteks sqrtsumk=1 m f(x). Seal mõeldakse sqrt argumendiks sumk=1 m, aga loetakse ainult sumk=1, sest loetakse esimese tühikuni. See funktsioon üritab seda probleemi lahendada, et ei peaks igale poole sulgusid toppima ainult selle pärast, et meie programm muidu ei tööta.
/*char* LeiaArgumentRekursiivselt(char* argumendiAlgus)
{
    for(unsigned int i=0; ; )
    {
        KasKäsk();
    }
}*/


// Funktsioon, mis on mõeldud funktsiooni tul argumendi lihtsustamiseks. See peab kaks kõrvutiolevat sama tähte asendama selle tähe ruuduga, kolm kõrvutiolevat sama tähte selle tähe kuubiga jne. Näiteks xxxyy -> x^{3}y^{2} ja xxyxy -> x^{2}yxy
char* KõrvutiolevadAstmeks(const char* tekst)
{
    char* tõlge = malloc(1);
    tõlge[0] = '\0';
    unsigned int aste = 1;
    for (unsigned int i = 0; tekst[i]!='\0'; i++)
    {
        if (tekst[i+1] == tekst[i])
        {
            aste++;
            printf("%d\n", aste);
            puts("Astet kasvatati!");
            continue;
        }
        else
        {
            if (aste == 1)
            {
                tõlge = LiidaTäht(tõlge, tekst[i]);
            }
            else
            {
                //puts("Programm jõudis ^{ lisamise juurde!");
                tõlge = LiidaTäht(tõlge, tekst[i]);
                tõlge = LiidaTekstid(tõlge, "^{");
                tõlge = LiidaArv(tõlge, aste);

                tõlge = LiidaTekstid(tõlge, "}");
                aste = 1;
            }
        }
    }
    return tõlge;
}



/*
// Funktsioon uurib, kas tegu on käsuga. Tagastab 1, kui on käsk, ja 0, kui ei ole käsk.
int KasKäsk(const char* tekst, struct KäskList* käsuNimek, int* indeks)
{
    printf("KasKäsk\n");
    printf("  SISSE: %s, %d\n", tekst, *indeks);
    for (unsigned int i = 0; i<käsuNimek->count; i++)
    {
        //printf("\"%s\"", käsuNimek->käsud[i].käsunimi);
        if (KasEsimesedTähed(tekst, käsuNimek->käsud[i].käsunimi))
        {
            *indeks = i;
            printf("  VÄLJA: 1\n");
            return 1;
        }
    }
    printf("  VÄLJA: 0\n");
    return 0;
}
*/

/* SOME MORE ENVIRONMENT METHODS (I COULDNT FIND THE ACTUAL SPOT) */
void init_environment_list(struct EnvironmentList* list) {
    list->environments = malloc(10 * sizeof(struct Environment));
    if (!list->environments) {
        perror("Failed to allocate memory for EnvironmentList");
        exit(EXIT_FAILURE);
    }
    list->count = 0;
    list->capacity = 10;
}

void add_environment(struct EnvironmentList* list, struct Environment env) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->environments = realloc(list->environments, list->capacity * sizeof(struct Environment));
        if (!list->environments) {
            perror("Failed to reallocate memory for EnvironmentList");
            exit(EXIT_FAILURE);
        }
    }
    list->environments[list->count++] = env;
}

void free_environment_list(struct EnvironmentList* list) {
    for (size_t i = 0; i < list->count; i++) {
        free_environment(&list->environments[i]); // See perse meetod mis ma ennem kirjutasin selle jaoks lols
    }
    free(list->environments);
    list->environments = NULL;
    list->count = 0;
    list->capacity = 0;
}


struct Environment* KasEnvironment(const char* tekst)
{
    for (size_t i = 0; i < environList.count; i++) {
        if (KasEsimesedTähed(tekst, environList.environments[i].name)) {
            return &environList.environments[i];
        }
    }
}

struct Käsk* KasEnvironmentKäsk(const char* tekst, const struct Environment* env) {
    for (unsigned int i = 0; i < env->käsk_list.count; i++) {
        if (KasEsimesedTähed(tekst, env->käsk_list.käsud[i].käsunimi)) {
        
            return &env->käsk_list.käsud[i];
        }
    }
    return NULL;
}

struct TekstArv TõlgiEnvironment(const struct Environment* env, FILE* input)
{
    char* line;

    while ((line = read_line(input)) != NULL) {
        // Stop if we encounter an empty line
        if (line[0] == '\0') {
            printf("Empty line detected, ending environment.\n");
            free(line);
            break;
        }

        // Try to match a subcommand within the environment
        struct Käsk* käsk = KasEnvironmentKäsk(line, env);
        if (käsk) {
            printf("Matched Subcommand: %s\n", käsk->käsunimi);
            
            // Siia paneme hiljem veel sitta
        } else {
            printf("No match for line: %s\n", line);
        }

        free(line);
    }
}


void read_environments_from_config(const char* filepath, struct EnvironmentList* env_list) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    char* line;
    while ((line = read_line(file)) != NULL) {
        // Skip empty line or kommentaarid yo
        if (line[0] == '\0' || line[0] == '#') {
            free(line);
            continue;
        }

        // Eeldame, et "env" kasutame AINULT siis, kui defineerime uut keskkonda
        if (strncmp(line, "env", 3) == 0) {
            printf("Valid Environment Line: %s\n", line);
            struct Environment env = {0}; // Initialize a new Environment struct
            parse_environment(line, &env); // Use the existing parsing method
            add_environment(env_list, env); // Add the parsed environment to the global list
        }

        free(line);
    }

    fclose(file);
}

// Prindib konkreetse keskkonna kõikvõimaliku info välja
void print_environment_info(struct Environment* env) {
    if (!env) {
        printf("Invalid environment pointer.\n");
        return;
    }

    printf("Environment Name: %s\n", env->name);
    printf("Freeform Allowed? (Body): %d\n", env->body);
    printf("Nesting Allowed? (Nest): %d\n", env->nest);
    printf("Begin Define: %s\n", env->beginDefine ? env->beginDefine : "Not found");
    printf("End Define: %s\n", env->endDefine ? env->endDefine : "Not found");
    printf("Content: %s\n", env->Content ? env->Content : "Not found");

    for (size_t i = 0; i < env->käsk_list.count; i++) {
        printf("Subcommand %zu:\n", i + 1);
        printf("  Name: %s\n", env->käsk_list.käsud[i].käsunimi);
        printf("  LaTeX Definition: %s\n", env->käsk_list.käsud[i].definitsioon);
        printf("  Argument Count: %u\n", env->käsk_list.käsud[i].argumentideKogus);

        for (unsigned int j = 0; j < env->käsk_list.käsud[i].argumentideKogus; j++) {
            printf("    Argument %u: %s (Type: %d)\n", j + 1, env->käsk_list.käsud[i].argumentideNimed[j], env->käsk_list.käsud[i].argumentideTüübid[j]);
        }
    }
}


// Funkstioon uurib kas tegu on käsuga. Tagastab vastava käsu structi mäluaadressi kui on käsk ja NULL, kui ei ole käsk.
#define KasKäskDebug 0
struct Käsk* KasKäsk(const char* tekst)
{
    #if KasKäskDebug == 1
    prindiTaane();
    printf("KasKäsk\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
    #endif

    for (unsigned int i = 0; i<käskList.count; i++)
    {
        if (KasEsimesedTähed(tekst, käskList.käsud[i].käsunimi))
        {
            #if KasKäskDebug == 1
            rekursiooniTase -= 1;
            prindiTaane();
            printf("VÄLJA: käsustruktuur %s\n", käskList.käsud[i].käsunimi);
            #endif
            // Operaatorid . ja [] on prioriteedilt enne mäluaadressivõtmisoperaatorit, seega sulge ei pea olema.
            return &käskList.käsud[i];
        }
    }
    #if KasKäskDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: NULL\n");
    #endif
    return NULL;
}

extern char main_path[256];
// Rekursiivselt tõlgime math moodi latexisse. Funktsionn võtab sisse teksti, mida hakata tõlkima ja nimekirja nendest käskudest, mida funktsioon saab tõlkimiseks kasutada.
#define TõlgiMathModeDebug 1
char* TõlgiMathMode(const char* expression)
{
    #if TõlgiMathModeDebug == 1
    prindiTaane();
    printf("TõlgiMathMode\n");
    prindiTaane();
    printf("SISSE: %s\n", expression);
    rekursiooniTase += 1;
    #endif

    //printf("Main_path tõlgimathmodes: %s\n", main_path);

    char* result = malloc(1); // Empty string
    result[0] = '\0';

    int i = 0;
    while (i < strlen(expression))
    {

        for (unsigned int j = 0 ; j<käskList.count; j++)
        {
            // Ei lasta tähtede erinevusi kontrollida, kui käsolev käsk on ainult 1 v kahe tähe pikkune. Nende puhul on ühetäheline erinevus liiga tõenäoline.
            if (strlen(käskList.käsud[j].käsunimi)<= 2)
            {
                continue;
            }
            if (MitmeTäheVõrraErineb(käskList.käsud[j].käsunimi, &expression[i]) == 1 && KasKäsk(&expression[i]) == 0)
            {
                prindiTaane();
                unsigned int veapikkus = strlen(käskList.käsud[j].käsunimi);
                char* viga = malloc(veapikkus+1);
                strncpy(viga, &expression[i], veapikkus);
                
                printf("\nLeiti ainult ühe täheline erinevus.\nArvatatav viga: %s, mida tõenäoliselt mõeldi: %s.\n", viga ,käskList.käsud[j].käsunimi);

                int mituRidaEtte = 1; 
                int mituRidaTaha = 1; 
                FILE* file = fopen(main_path, "r");
                char* line;
                for (int k = 0; k<=reanumber+mituRidaTaha-1; k++)
                {
                    line = read_line(file);
                    if (k >= reanumber-1-mituRidaEtte)
                    {
                        // Kui on veaga rida
                        if (k == reanumber-1)
                        {
                            char* enneViga = LeiaTekstEnneTeksti(line, viga);
                            printf("  %d: ", k+1);
                            printf(enneViga);
                            prindiVärviga(viga, "punane");
                            printf(&line[strlen(enneViga)+veapikkus]);
                            printf("\n");
                            free(enneViga);
                        }
                        // Eelnevate ja järgnevate ridade printimine on ilma mingi vormistuseta ns.
                        else
                        {
                            printf("  %d: %s\n", k+1, line);
                        }
                    }
                }
                puts("");
            }
        }
        


        // Kontrollib kas tegu on lugejaga.
        if (KasLugeja(&expression[i]) == 1) {
            char* lugeja = LeiaLugeja(&expression[i]);
            int lugejaOnSulgudeta = 1;
            char* sulgudetaLugeja;

            // Check if the reader has parentheses
            if (KasAvaldiseÜmberOnSulud(lugeja)) 
            {
                lugejaOnSulgudeta = 0;
                sulgudetaLugeja = EemaldaEsimeneViimane(lugeja);
            } 
            else 
            {
                sulgudetaLugeja = lugeja;
            }

            char* lugejaTõlge = TõlgiMathMode(sulgudetaLugeja);
            char* nimetaja = LeiaNimetaja(&expression[i + strlen(lugeja) + 1]);
            char* nimetajaTõlge = TõlgiMathMode(nimetaja);

            result = LiidaTekstid(result, "\\frac{");
            result = LiidaTekstid(result, lugejaTõlge);
            result = LiidaTekstid(result, "}{");
            result = LiidaTekstid(result, nimetajaTõlge);
            result = LiidaTekstid(result, "}");

            // Update the index
            i += strlen(lugeja) + 1 + strlen(nimetaja);

            // Free memory to prevent leaks
            free(lugeja);
            if (lugejaOnSulgudeta == 0)
            {
                free(sulgudetaLugeja);
            }
            free(nimetaja);
            free(lugejaTõlge);
            free(nimetajaTõlge);
            continue;
        }


        if (expression[i] == '(')
        {
            result = LiidaTekstid(result, "\\left(");
            i += 1;

            char* suluSisu = LeiaSuluSisu(&expression[i]);
            i += strlen(suluSisu);

            char* suluSisuTõlge = TõlgiMathMode(suluSisu);
            free(suluSisu);
            
            result = LiidaTekstid(result, suluSisuTõlge);
            free(suluSisuTõlge);

            result = LiidaTekstid(result, "\\right)");
            i+=1;
        }


        // Check for 'tul' commands
        if (KasEsimesedTähed(&expression[i], "tul"))
        {
            char* tõlge = malloc(1);
            tõlge[0] = '\0';

            char* argument = LeiaLühemArgument(&expression[i + 3]);
            // In the case of argument 'xxy', the translation must be '''_{xxy}
            char* alatekst = KõrvutiolevadAstmeks(argument);
            for (unsigned int j = 0; j < strlen(argument); j++)
            {
                tõlge = LiidaTäht(tõlge, '\'');
            }
            tõlge = LiidaTekstid(tõlge, "_{");
            tõlge = LiidaTekstid(tõlge, alatekst);
            tõlge = LiidaTekstid(tõlge, "}");

            i += 3 + strlen(argument);
            result = LiidaTekstid(result, tõlge);

            // Free allocated memory
            free(argument);
            free(alatekst);
            free(tõlge);
            continue;
        }


        // Check for known commands
        struct Käsk* käsk = KasKäsk(&expression[i]);
        if (käsk != NULL)
        {
            struct TekstArv käsuTagastus = TõlgiKäsk(&expression[i], käsk);
            result = LiidaTekstid(result, käsuTagastus.Tekst);
            i += käsuTagastus.Arv;

            free(käsuTagastus.Tekst);
        }

        else 
        {
            // Handle regular characters
            char* single_char = my_strndup(&expression[i], 1);
            result = LiidaTekstid(result, single_char);
            free(single_char);
            i++;
        }
    }

    #if TõlgiMathModeDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: %s\n", result);
    #endif

    return result;
}


char* LeiaTekstEnneTähte(const char* tekst, char täht)
{
    for (unsigned int i=0; i<strlen(tekst); i++)
    {
        if (tekst[i] == täht)
        {
            char* tekstEnne = malloc(i+1);
            memcpy(tekstEnne, tekst, i);
            tekstEnne[i] = '\0';
            return tekstEnne;
        }
    }
    return NULL;
}



/* Hakkab esimest argumenti algusest uurima ja otsib, ega mingil kohal alga teise argumendi tekst. Kui mingil kohal algab, ss eraldab mälu ja kopeerib sinna kogu teksti, mis eelnes sellele kohale. Mõeldud kasutamiseks kohtades, kus on vaja leida tekst enne sulgevat sulgu v tekst enne jagamismärki vms. */
char* LeiaTekstEnneTeksti(const char* tekst, const char* teksti)
{
    char* koht = strstr(tekst, teksti);
    if (koht == NULL)
    {
        char* leitud = malloc(strlen(tekst)+1);
        memcpy(leitud, tekst, strlen(tekst));
        leitud[strlen(tekst)] = '\0';
        return leitud;
    }
    char* leitud = malloc(koht-tekst+1);
    memcpy(leitud, tekst, koht-tekst);
    leitud[koht-tekst] = '\0';
    return leitud;
}



/* Funktsioon võtab sisse mäluaadressi, kus on limi algus. Funktsioon hakkab sealt teksti lugema, eraldab endale sobiva koguse mälu ja kirjutab sellesse loetud teksti tõlke. Tagastab structi, mis sisaldab endas tõlke alguse aadressi ja seda, kui palju funktsiooni väline kood peaks lähtekoodis edasi hüppama, et jõuda tekstis sinnamaale, kus lim läbi saab.
Täpsemalt on kolm võimalust, mis limi tekstile järgnev jähemärk olla saab. Iga võimaluse puhul käitutakse erinevalt.
1) limi järel on tühik. Siis limi alla teksti ei tule ja lim on üksinda. Selline lim on jadade piirväärtusel.
2) limi järel on avanev sulg. Siis limi alla läheb kõik see, mis jääb nimetatud avaneva ja seda sulgeva sulu vahele.
3) limi järel on mingi muu täht. Siis läheb limi alla kõik see, mis jääb limi ja esimese tühiku vahele. limlima/btoc (a+b)/c+d*/
struct LimiTagastus TõlgiLim(char* tekst)
{
    struct LimiTagastus tagastus = {.TähtiLoeti=0, .Tõlge=NULL};

    char* mälu = malloc(1);
    if (mälu == NULL)
    {
        perror("Ei õnnestunud mälu eraldada :(");
        exit(EXIT_FAILURE);
    }
    mälu[0] = '\0';    

    mälu = LiidaTekstid(mälu, "\\lim");
    if (tekst[3] == '(')
    {
        char* sulusisu = LeiaSuluSisu(&tekst[4]);
        char* tõlge = TõlgiMathMode(sulusisu);
        mälu = LiidaTekstid(mälu, "_{");
        mälu = LiidaTekstid(mälu, tõlge);
        free(tõlge);
        mälu = LiidaTekstid(mälu, "}");
        tagastus.TähtiLoeti = 4+strlen(sulusisu)+1;
        tagastus.Tõlge = mälu;
        free(sulusisu);
        return tagastus;
    }
    else if (tekst[3] == ' ')
    {
        mälu = LiidaTekstid(mälu, " ");
        tagastus.TähtiLoeti = 4;
        tagastus.Tõlge = mälu;
        return tagastus;
    }
    // Tegeleda hiljem põhjalikumalt üldisema juhu jaoks, kus limi all võib omakorda lim olla ja esimene tühik tajutakse ära sisemise limi omaks, seega loetake edasi otsimaks järgmist tühikut, mis lõpetaks välimise limi.
    else
    {
        char* limiAlla = LeiaTekstEnneTähte(&tekst[3], ' ');
        if (limiAlla == NULL)
        {
            free(limiAlla);
            limiAlla = &tekst[3];
        }
        char* tõlge = TõlgiMathMode(limiAlla);
        mälu = LiidaTekstid(mälu, "_{");
        mälu = LiidaTekstid(mälu, tõlge);
        free(tõlge);
        mälu = LiidaTekstid(mälu, "}");
        tagastus.TähtiLoeti = 3+strlen(limiAlla)+1;
        free(limiAlla);
        tagastus.Tõlge = mälu;
        return tagastus;
    }
}



#define LeiaNimetajaDebug 0
char* LeiaNimetaja(const char* tekst) // sin(x)/sin(x + 4)abc     va(4 sin(x)x)/sin(x + 4)abc
{
    #if LeiaNimetajaDebug == 1
    prindiTaane();
    printf("LeiaNimetaja\n");
    prindiTaane();
    printf("SISSE: %s.\n", tekst);
    rekursiooniTase+=1;
    #endif
    for (unsigned int i = 0; tekst[i]!='/';)
    {
        if (tekst[i] == ' ' || tekst[i]=='\0')
        {
            char* nimetajaMälu = malloc(i+1);
            memcpy(nimetajaMälu, tekst, i);
            nimetajaMälu[i] = '\0';

            #if LeiaNimetajaDebug == 1
            rekursiooniTase-=1;
            prindiTaane();
            printf("VÄLJA: %s\n", nimetajaMälu);
            #endif
            return nimetajaMälu;
        }
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            unsigned int pikkus = strlen(sulusisu);
            free(sulusisu);
            i += pikkus+2;
        }
        else
        {
            i++;
        }
    }
}



#define LeiaLugejaDebug 1
char* LeiaLugeja(const char* tekst)
{
    #if LeiaLugejaDebug == 1
    prindiTaane();
    printf("Leiaugeja\n");
    prindiTaane();
    printf("SISSE: %s.\n", tekst);
    rekursiooniTase+=1;
    #endif

    unsigned int i=0;
    for ( ; tekst[i] != '/'; )
    {
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            unsigned int sulupikkus = strlen(sulusisu);
            free(sulusisu);
            i+=sulupikkus+2;
            continue;
        }
        i++;
    }
    char* lugeja = malloc(i+1);
    memcpy(lugeja, tekst, i+1);
    lugeja[i] = '\0';
    #if LeiaLugejaDebug == 1
    rekursiooniTase-=1;
    prindiTaane();
    printf("VÄLJA: %s\n", lugeja);
    #endif
    return lugeja;
}



#define KasLugejaDebug 1
int KasLugeja(const char* tekst) // nin(x)/sin(x + 4)abc     va(4 sin(x)x)/sin(x + 4)abc
{
    #if KasLugejaDebug == 1
    prindiTaane();
    printf("KasLugeja\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
    #endif
    for (unsigned int i = 0; tekst[i]!='/';)
    {
        if(tekst[i]=='\0')
        {
            #if KasLugejaDebug == 1
            rekursiooniTase-=1;
            prindiTaane();
            printf("VÄLJA: 0\n");
            #endif
            return 0;
        }
        else if (tekst[i] == ' ')
        {
            #if KasLugejaDebug == 1
            rekursiooniTase-=1;
            prindiTaane();
            printf("VÄLJA: 0\n");
            #endif
            return 0;
        }
        else if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            unsigned int pikkus = strlen(sulusisu);
            free(sulusisu);
            i += pikkus+2;
        }
        else
        {
            i++;
        }
    }
    #if KasLugejaDebug == 1
    rekursiooniTase-=1;
    prindiTaane();
    printf("VÄLJA: 1\n");
    #endif
    return 1;
}



// Argumendi võtmine käib nii, et tagastatakse tekst enne esimest tühikut välja arvatud juhul kui see tühik on mingite sulgude sees. See ei ole aga ainus viis argumenti võtta, sest näitekst astmed tahavad veidi teistsugust argumendivõtmist. Teksti ax^2+bx+c peab tõlgendama nii, et ax^(2)+bx+c, mitte ax^(2+bx+c) nagu esimene variant tõlgendaks. Seega on olemas kaks argumendivõtmise funtksiooni. Üks on tavaline LeiaArgument, teine on LeiaLühemArgument selline, mis lõpetab argumendivõtu mitte ainult tühiku peale vaid ka tehete +-* peale. Jagamismärki selles nimekirjas pole, sest jagamise argumendid selgitatakse välja enne astmete omi, mistõttu jagamismärki ei tule kunagi astme argumendi otsimisel ette.
#define LeiaArgumentDebug 0
char* LeiaArgument(const char* tekst)
{
    #if LeiaArgumentDebug == 1
    printf("LeiaArgument\n");
    printf("  SISSE: %s\n", tekst);
    #endif
    for (unsigned int i=0; ; )
    {
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            i += strlen(sulusisu)+2;
            free(sulusisu);
            continue;
        }
        if  (tekst[i] == ' ' || tekst[i]=='\0')
        {
            char* argument = malloc(i+1);
            memcpy(argument, tekst, i);
            argument[i]='\0';

            #if LeiaArgumentDebug == 1
            printf("  VÄLJA: %s\n", argument);
            #endif
            return argument;
        }
        i++;
    }
}

// Identne eelmise funktsooniga, lic argumendi võtmise lõpetamise tingimusi on rohkem.
#define LeiaLühemArgumentDebug 0
char* LeiaLühemArgument(const char* tekst)
{
    #if LeiaLühemArgumentDebug == 1
    printf("LeiaLühemArgument\n");
    printf("  SISSE: %s\n", tekst);
    #endif
    for (unsigned int i=0; ; )
    {
        if (tekst[i] == '(')
        {
            char* sulusisu = LeiaSuluSisu(&tekst[i+1]);
            i += strlen(sulusisu)+2;
            free(sulusisu);
            continue;
        }
        if  (tekst[i] == '+' || tekst[i] == '-' || tekst[i] == '*' || tekst[i] == ' ' || tekst[i]=='\0')
        {
            char* argument = malloc(i+1);
            memcpy(argument, tekst, i);
            argument[i]='\0';

            #if LeiaLühemArgumentDebug == 1
            printf("  VÄLJA: %s\n", argument);
            #endif
            return argument;
        }
        i++;
    }
}