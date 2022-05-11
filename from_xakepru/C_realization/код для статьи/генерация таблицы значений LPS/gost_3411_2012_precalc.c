#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "gost_3411_2012_precalc_const.h"

void
generate_tables()
{
    FILE *file;
    int j, i, k;
    file = fopen("precalctable.txt", "w");
    for(j = 0; j < 8; j++)
    {
        fprintf(file, "{");
        for(i = 0; i < 256; i++)
        {
            if (i)
                fprintf(file, ", ");
            if ((i % 4) == 0)
                fprintf(file, "\n\t");
            uint8_t t = Pi[i];
            uint64_t a = 0x00;
            for(k = 0; k < 8; k++)
            {
                if (t & 1)
                    a ^= A[63 - 8 * j - k];
                t >>= 1;
            }
            fprintf(file, "0x");
                fprintf(file, "%016llx", a);
        }
        if (j != 7)
            fprintf(file, ",\n},\n");
    }
    fprintf(file, "\n}\n");
    fclose(file);
}


int main(int argc, char *argv[])
{
    generate_tables();
}
