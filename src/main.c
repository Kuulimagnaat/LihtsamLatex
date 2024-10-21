#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <Windows.h>

#define INITIAL_BUFFER_SIZE 256

/* PSEUDO-KOODI NOTATSIOONI IDEED 
\newcommand{\ekv}{\;\ensuremath{\Leftrightarrow}\;}
\newcommand{\imp}{\;\ensuremath{\Rightarrow}\;}

\newcommand{\ja}{\quad\text{ja}\quad}
\newcommand{\siis}{\quad\text{siis}\quad}

*/


/* Function to convert fractions to LaTeX format */
void convert_fraction(const char* input, char* output) {
    char numerator[128] = {0}, denominator[128] = {0};
    sscanf(input, "%[^/]/%s", numerator, denominator);  // Extract numerator and denominator
    sprintf(output, "\\dfrac{%s}{%s}", numerator, denominator);  // Create LaTeX fraction
}

/* Check if the string is a fraction */
int is_fraction(const char* str) {
    return strchr(str, '/') != NULL; // Check for '/'
}

/* Function to check if string is bold text */
int is_bold_text(const char* str) {
    return str[0] == '*' && str[1] == '*' && str[strlen(str)-2] == '*' && str[strlen(str)-1] == '*';
}

/* Function to convert bold text to LaTeX format */
void convert_bold(const char* input, char* output) {
    char bold_text[256] = {0};
    sscanf(input, "**%[^*]**", bold_text);  // Extract text between "** **"
    sprintf(output, "\\textbf{%s}", bold_text);  // Create LaTeX bold text
}

/* Dynamic line reading function */
char* read_line(FILE* file) {
    char* line = malloc(INITIAL_BUFFER_SIZE);
    if (!line) {
        perror("Memory allocation error :(");
        exit(EXIT_FAILURE);
    }

    int capacity = INITIAL_BUFFER_SIZE;
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

/* Main function */
int main(int argc, char **argv) {
    SetConsoleOutputCP(CP_UTF8);
    
    // Open the input .txt file for reading
    FILE *file = fopen("src/main.txt", "r");
    if (file == NULL) {
        perror("Unable to open input file");
        return EXIT_FAILURE;
    }

    // Open the template file for reading
    FILE *template_file = fopen("templates/defaultTemplate.txt", "r");
    if (template_file == NULL) {
        perror("Unable to open template file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Open the output .tex file for writing
    FILE *output_file = fopen("output/output.tex", "w");
    if (output_file == NULL) {
        perror("Error creating output file");
        fclose(file);
        fclose(template_file);
        return EXIT_FAILURE;
    }

    char* line; // Pointer for the line
    int in_math_mode = 0;

    // Read the template line by line and write to output
    while ((line = read_line(template_file)) != NULL) {
        // Check if the line contains a placeholder for content
        if (strcmp(line, "{{content}}") == 0) {
            // Process the main.txt content
            while ((line = read_line(file)) != NULL) {
                char *token = strtok(line, " ");
                int has_text_before = 0; // Has text before math mode

                while (token != NULL) {
                    char result[512];

                    if (strcmp(token, "mm") == 0) {
                        if (!in_math_mode) {
                            // Enter math mode
                            if (has_text_before) {
                                fprintf(output_file, "$"); // Inline math
                            } else {
                                fprintf(output_file, "\\[ "); // Display math
                            }
                            in_math_mode = 1;
                        } else {
                            // Exit math mode
                            if (has_text_before) {
                                fprintf(output_file, "$ ");
                            } else {
                                fprintf(output_file, "\\] ");
                            }
                            in_math_mode = 0;
                        }
                    } else {
                        // Regular text or math
                        if (in_math_mode) {
                            if (is_fraction(token)) {
                                convert_fraction(token, result);
                                fprintf(output_file, "%s ", result);
                            } else if (is_bold_text(token)) {
                                convert_bold(token, result);
                                fprintf(output_file, "%s ", result);
                            } else {
                                fprintf(output_file, "%s ", token); // Regular text in math mode
                            }
                        } else {
                            if (is_bold_text(token)) {
                                convert_bold(token, result);
                                fprintf(output_file, "%s ", result);   
                            } else {
                                fprintf(output_file, "%s ", token); // Regular text
                            }
                            has_text_before = 1;
                        }
                    }
                    token = strtok(NULL, " ");
                }

                // Close math mode at the end of the line
                if (in_math_mode) {
                    if (has_text_before) {
                        fprintf(output_file, "$");
                    } else {
                        fprintf(output_file, "\\] ");
                    }
                }

                fprintf(output_file, "\n"); // New line for output
                free(line); // Free dynamically allocated line
            }
        } else {
            // Write the current line from the template
            fprintf(output_file, "%s\n", line);
        }
        free(line); // Free dynamically allocated template line
    }

    // Close files
    fclose(file);
    fclose(template_file);
    fclose(output_file);

    printf("Conversion completed. Output written to output.tex\n");
    return EXIT_SUCCESS;
}