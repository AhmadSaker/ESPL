#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MENU_LENGTH 6
#define BUFFERSIZE 10000

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link link;

struct link
{
    link *nextVirus;
    virus *vir;
};

typedef struct fun_desc
{
    char *name;
    void (*fun)();
} fundesc;

// task 0
void PrintHex(const unsigned char *buffer, size_t length, FILE *output);

// task 1a
int isBigEndian = 0; // default is LE
char sigFileName[256] = "signatures-L";

void SetSigFileName();
virus *readVirus(FILE *file);
void printVirus(virus *v, FILE *output);

// task 1b
void list_print(link *virus_list, FILE *file);
link *list_append(link *virus_list, virus *data);
void list_free(link *virus_list);
void listprint();
void LoadSignatures();
void DetectViruses();
void FixFile();
void Quit();

// helper func
void readviruses(FILE *file);

// global vars
link *virusList = NULL;
FILE *file_b = NULL;

// task 1c
void detect_virus(char *buffer, unsigned int size, link *virus_list);

// global var 1c
char buffer[BUFFERSIZE] = "";
char **temp_argv;
int temp_argc ;

int main(int argc, char **argv) 
{
    // FILE* file = fopen(sigFileName, "rb"); // test for task a1
    // if (!file) {
    //     perror("Failed to open file");
    //     exit(1);
    // }

    // char magicNum[4];
    // if (fread(magicNum , 1, 4, file) != 4) {
    //     perror("Failed to read magic number");
    //     fclose(file);
    //     exit(1);
    // }

    // if (strncmp(magicNum , "VIRL", 4) != 0 && strncmp(magicNum , "VIRB", 4) != 0) { // if not LE nor BE
    //     fprintf(stderr, "Invalid magic number \n");
    //     fclose(file);
    //     exit(1);
    // }

    // if (strncmp(magicNum , "VIRB", 4) == 0){ // checking if the magic number is BE
    //     isBigEndian = 1 ;
    // }

    // virus* v = readVirus(file); // reading the virus
    // printVirus(v , stdout);


    // task 1b,1c,2 ...

    temp_argc = argc;
    temp_argv = argv;

    fundesc menu[] = {
        {"Set signatures file name", &SetSigFileName},
        {"Load signatures", &LoadSignatures},
        {"Print signatures", &listprint},
        {"Detect viruses", &DetectViruses},
        {"Fix file", &FixFile},
        {"Quit", &Quit},
        {NULL, NULL} // End of menu
    };

    char input[100];

    while (1)
    {
        printf("Menu:\n");
        for (int i = 0; i < MENU_LENGTH; ++i)
        {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Enter option: ");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break; // Exit on EOF
        }

        int option;
        if (sscanf(input, "%d", &option) != 1)
        {
            printf("Invalid input\n");
            continue;
        }

        if (option >= 0 && option < 6)
        {
            menu[option].fun();
        }
        else
        {
            printf("Invalid option. Please try again.\n");
        }
    }

    // free(v); // for task a1
    // fclose(file);

    return 0;
}

// task0
void PrintHex(const unsigned char *buffer, size_t length, FILE *output)
{
    for (size_t i = 0; i < length; ++i)
    {
        fprintf(output, "%02x ", buffer[i]); // determenes where to print and the way (%02x) ... just to remember from man
    }
    fprintf(output, "\n");
}


// task 1

void SetSigFileName()
{
    char input[256];
    printf("Enter new signature file name : \n");
    fgets(input, 256, stdin);
    sscanf(input, "%s", sigFileName);
}

virus *readVirus(FILE *file)
{
    // Allocate memory for the virus structure and its signature
    virus *v = (virus *)malloc(sizeof(virus));

    if (fread(v, 1, 16 * sizeof(char) + sizeof(short), file) != 18)
    {
        perror("Failed to read virus data");
        free(v->sig);
        free(v);
        return NULL;
    }

    v->sig = (unsigned char *)malloc(v->SigSize);

    if (isBigEndian == 1)
    { // if there is no error and it is big endian we switch the bytes

        v->SigSize = (v->SigSize >> 8) | (v->SigSize << 8); // switching the bytes
    }

    if (fread(v->sig, 1, v->SigSize, file) != v->SigSize)
    { // reading the signature and checking also
        perror("Failed to read the signature of the virus \n");
        free(v->sig); // freeing the allocated memory
        free(v);
        return NULL;
    }

    return v;
}

void printVirus(virus *v, FILE *output)
{
    fprintf(output, "Name: %s\n", v->virusName);
    fprintf(output, "Signature Length: %u\n", v->SigSize);
    fprintf(output, "Signature : ");
    PrintHex(v->sig, v->SigSize, output);
}

// task 1b 

