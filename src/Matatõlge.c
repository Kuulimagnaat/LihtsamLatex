#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"
#include <math.h>
#include <ctype.h>
#include "Headers/Abifunktsioonid.h"
#include "Headers/Kõigetõlge.h"
#include "Windows.h"

extern struct KäskList käskList;
extern struct EnvironmentList environList;
extern int reanumber;
extern struct TextmodeKäskList textmodeKäskList;

// Muutuja, mis hoiab endas seda infot, kui sügaval rekursiooniga ollakse. Võimaldab printida sügavusele vastavalt tühkuid debug sõnumite ette, et oleks kenam.
extern unsigned int rekursiooniTase;
// See kui mitu tühikut on taande pikkus.
extern unsigned int taandePikkus;





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

        char* token = strtok(trimmed_flags, ",");
        while (token != NULL) {
            token = trim_whitespace(token);

            if (strcmp(token, "body") == 0) {
                env->body = 1;
            } else if (strcmp(token, "nest") == 0) {
                env->nest = 1;
            } else if (strcmp(token, "multiline") == 0) {
                env->multiline = 1;
            } else if (strncmp(token, "end:{", 5) == 0) {
                // Extract the value inside end:{...}
                const char* end_start = strchr(token, '{');
                const char* end_end = strchr(token, '}');
                if (end_start && end_end && end_end > end_start) {
                    size_t length = end_end - end_start - 1;

                    char temp[length + 1];
                    strncpy(temp, end_start + 1, length);
                    temp[length] = '\0'; // Null-terminate the string

                    env->endText = strdup(temp);
                }
            }

            token = strtok(NULL, ",");
        }

    } else {
        // If no square brackets, keep default values (both flags are 0)
        env->body = 0;
        env->nest = 0;
        env->multiline = 0;
        env->endText = NULL;
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
    
    // GET THE ENVIRONMENT BEGIN AND END DEFINITIONS
    begin_pos = strstr(config_line, "\\begin{");
    end_pos = strstr(config_line, "\\end{");

    if (begin_pos && end_pos) {
        // Find the entire "\begin{...}" substring
        const char* begin_end_pos = strchr(begin_pos, '}');
        if (begin_end_pos) {
            size_t begin_len = begin_end_pos - begin_pos + 1; // Include the closing brace '}'
            char* begin_content = malloc(begin_len + 1);
            strncpy(begin_content, begin_pos, begin_len);
            begin_content[begin_len] = '\0';
            env->beginDefine = begin_content;
        }

        // Find the entire "\end{...}" substring
        const char* end_end_pos = strchr(end_pos, '}');
        if (end_end_pos) {
            size_t end_len = end_end_pos - end_pos + 1; // Include the closing brace '}'
            char* end_content = malloc(end_len + 1);
            strncpy(end_content, end_pos, end_len);
            end_content[end_len] = '\0';
            env->endDefine = end_content;
        }
    }
    
    // GET THE ENVIRONMENT NAME
    const char* start = config_line;
    while (*start == ' ' || *start == '\t') {
        start++; // Skip leading whitespace
    }
    const char* end = strchr(start, '[');
    if (end) {
        size_t name_len = end - start;
        char name[name_len + 1];
        strncpy(name, start, name_len);
        name[name_len] = '\0'; // Null-terminate the string
        char * nameReal = strdup(name);
        nameReal = trim_whitespace(nameReal);
        env->name = nameReal; // Allocate and assign the environment name
    } else {
        printf("Error: Environment name not found or malformed configuration.\n");
        env->name = NULL;
    }
    
    // THE BODY, NEST DETECTION PART AND OTHER STARTS HERE
    parse_flags_in_brackets(config_line, env);

    

    // THE SUBCOMMAND DETECTION PART STARTS HERE
    const char *definitions_start = strchr(config_line, '|');
    if (!definitions_start) {
        printf("No subcommand definitions found.\n");
        return;
    }
    definitions_start++; // Move past the '|'
    
    

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

        if (*current_pos == '\0' || open_parens != 0) {
            printf("Error: Unmatched parentheses in subcommand definition.\n");
            break;
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
        char command_name[100]; // Assuming command names are up to 49 characters
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

        

        // CONSTRUCTION OF THE KÄSKLIST

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
    env->multiline = 0;
    env->beginDefine = NULL;
    env->endDefine = NULL;
    env->Content = NULL;
    env->endText = NULL;
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

    unsigned int i = 0;
    for (; tekst[i]!='\0'; i++)
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
    prindiVärviga("Sulud ei klapi.", "punane");
    if (sulutase == 1)
    {
        printf(" Rida %d. Tõlgitakse edasi justkui oleks 1 puuduolev sulg rea lõpus.", reanumber);
    }
    else
    {
        printf(" Rida %d. Tõlgitakse edasi justkui oleks %d puuduolevat sulgu rea lõpus.", reanumber, sulutase);
    }
    mälu[i] = '\0';
    #if LeiaSuluSisuDebug == 1
    rekursiooniTase -= 1;
    prindiTaane();
    printf("VÄLJA: %s\n", mälu);
    #endif
    return mälu;
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
    if (!eelmineMälu || !lisatav) {
        fprintf(stderr, "LiidaTekstid error: NULL input provided.\n");
        exit(EXIT_FAILURE);
    }
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
    if (str == NULL) {
        fprintf(stderr, "Error: Null string passed to trim_whitespace\n");
        return NULL;
    }


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
    if (tekst == NULL) {
        fprintf(stderr, "Error: Memory allocation failed in trim_whitespace\n");
        return NULL;
    }
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
void add_käsk(struct KäskList* list, struct Käsk subcommand) {
    // Ensure the list has enough capacity
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;

        struct Käsk* new_käsud = realloc(list->käsud, new_capacity * sizeof(struct Käsk));
        if (!new_käsud) {
            fprintf(stderr, "Memory allocation failed in add_käsk\n");
            exit(EXIT_FAILURE); // Exit gracefully on allocation failure
        }
        list->käsud = new_käsud;
        list->capacity = new_capacity;
    }

    // Add the subcommand to the list
    list->käsud[list->count] = subcommand;
    list->count++;
}

void read_commands_from_config(const char* filepath, struct KäskList* käsk_list)
{
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Unable to open config file");
        exit(EXIT_FAILURE);
    }

    char* line;
    int onMathmodeKäskudeJuures = 0;
    // Loetakse config faili ridahaaval. Hiljem võiks teha nagu lähtekoodifailigagi et enne lugeda pikka sõnesse ja ss seda analüüsida.
    while ((line = read_line(file)) != NULL) 
    {
        if (KasEsimesedTähed(line, "MATHMODE KÄSUD"))
        {
            onMathmodeKäskudeJuures = 1;
            continue;
        }
        if (KasEsimesedTähed(line, "KESKKONNAD") || KasEsimesedTähed(line, "TEXTMODE KÄSUD") || KasEsimesedTähed(line, "TEMPLATE FAIL"))
        {
            onMathmodeKäskudeJuures = 0;
            continue;
        }
        // Skip empty lines or comments and environment lines
        if (line[0] == '\0' || line[0] == '#' || KasEsimesedTähed(line, "env"))
        {
            free(line);
            continue;
        }
        if (onMathmodeKäskudeJuures)
        {
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
                            käsk.argumentideTüübid[käsk.argumentideKogus-1] = 1;
                        }
                        else if (left[i] == '[')
                        {
                            käsk.argumentideTüübid[käsk.argumentideKogus-1] = 0;
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

            // Add the parsed command to the list
            add_käsk(käsk_list, käsk);
        }

        // Free the line buffer
        free(line);
    }

    fclose(file);
}


