#ifndef Matatõlge_h
#define Matatõlge_h

struct LimiTagastus
{
    char* Tõlge;
    unsigned int TähtiLoeti;
};

struct TekstArv
{
    char* Tekst;
    unsigned int Arv;
};



/* Kasutaja on config.txt failis defineerinud käske. Iga käsu kirjutis tõlgitakse Käsu struktuuriks. Käsu struktuurid sisaldavad kõike vajalikku infot käsu kohta: käsu nimi, argumentide tüübid, argumentide nimed, definitsioon – täpsemalt kirjeldatud allpool. Kõigist config.txt failist saadud käsustructidest koostatakse nimekiri enne seda, kui peamine tälkimistsükkel käima läheb. Peamise tsükli käigus iga kord, kui liigutakse uuele tähele, siis käiakse üle loodud käsunimekirja ja kontrollitakse ega mõni käsk sellelt tähekohalt alga. Kui algab, siis kutsutakse välja TõlgiKäsk funktsioon, millele antakse avastatud käsu struct, et see teaks, kuidas õigesti tõlkida.
1) Käsu nimi on see, mida kasutaja tahab lähtekoodis kijrutada, et oma käsku välja kutsuda. 
2) Argumentide tüübid on nimekiri nii mitmest elemendist+1, kui palju kasutaja oma käsule definitsioonis andis. Elemendid saavad selles nimekirjas olla kas 0 või 1. 0 tähistab seda, kui järjekorras vastaval kohal olev argument on lühem: argumendi lõppu tähistab lähtekoodis tühik või suvaline tehtemärk. 1 tähistab seda, kui vastaval kohal olev argument on pikem: lähtekoodis lõpetab seda argumenti ainult tühik.
3) Argumentide nimed on nimekiri kõigist argumentide tähistustest, mida kasutaja oma argumentidele andis, et neid siduda oma käsu deklaratsiooni ja definitsiooni vahel.
4) Definitsioon on tekst, mis config.txt failis on -> märgi järel. See tekst on latexi kood, milles mõnes kohas on kasutaja argumentide tähist. */
struct Käsk
{
    const char* käsunimi;
    int* argumentideTüübid;
    const char** argumentideNimed;
    char* definitsioon;
    unsigned int argumentideKogus;
};



// A dynamic array of Käsk structures
struct KäskList {
    struct Käsk* käsud;   // Array of Käsk
    size_t count;         // Number of commands
    size_t capacity;      // Array capacity
};

struct Environment {
    const char *name;        // Name of the environment
    struct KäskList käsk_list; // List of Käsk structs associated with the environment
    int body;                //If the environment has a body field
    int nest;                //If the environment has a nest field
    int multiline;
    const char *beginDefine; // Content inside the \begin{}
    const char *endDefine;   // Content inside the \end{}
    const char *Content;     // Everything from \begin{} to \end{} (including)
    const char *endText;     // The text we search for in our source code to end the env.
};

// The global struct of all defined environments
struct EnvironmentList {
    struct Environment* environments; // Dynamic array of environments
    size_t count;                     // Number of environments
    size_t capacity;                  // Capacity of the array
};

struct Käsk* KasKäsk(const char* tekst);
int TõlgiEnvironment(const struct Environment* env, FILE* input, FILE* output_file);

void print_environment_info(struct Environment* env);

void read_environments_from_config(const char* filepath, struct EnvironmentList* env_list);
struct Environment* KasEnvironment(const char* tekst);
void free_environment_list(struct EnvironmentList* list);
void add_environment(struct EnvironmentList* list, struct Environment env);
void init_environment_list(struct EnvironmentList* list);

void free_environment(struct Environment* env);
void init_environment(struct Environment* env);
void parse_environment(const char *config_line, struct Environment* env);
void parse_flags_in_brackets(const char* config_line, struct Environment* env);
void extract_between(const char *source, const char *start, const char *end, char *result, int max_len);

struct TekstTekst
{
    const char* Tekst1;
    const char* Tekst2;
};



struct Keskkond
{
    const char* alguseTähis;
    const char* alguseLatex;
    const char* igaReaAlgusesse; 
    struct TekstTekst* tekstJaAsendus;
    const char* igaReaLõppu;
    const char* lõpuTähis;
    const char* lõpuLatex;
};



struct KeskkonnaNimekiri
{
    struct Keskkond* keskkonnad;
    size_t count;
};



void read_commands_from_config(const char* filepath, struct KäskList* käsk_list);
void add_käsk(struct KäskList* list, struct Käsk subcommand);
void free_käsk_list(struct KäskList* list);
void init_käsk_list(struct KäskList* list);
char* read_line(FILE* file);
int KasEsimesedTähed(const char* tekstis, const char* tekst);
char* LeiaSuluSisu(const char* tekst);
char* LiidaTekstid(char* eelmineMälu, const char* lisatav);
char* my_strndup(const char* s, size_t n);
char* append_str(const char* a, const char* b);
char* TõlgiMathMode(const char* expression);
char* LeiaTekstEnneTähte(const char* tekst, char täht);
struct LimiTagastus TõlgiLim(char* tekst);
char* LeiaTekstEnneTeksti(const char* tekst, const char* teksti);
int KasLugeja(const char* tekst);
char* LeiaNimetaja(const char* tekst);
struct TekstArv TõlgiAste(const char* tekst);
char* LeiaLugeja(const char* tekst);
char* EemaldaEsimeneViimane(char* tekst);
int KasAvaldiseÜmberOnSulud(const char* tekst);
char* LeiaArgument(const char* tekst);
char* LeiaLühemArgument(const char* tekst);
char* KõrvutiolevadAstmeks(const char* tekst);
char* LiidaTäht(char* eelmineMälu, char lisatav);
char* LiidaArv(char* eelmineMälu, int lisatav);
struct TekstArv TõlgiKäsk(const char* tekst, struct Käsk* käsk);
char* trim_whitespace(char* str);

#endif