#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Headers/Matatõlge.h"

// mingi list tuntud funktsioonidest
const char* math_functions[] = {"sin", "cos", "tan", "log", "ln", "sqrt", NULL};

// duplikeerib antud stringi kuni n baidini (tagastab pointeri uuele stringile)
char* my_strndup(const char* s, size_t n) {
    char* result;
    size_t len = strlen(s);

    if (n < len) {
        len = n;
    }

    result = (char*)malloc(len + 1); // Mälu substringi jaoks
    if (!result) {
        return NULL;
    }

    strncpy(result, s, len); // Kopeerime kuni n characteri
    result[len] = '\0'; //Null-terminate
    return result;
}

// Kas antud string on listis olev funktsioon
int is_math_function(const char* str) {
    for (int i = 0; math_functions[i] != NULL; i++) {
        if (strncmp(str, math_functions[i], strlen(math_functions[i])) == 0) {
            return 1;
        }
    }
    return 0;
}

// Abimeetod, mis appendib kaks stringi
char* append_str(const char* a, const char* b) {
    char* result = (char*)malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

// Rekursiivselt tõlgime math moodi latexisse
char* TõlgiMathMode2(const char* expression) {
    char* result = (char*)malloc(1); // Tühi string
    result[0] = '\0';
    
    int i = 0;
    while (expression[i] != '\0') {
        // Kontrollime kas on mõni tuntud käsk
        if (is_math_function(&expression[i])) {
            // Leiame käsu nime pikkuse
            int func_len = 0;
            while (expression[i + func_len] != '(' && expression[i + func_len] != '\0') {
                func_len++;
            }

            //Leiame ka funktsiooni nime
            //Kopeerime leitud funktsiooni nime, et siis saaksime seda kasutada kui eraldi "muutujat". Me ei taha modifitseerida algset *expression* stringi.
            //Samuti saame selle eluaega ise määrata, st. vabastada, millal soovime
            char* func_name = my_strndup(&expression[i], func_len);
            
            // Lisame vajaliku latex süntaksi
            char* latex_func_with_left = append_str(append_str("\\", func_name), "\\left(");
            free(func_name);

            result = append_str(result, latex_func_with_left);
            free(latex_func_with_left);

            i += func_len; // Liigume mööda antud käsust

            // Näpime seda argumenti sulgude vahel
            if (expression[i] == '(') {
                int start = i + 1;
                int paren_count = 1;
                i++;

                // Leiame sulud
                while (expression[i] != '\0' && paren_count > 0) {
                    if (expression[i] == '(') paren_count++;
                    else if (expression[i] == ')') paren_count--;
                    i++;
                }

                // Leiame rekursiivselt sisu
                char* inner_expression = my_strndup(&expression[start], i - start - 1);
                char* inner_latex = TõlgiMathMode2(inner_expression);
                free(inner_expression);

                result = append_str(result, inner_latex);
                free(inner_latex);

                result = append_str(result, "\\right)");
            }
        } else {
            // Kõik ülejäänud pask läheb tavaliselt
            char* single_char = my_strndup(&expression[i], 1);
            result = append_str(result, single_char);
            free(single_char);
            i++;
        }
    }

    return result;
}