// Kui TõlgiMathMode funktsioonis tajutakse, et kättejõudnud kohal on mingi käsk, siis seal kohas antakse selle koha aadress ja tajutud käsule vastav struct selele funtksiooile, et see saaks tõlkida seda kohta. 
#define TõlgiKäskDebug 0
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
    // Ütleme, et ei tohi nii olla ja ütleme, et ei tohi isegi olla käsu definitsioonid sellised: uuga(arg1)buuga(arg2) MUIDE: projekt on peaaegu valmis ja programmi sai juurde lisatud textmodecommandid, mida on võimalik kasutajal defineerida nimetatud kujul uuga(arg1)buuga(arg2). Mathmodecommandid on ses mõttes nendega võrreldes vähem võimekamad. Erinevus tuleb sellest, et tõlgimathmode kood on vana ja selle kirjutamise ajal olime lollimad. Siiski, fracilaadset argumendiga algavat käsku kujul (arg1)uuga(arg2) ei saa isegi textmodekäskudega defineerida frac ikkagi on ainus omalaadne MUIDELÕPP. Postuleerime, et kõik argumendid peavad olema kõige lõpus igal commandil ja loodame selle peale, et me ei taha tulevikus midagi muud fracilaadset programmi lisada. frac olgu ainus omalaadne.

    // Kõik argumendid selles nimekirjas ja hiljem see nimekiri ise on vaja vabastada.*/
    char** argumentideTõlked = malloc(käsk->argumentideKogus*sizeof(char*));
    unsigned int i = strlen(käsk->käsunimi);

    // Tsükkel, mis otsib ja tõlgib argumente. Iga käiguga üks argument.
    for (unsigned int j=0; j<käsk->argumentideKogus; j++)
    {
        char* argument = NULL;
        if (käsk->argumentideTüübid[j] == 0)
        {
            argument = LeiaLühemArgument(&tekst[i]);
            // Tegu on lühema argumendiga, seega see ei sisalda tühikut, mistõttu kasutaja ei kasuta sulge, mistõttu ei pea sulukontrolli tegema nagu pikema argumendiga allpool.
            i += strlen(argument);
        }
        else if (käsk->argumentideTüübid[j] == 1)
        {
            argument = LeiaArgument(&tekst[i]);
            
            if (KasAvaldiseÜmberOnSulud(argument))
            {
                i += strlen(argument)+1;
                char* sulgudeta = EemaldaEsimeneViimane(argument);
                puts("SULGUDETA!");
                puts(sulgudeta);
                free(argument);
                argument = sulgudeta;
            }
            else
            {
                i += strlen(argument)+1;
            } 
        }
        argumentideTõlked[j] = TõlgiMathMode(argument);
        
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
        size_t new_capacity = list->capacity * 2;
        struct Environment* new_environments = realloc(list->environments, new_capacity * sizeof(struct Environment));
        if (!new_environments) {
            perror("Failed to resize EnvironmentList");
            exit(EXIT_FAILURE);
        }
        list->environments = new_environments;
        list->capacity = new_capacity;
    }
    list->environments[list->count++] = env; // Add the new environment.
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
    return NULL;
}

