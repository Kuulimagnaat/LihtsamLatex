#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    unsigned int kogumaht = 16;
    // Tüki maht on see kogus mälu baitides, mille võrra mälu juurde tellitakse, kui olemasolev mälu otsa saab. Sellesse mahutub neli tähte.
    unsigned int tukiMaht = 16;
    // funktsioon fgetc annab tähe andmetüübina int. Seda sellepärast, et inti suuremat mahtu on vaja unicode'i tähtede hoidmiseks. Selleks on mälu int* mitte char*. 
    int* malu = malloc(kogumaht);
    unsigned int kogus = 0;

    FILE* failiPtr = fopen("Matemaatiline.tex", "r");

    int taht;
    while((taht = fgetc(failiPtr)) != EOF)
    {
        if (kogus)

        malu[kogus] = taht;
        kogus += 1;
        printf("%c", taht);
    }

    //printf("Tere!");
}