void list_print(link *virus_list, FILE *output)
{
    if(virus_list == NULL){
        return;
    }

    list_print(virus_list->nextVirus , output);

    printVirus(virus_list->vir , output);

    fprintf(output , "\n");
}

link *list_append(link *virus_list, virus *data)
{

    link *new_link = (link *)malloc(sizeof(link));
    new_link->vir = data;

    if (virus_list != NULL)
    {
        new_link->nextVirus = virus_list;
    }

    return new_link;
}

void list_free(link *virus_list)
{

    link *curr = virus_list;

    while (curr->nextVirus != NULL)
    {

        link *next = curr->nextVirus; // a ptr to the next link
        free(curr->vir->sig);         // freeing the memory inside the virus
        free(curr->vir);              // freeing the memory of the virus inside the link
        free(curr);                   // freeing the link itself
        curr = next;                  // current link now is the "next"
    }
}

void LoadSignatures()
{
    file_b = fopen(sigFileName, "rb");
    if (!file_b)
    {
        perror("Failed to open file");
        return;
    }

    readviruses(file_b);
}

void listprint()
{
    list_print(virusList, stdout);
}

// Function to detect viruses
void DetectViruses() {
    size_t bytes_read;
    if (temp_argc == 1)
        Quit();

    FILE *in_file = fopen(temp_argv[1], "r");
    if (in_file == NULL)
    {
        fprintf(stderr, "Error: Couldn't open file\n");
        exit(1);
    }
    fseek(in_file, 0L, SEEK_END);
    bytes_read = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);
    fread(buffer, bytes_read, 1, in_file);

    detect_virus(buffer, bytes_read > BUFFERSIZE ? BUFFERSIZE : bytes_read, virusList);
}





// Function to quit the program
void Quit()
{
    if (virusList)
    {
        list_free(virusList);
    }
    printf("Exiting program ... \n");
    exit(0);
}

// help function for reading all the viruses in the file using readvirus in a loop
void readviruses(FILE *file)
{
    char magicNum[4];
    if (fread(magicNum, 1, 4, file) != 4)
    {
        perror("Failed to read magic number");
        fclose(file);
        exit(1);
    }

    if (strncmp(magicNum, "VIRL", 4) != 0 && strncmp(magicNum, "VIRB", 4) != 0)
    { // if not LE nor BE
        fprintf(stderr, "Invalid magic number \n");
        fclose(file);
        exit(1);
    }

    if (strncmp(magicNum, "VIRB", 4) == 0)
    { // checking if the magic number is BE
        isBigEndian = 1;
    }

    while (file != NULL && fgetc(file) != EOF)
    {
        fseek(file, -1, SEEK_CUR);
        virus *v = readVirus(file);
        virusList = list_append(virusList, v);
    }

    fclose(file);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    
    for (int byte = 0; byte < size; byte++)
    {
        link *curr = virus_list;
        while (curr)
        {
            size_t sigSize = curr->vir->SigSize;
            // check bounds
            if (byte + sigSize < size)
            {
                if (memcmp(buffer + byte, curr->vir->sig, sigSize) == 0)
                {
                    fprintf(stdout, "The starting byte location in the suspected file is: %d\n", byte);
                    fprintf(stdout, "The virus name is: %s\n", curr->vir->virusName);
                    fprintf(stdout, "The size of the virus signature is: %d\n", sigSize);
                }
            }
            curr = curr->nextVirus;
        }
    }
}

// task2 

void neutralize_virus(char *fileName, int signatureOffset)
{
    char RET[] = {0xC3};

    FILE *file = fopen(fileName, "r+");
    if (file == NULL)
    {
        fprintf(stderr, "Error: file can't be opened \n");
        exit(1);
    }
    fseek(file, signatureOffset, SEEK_SET);
    fwrite(RET, sizeof(char), 1, file);
    fclose(file);
}


void FixFile()
{
    char buffer[BUFFERSIZE];
    size_t bytes_read;
    size_t size;
    if (temp_argc == 1)
        Quit();

    FILE *in_file = fopen(temp_argv[1], "r");
    if (in_file == NULL)
    {
        fprintf(stdout, "Error: Couldn't open file\n");
        exit(1);
    }
    fseek(in_file, 0L, SEEK_END);
    bytes_read = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);
    fread(buffer, bytes_read, 1, in_file);
    size = bytes_read > BUFFERSIZE ? BUFFERSIZE : bytes_read;
    for (int byte = 0; byte < size; byte++)
    {
        link *curr = virusList;
        while (curr)
        {
            size_t sigSize = curr->vir->SigSize;
            // check bounds
            if (byte + sigSize < size)
            {
                if (memcmp(buffer + byte, curr->vir->sig, sigSize) == 0)
                {
                    neutralize_virus(temp_argv[1], byte);
                }
            }
            curr = curr->nextVirus;
        }
    }
    fclose(in_file);
}