struct Käsk* KasEnvironmentKäsk(const char* tekst, const struct Environment* env) {
    for (unsigned int i = 0; i < env->käsk_list.count; i++) {
        if (KasEsimesedTähed(tekst, env->käsk_list.käsud[i].käsunimi)) {
        
            return &env->käsk_list.käsud[i];
        }
    }
    return NULL;
}

char** SplitByDelimiter(const char* input)
{
    if (!input) return NULL;

    // Allocate memory for the result list (estimate large enough for simplicity)
    char** parts = malloc(128 * sizeof(char*)); // 128 max parts (arbitrary limit for simplicity)
    if (!parts) return NULL;

    int index = 0;
    const char* start = input;
    const char* triple_space = strstr(start, "   "); // Find first triple space

    while (triple_space) {
        // Calculate the length of this segment
        size_t segment_length = triple_space - start;

        // Allocate memory and copy this segment
        parts[index] = malloc(segment_length + 1);
        strncpy(parts[index], start, segment_length);
        parts[index][segment_length] = '\0'; // Null-terminate

        index++;
        start = triple_space + 3; // Move past the triple spaces
        triple_space = strstr(start, "   "); // Find the next triple space
    }

    // Add the remaining part of the string (or the whole string if no triple spaces were found)
    parts[index] = strdup(start);
    index++;

    // Null-terminate the array
    parts[index] = NULL;
    return parts;
}

