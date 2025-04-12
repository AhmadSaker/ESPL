#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char encode_char(char ch, int shift, int is_addition) // I made this function to encode each char
{
    if (ch >= 'a' && ch <= 'z')
    {
        if (is_addition)
        {
            return ((ch - 'a' + shift) % 26) + 'a';
        }
        else
        {
            return ((ch - 'a' - shift + 26) % 26) + 'a';
        }
    }
    else if (ch >= '0' && ch <= '9')
    {
        if (is_addition)
            return ((ch - '0' + shift) % 10) + '0';
        else
            return ((ch - '0' - shift + 10) % 10) + '0';
    }
    else
    {
        return ch;
    }
}

void encode(FILE *input, FILE *output, int encoding_mode, const char *encodeStr) // this function encode using the encode_char function
{
    int inputCh, i = 0;

    while ((inputCh = fgetc(input)) != EOF)
    {
        if (encodeStr[i] == '\0')
        {
            i = 0;
        }

        char outputCh = encode_char(inputCh, encodeStr[i] - '0', encoding_mode == 1);

        i = (i + 1) % strlen(encodeStr);

        fputc(outputCh, output);
    }
}




int main(int argc, char **argv)
{
    FILE *In_File = stdin;
    FILE *Out_File = stdout;
    const char *encodeStr = NULL;
    int debug_mode = 1;
    int encoding_mode = 0;

    for (int i = 0; i < argc; i++)
    {
        int len = strlen(argv[i]);

        if (len > 2 && argv[i][0] == '-' && argv[i][1] == 'D')
        {
            debug_mode = 0;
        }
        else if (len > 2 && argv[i][0] == '+' && argv[i][1] == 'D')
        {
            debug_mode = 1;
        }
        if (debug_mode)
        {
            fprintf(stderr, "%s\n", argv[i]);
        }
        else if (len > 2 && argv[i][0] == '+' && argv[i][1] == 'e')
        {
            encoding_mode = 1;
           encodeStr = argv[i] + 2;
        }
        else if (len > 2 && argv[i][0] == '-' && argv[i][1] == 'e')
        {
            encoding_mode = -1;
            encodeStr = argv[i] + 2;
        }
        else if (len > 2 && argv[i][0] == '-' && argv[i][1] == 'O')
        {
            Out_File = fopen(argv[i] + 2, "w");
        }
        else if (len > 2 && argv[i][0] == '-' && argv[i][1] == 'I')
        {
            In_File = fopen(argv[i] + 2, "r");
            if (!In_File)
            {
                fprintf(stderr, "Error: Cannot open input file\n");
                return 1;
            }
        }

        
    }

    if (encoding_mode != 0)
    {
        encode(In_File, Out_File, encoding_mode, encodeStr);
    }

    fclose(In_File);
    fclose(Out_File);
}