char* ReplaceArgumentInDefinition(char* definition, const char* placeholder, const char* translatedArg)
{
    // Find the placeholder in the definition
    char* pos = strstr(definition, placeholder);
    if (pos) {
        size_t placeholderLen = strlen(placeholder);
        size_t defLen = strlen(definition);
        size_t translatedArgLen = strlen(translatedArg);

        // Allocate new string for the final translation
        char* result = malloc(defLen - placeholderLen + translatedArgLen + 1);
        if (!result) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        // Copy the part before the placeholder
        size_t prefixLen = pos - definition;
        strncpy(result, definition, prefixLen);

        // Add the translated argument
        strcpy(result + prefixLen, translatedArg);

        // Copy the part after the placeholder
        strcpy(result + prefixLen + translatedArgLen, pos + placeholderLen);

        // Free the original definition and return the new result
        free(definition);
        return result;
    }

    return definition;  // If no placeholder is found, return the original definition
}


// Argument kutsuti välja mathmodest on 1 siis, kui see kutsuti välja mathmode sees olles ja 0 ss kui tavalises textmodes v muus mittemathmodes. 
struct TekstArv TõlgiEnvironment(char* tekst, struct Environment* env, int KutsutiVäljaMMist)
{       
    struct TekstArv tõlkeStruct = { NULL, 0 }; 
    char* tõlge = malloc(1);
    tõlge[0] = '\0';

    tõlge = LiidaTekstid(tõlge, env->beginDefine);
    tõlge = LiidaTekstid(tõlge, "\n"); // Pärast algust üks newline.
    char* keskkonnaSisu = LeiaTekstEnneTeksti(tekst, env->endText); //Alates keskkonna definitsioonist, lõppeb just ENNE --

    if (!keskkonnaSisu) {
        prindiVärviga("Keskkonna sisu ei leitud. Arvatavasti tähendab, et on viga keskkonda lõpetava sümboliga või selle leidmisega.", "punane");
        return tõlkeStruct;
    }

    int koguArv = strlen(keskkonnaSisu); // Terve environmenti sisu võrra liigume edasi lähtekoodis
    koguArv += strlen(env->endText)+1; // Kuna keskkonda lõpetavate sümbolite pikkus pole seal sees.
    tõlkeStruct.Arv = koguArv; 
    


    // MAIN TRANSLATION
    // Kui keskkonnal on multiline flag TRUE, siis on tarvis, et igale reale kirjutataks ette kasutatava subcommandi nimi (eg. it, k, jne.).
    // Kui keskkonnal ei ole multiline flagi, siis KUI keskkonnal on ainult üks subcommand defineeritud, siis võib iga rea algusest jätta subcommandi nime kirjutamata, sest igal real automaatselt eeldatakse, et sa kutsud seda välja.
    // Vastasel juhul pead ikka igal real subcommandi nime välja kirjutama (sest subcommande võib olla mitu).

    char* keskkonnaSisuTokeniteks = strdup(keskkonnaSisu); // Et esialgne keskkonna sisu mälu jääks alles

    char* line = strtok(keskkonnaSisuTokeniteks, "\n");
    line = strtok(NULL, "\n"); // Esimene rida on alati keskkonna nimi
    
    // Lets tõlk this bitch
    while (line != NULL) {
        if (env->multiline) {
            // Teame, et iga uue rea alguses (mis ei ole eelnevaga seotud), peab olema käsu nimi
            
            // Kontrollime, kas real algab mõni käsk
            // Kui käsku ei alga, ss on pohhui, sest me nkn oleme selle kinni püüdnud siis sellele reale eelneva (kusiganes see ka poleks) käsu reaga
            struct Käsk* subcmd = KasEnvironmentKäsk(line, env);
            if (subcmd) {
                // Leidsime real käsu. See oli kas esimene rida või meil on eelnevalt olnud teksti, mida on vaja nüüd tõlkida
                
                char* TekstPraegusestReast = strstr(keskkonnaSisu, line);
                TekstPraegusestReast += strlen(subcmd->käsunimi) + 1; // Et käsunimi välja jäetaks, +1 et tühikuga arvestada pärast käsunime
                
                if (TekstPraegusestReast) {
                    //char* TerveKäsuRida = LeiaTekstEnneTeksti(TekstPraegusestReast, subcmd->käsunimi);


                    // Leiame järgmise käsunime (See ei pea olema SAMA, mis välja kutsunud real!)
                    // Kõik järgnev kood on selleks, et korrektselt leida õige käsu rida antud käsu jaoks
                    char* KäsuRead[100];
                    unsigned int lineCount = 0;

                    for (unsigned int i = 0; i < env->käsk_list.count; i++) {
                        struct Käsk* JärgnevKäsk = &env->käsk_list.käsud[i];

                        char* TerveKäsuRida = LeiaTekstEnneTeksti(TekstPraegusestReast, JärgnevKäsk->käsunimi);
                        if (TerveKäsuRida) {
                            KäsuRead[lineCount++] = TerveKäsuRida;
                        }
                    }

                    char* TerveKäsuRida = NULL;
                    for (unsigned int i = 0; i < lineCount; i++) {
                        char* currentLine = KäsuRead[i];

                        int containsSubcommand = 0;
                        for (unsigned int n = 0; n < env->käsk_list.count; n++) {
                            struct Käsk* JärgnevKäsk = &env->käsk_list.käsud[n];
                            
                            if (strstr(currentLine, JärgnevKäsk->käsunimi)) {
                                containsSubcommand = 1;
                                break;
                            }
                        }

                        if (!containsSubcommand) {
                            TerveKäsuRida = strdup(currentLine);
                        }
                    }

                    for (unsigned int i = 0; i < lineCount; i++) {
                        free(KäsuRead[i]);
                    }

                    // Teeme väikese clean upi leitud teksti peal.
                    size_t len = strlen(TerveKäsuRida);
                    if (len >= 3 && TerveKäsuRida[len - 1] == '-' && TerveKäsuRida[len - 2] == '-' && TerveKäsuRida[len - 3] == '\n') {
                        TerveKäsuRida[len - 3] = '\0'; // Remove "\n--"
                    } else if (len >= 1 && TerveKäsuRida[len - 1] == '\n') {
                        TerveKäsuRida[len - 1] = '\0'; // Remove single '\n'
                    }
                    
                    // Nüüd tekib küsimus selles, et kas see terve käsurida sisaldab mitut argumenti, mis oli mõeldud välja kutsud käsu jaoks.
                    // Loome masiivi, mis hoiab leitud argumente.
                    char** arguments = SplitByDelimiter(TerveKäsuRida);
                    char* finalTranslation = strdup(subcmd->definitsioon);

                    for (unsigned int i = 0; i < subcmd->argumentideKogus; i++) {
                        // Oleme leidnud kõik argumendid, mida see subcommand endale soovib. Nüüd peame tõlkima igat ühte ja asetama need sobivale kohale definitsioonis.
                        char* tõlgitudArgument = NULL;
                        if (KutsutiVäljaMMist) 
                        {
                            tõlgitudArgument = TõlgiMathMode(arguments[i]);
                        }
                        else 
                        {
                            tõlgitudArgument = TõlgiKõik(arguments[i]);
                        }
                        
                        // Teeme placeholderi, kus me hakkame ükshaaval argumente asendama
                        char placeholder[50];
                        snprintf(placeholder, sizeof(placeholder), "%s", subcmd->argumentideNimed[i]);
                        
                        // Saame selle argumendi suhtes lõpliku tõlke.
                        finalTranslation = ReplaceArgumentInDefinition(finalTranslation, placeholder, tõlgitudArgument);
                        free(tõlgitudArgument);
                    }

                    tõlge = LiidaTekstid(tõlge, finalTranslation);
                    free(TerveKäsuRida);
                    free(finalTranslation);
                }
            }
        }
        else
        {
            // Tuleb välja, et keskkonnas pole multiline lubatud. Seega on võimalusi mitu: kui keskkonnas on subcommande ainult üks, siis ei pea ühelgil real märkima kasutavat käsku (aga võib). Kui käske on mitu, siis peab igal real seda kasutama.
            if (env->käsk_list.count == 1) {
                // Ainult üks subcommand on. Igal real VÕIB olla käsk defineeritud, kuid ei pea olema.
                struct Käsk* subcmd = KasEnvironmentKäsk(line, env);
                if (subcmd) {
                    // Real leiti käsk.
                    // Võtame rea algusest ära käsu nime (ja tühiku).
                    size_t subcmd_len = strlen(subcmd->käsunimi);
                    if (strncmp(line, subcmd->käsunimi, subcmd_len) == 0) {
                        line += subcmd_len + 1;
                    }

                    char ** arguments = SplitByDelimiter(line);
                    char* finalTranslation = strdup(subcmd->definitsioon);

                    for (unsigned int i = 0; i < subcmd->argumentideKogus; i++) {
                        char* tõlgitudArgument = NULL;
                        if (KutsutiVäljaMMist) 
                        {
                            tõlgitudArgument = TõlgiMathMode(arguments[i]);
                        }
                        else 
                        {
                            tõlgitudArgument = TõlgiKõik(arguments[i]);
                        }
                        
                        char placeholder[50];
                        snprintf(placeholder, sizeof(placeholder), "%s", subcmd->argumentideNimed[i]);
                        
                        finalTranslation = ReplaceArgumentInDefinition(finalTranslation, placeholder, tõlgitudArgument);
                        free(tõlgitudArgument);
                    }
                    tõlge = LiidaTekstid(tõlge, finalTranslation);
                    free(finalTranslation);

                } else {
                    // Real ei leitud käsku. Sellisel juhul ei ole real alguses defineeritud käsu nime.
                    struct Käsk* subcmd = &env->käsk_list.käsud[0]; // Leiame keskkonna ainsa käsu. 

                    char ** arguments = SplitByDelimiter(line);
                    char* finalTranslation = strdup(subcmd->definitsioon);

                    for (unsigned int i = 0; i < subcmd->argumentideKogus; i++) {
                        char* tõlgitudArgument = NULL;
                        if (KutsutiVäljaMMist) 
                        {
                            tõlgitudArgument = TõlgiMathMode(arguments[i]);
                        }
                        else 
                        {
                            tõlgitudArgument = TõlgiKõik(arguments[i]);
                        }
                        
                        char placeholder[50];
                        snprintf(placeholder, sizeof(placeholder), "%s", subcmd->argumentideNimed[i]);
                        
                        finalTranslation = ReplaceArgumentInDefinition(finalTranslation, placeholder, tõlgitudArgument);
                        free(tõlgitudArgument);
                    }
                    tõlge = LiidaTekstid(tõlge, finalTranslation);
                    free(finalTranslation);
                }
            }
            else 
            {
                // Mitu subcommandi on. Peame igal real leidma vastava commandi ja seda tõlkima.
                struct Käsk* subcmd = KasEnvironmentKäsk(line, env);
                if (subcmd) {
                    // Võtame rea algusest ära käsu nime (ja tühiku).
                    size_t subcmd_len = strlen(subcmd->käsunimi);
                    if (strncmp(line, subcmd->käsunimi, subcmd_len) == 0) {
                        line += subcmd_len + 1;
                    }

                    char** arguments = SplitByDelimiter(line);
                    char* finalTranslation = strdup(subcmd->definitsioon);

                    for (unsigned int i = 0; i < subcmd->argumentideKogus; i++) {
                        char* tõlgitudArgument = NULL;
                        if (KutsutiVäljaMMist) 
                        {
                            tõlgitudArgument = TõlgiMathMode(arguments[i]);
                        }
                        else 
                        {
                            tõlgitudArgument = TõlgiKõik(arguments[i]);
                        }

                        char placeholder[50];
                        snprintf(placeholder, sizeof(placeholder), "%s", subcmd->argumentideNimed[i]);

                        finalTranslation = ReplaceArgumentInDefinition(finalTranslation, placeholder, tõlgitudArgument);
                        free(tõlgitudArgument);
                    }
                    tõlge = LiidaTekstid(tõlge, finalTranslation);
                    free(finalTranslation);
                }
            }
        }
        line = strtok(NULL, "\n");
    }
    
    free(line);
    tõlge = LiidaTekstid(tõlge, "\n"); // Enne lõppu newline.
    tõlge = LiidaTekstid(tõlge, env->endDefine);

    free(keskkonnaSisuTokeniteks);
    free(keskkonnaSisu);
    tõlkeStruct.Tekst = tõlge;
    return tõlkeStruct;
}


void read_environments_from_config(const char* filepath, struct EnvironmentList* env_list) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        exit(EXIT_FAILURE);
    }

    char* line;
    int onKeskkondadeJuures = 0;
    while ((line = read_line(file)) != NULL) {
        // Skip empty line or kommentaarid yo
        if (line[0] == '\0' || line[0] == '#') {
            free(line);
            continue;
        }
        if (KasEsimesedTähed(line, "KESKKONNAD"))
        {
            onKeskkondadeJuures = 1;
            continue;
        }
        if (KasEsimesedTähed(line, "MATHMODE KÄSUD") || KasEsimesedTähed(line, "TEXTMODE KÄSUD") || KasEsimesedTähed(line, "TEMPLATE FAIL"))
        {
            onKeskkondadeJuures = 0;
            continue;
        }
        if (onKeskkondadeJuures)
        {
            // Siin tuleks env() süntaksi nõudmine keskkondade puhul ära jätta, sest on teada, et kui lugemine toimub, siis on tegu keskkondadega, sest loetakse KESKKONNAD lõiku.
            // Eeldame, et "env" kasutame AINULT siis, kui defineerime uut keskkonda
            //printf("Valid Environment Line: %s\n", line);
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
    printf("Multilines Allowed?: %d\n", env->multiline);
    printf("Begin Define: %s\n", env->beginDefine ? env->beginDefine : "Not found");
    printf("End Define: %s\n", env->endDefine ? env->endDefine : "Not found");
    printf("Content: %s\n", env->Content ? env->Content : "Not found");
    printf("Ending Text: %s\n", env->endText ? env->endText : "Not found");

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
    //prindiVärviga("KasKäsk\n", "roheline");
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
#define TõlgiMathModeDebug 0
#define VigadestTeatamine 0
char* TõlgiMathMode(const char* avaldis)
{
    #if TõlgiMathModeDebug == 1
    prindiTaane();
    //prindiVärviga("TõlgiMathMode\n", "roheline");
    printf("TõlgiMathMode\n");
    prindiTaane();
    printf("SISSE: %s\n", avaldis);
    rekursiooniTase += 1;
    #endif

    //printf("Main_path tõlgimathmodes: %s\n", main_path);

    char* result = malloc(1); // Empty string
    result[0] = '\0';

    unsigned int w = 0;
    // Avaldis on terve, võimalik et mitmerealine, tekst, mis jääb mm-de vahele.
    while (w < strlen(avaldis))
    {
        char* expression = LeiaTekstEnneTeksti(&avaldis[w], "\n");
        w += strlen(expression);
        unsigned int i = 0;
        // Kas tähe juurde jõudes oli eelnev asi käsk. Kui oli, siis tuleb tavalise tähe korral selle ette panna tühik.
        int eelmiselKohalOliKäsk = 0;
        int OnKeskkond = 0;
        while (i < strlen(expression))
        {
            #if VigadestTeatamine == 1
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
            #endif

            struct Environment* env = KasEnvironment(&expression[i]);
            if (env)
            {   
                OnKeskkond = 1;
                // Detect and process the entire environment content
                
                char* enviSisu = strstr(avaldis, &expression[i]);
                struct TekstArv envTõlge = TõlgiEnvironment(enviSisu, env, 1);
                
                // Add the translated environment to the result
                result = LiidaTekstid(result, envTõlge.Tekst);
                
                // Free memory for the environment's translated content
                free(envTõlge.Tekst);

                // Move the index forward to the next part of the expression
                w += envTõlge.Arv;
                break; // Skip the rest of the loop, as we've handled this environment
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
                char* nimetaja = LeiaArgument(&expression[i + strlen(lugeja) + 1]);
                
                i += strlen(nimetaja);
                if (KasAvaldiseÜmberOnSulud(nimetaja))
                {
                    char* sulgudeta = EemaldaEsimeneViimane(nimetaja);
                    puts("SULGUDETA!");
                    puts(sulgudeta);
                    free(nimetaja);
                    nimetaja = sulgudeta;
                }
                char* nimetajaTõlge = TõlgiMathMode(nimetaja);

                result = LiidaTekstid(result, "\\frac{");
                result = LiidaTekstid(result, lugejaTõlge);
                result = LiidaTekstid(result, "}{");
                result = LiidaTekstid(result, nimetajaTõlge);
                result = LiidaTekstid(result, "}");

                // Update the index
                i += strlen(lugeja) + 1;

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
            if (expression[i] == '[')
            {
                result = LiidaTekstid(result, "\\left[");
                i += 1;

                char* suluSisu = LeiaSuluSisu(&expression[i]);
                i += strlen(suluSisu);

                char* suluSisuTõlge = TõlgiMathMode(suluSisu);
                free(suluSisu);
                
                result = LiidaTekstid(result, suluSisuTõlge);
                free(suluSisuTõlge);

                result = LiidaTekstid(result, "\\right]");
                i+=1;
            }
            if (expression[i] == '{')
            {
                result = LiidaTekstid(result, "\\left\\{");
                i += 1;

                char* suluSisu = LeiaSuluSisu(&expression[i]);
                i += strlen(suluSisu);

                char* suluSisuTõlge = TõlgiMathMode(suluSisu);
                free(suluSisu);
                
                result = LiidaTekstid(result, suluSisuTõlge);
                free(suluSisuTõlge);

                result = LiidaTekstid(result, "\\right\\}");
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
                
                eelmiselKohalOliKäsk = 1;
            }

            else 
            {
                // Vaja selleks, et panna tühik juhul, kui liidetav täht järgneb käsule. Kui ilma tühikuta panna, ss rikuks käsu ära, näiteks \lim_{x\toa}, kus \toa peaks olema \to a.
                if (eelmiselKohalOliKäsk && expression[i]!=' ')
                {
                    result = LiidaTekstid(result, " ");
                }
                eelmiselKohalOliKäsk = 0;

                // Handle regular characters
                char *single_char = malloc(2);
                if (single_char == NULL) {
                    perror("malloc failed for single char in Matatõlge.");
                    exit(1);
                }

                single_char[0] = expression[i];  // Copy the character
                single_char[1] = '\0';           // Null-terminate the string

                result = LiidaTekstid(result, single_char);
                free(single_char);
                i++;
            }
        }
        if (avaldis[w] != '\0' && !OnKeskkond)
        {
            w ++;
            result = LiidaTekstid(result, "\\\\\n");
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

    char* a = malloc(1);
    a = LiidaTekstid(a, "LmaoxdlolDab Nimetajat ei olnud bruh, stupid ass");
    return a;
}



#define LeiaLugejaDebug 0
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



#define KasLugejaDebug 0
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
    prindiTaane();
    //prindiVärviga("LeiaArgument\n", "roheline");
    printf("LeiaArgument\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
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
            rekursiooniTase-=1;
            prindiTaane();
            printf("VÄLJA: %s\n", argument);
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
    prindiTaane();
    //prindiVärviga("LeiaLühemArgument\n", "roheline");
    printf("LeiaLühemArgument\n");
    prindiTaane();
    printf("SISSE: %s\n", tekst);
    rekursiooniTase += 1;
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
        if  (tekst[i] == '+' || tekst[i] == '-' || tekst[i] == '*' || tekst[i] == ' ' || tekst[i] == '=' || tekst[i] == ',' || tekst[i]=='\0')
        {
            char* argument = malloc(i+1);
            memcpy(argument, tekst, i);
            argument[i]='\0';

            #if LeiaLühemArgumentDebug == 1
            rekursiooniTase-=1;
            prindiTaane();
            printf("VÄLJA: %s\n", argument);
            #endif
            return argument;
        }
        i++;
    }
}