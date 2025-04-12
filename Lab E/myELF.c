#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_ELF_FILES 2
#define MENU_SIZE 7

// Helper functions
long getFileSize(const char *filename);
void print_sections(int index);
void printSymbolTableInfo(Elf32_Ehdr *ehdr);

// Function pointer typedef
typedef void (*menu_func)();

// Struct for menu items
typedef struct
{
    const char *name;
    menu_func func;
} fundesc;

// Global variables
int debug_mode = 0;
int fd[MAX_ELF_FILES] = {-1, -1};
void *map_start[MAX_ELF_FILES] = {NULL, NULL};

// Function prototypes
void toggle_debug_mode();
void examine_elf_file();
void print_section_names();
void print_symbols();
void CheckMerge();
void merge_elf_files();
void quit();

// Menu items
fundesc menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Examine ELF File", examine_elf_file},
    {"Print Section Names", print_section_names},
    {"Print Symbols", print_symbols},
    {"Check Files for Merge", CheckMerge},
    {"Merge ELF Files", merge_elf_files},
    {"Quit", quit},
    {NULL, NULL}};

// Main function
int main()
{
    int choice;
    while (1)
    {
        printf("Choose action:\n");
        for (int i = 0; menu[i].name != NULL; i++)
        {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option: ");
        scanf("%d", &choice);

        if (choice >= 0 && choice < MENU_SIZE)
        {
            menu[choice].func();
        }
        else
        {
            printf("Invalid choice\n");
        }
    }
    return 0;
}

// Toggle debug mode function
void toggle_debug_mode()
{
    debug_mode = (debug_mode + 1) % 2;
    printf("Debug mode %s\n", debug_mode ? "ON" : "OFF");
}

// Examine ELF file function
void examine_elf_file()
{
    char filename[100];
    int index;

    // Determine index of file descriptor array 
    if (fd[0] == -1)
    {
        index = 0;
    }
    else if (fd[1] == -1)
    {
        index = 1;
    }
    else
    {
        printf("Can't handle another file !\n");
        return;
    }

    printf("Enter file name: ");
    scanf("%s", filename);

    // Debug mode printing
    if (debug_mode)
    {
        printf("File name: %s\n", filename);
    }

    // Open the file
    fd[index] = open(filename, O_RDWR);
    if (fd[index] == -1)
    {
        perror("Failed to open the file");
        return;
    }

    
    long fsize = getFileSize(filename);
    if (fsize == -1)
    {
        close(fd[index]);
        fd[index] = -1;
        return;
    }

    struct stat st;
    if (fstat(fd[index], &st) < 0)
    {
        perror("fstat");
        close(fd[index]);
        fd[index] = -1;
        return;
    }

    // Memory map the file
    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd[index], 0);
    if (map == MAP_FAILED)
    {
        perror("mmap");
        close(fd[index]);
        fd[index] = -1;
        return;
    }

    // Check if the file is an ELF file
    unsigned char *e_ident = (unsigned char *)map;
    if (e_ident[EI_MAG0] != ELFMAG0 || e_ident[EI_MAG1] != ELFMAG1 ||
        e_ident[EI_MAG2] != ELFMAG2 || e_ident[EI_MAG3] != ELFMAG3)
    {
        fprintf(stderr, "Error: Not an ELF file.\n");
        munmap(map, st.st_size);
        close(fd[index]);
        fd[index] = -1;
        return;
    }

    // Store mapped file pointer
    map_start[index] = map;

    // Print ELF header information
    Elf32_Ehdr *header = (Elf32_Ehdr *)map;
    printf("Magic:   %.3s\n", header->e_ident + 1);
    printf("Data:    %d\n", header->e_ident[EI_DATA]);
    printf("Entry point: 0x%x\n", header->e_entry);
    printf("Section header offset: %d\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of section header: %d\n", header->e_shentsize);
    printf("Program header offset: %d\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of program header: %d\n", header->e_phentsize);

    // Debug mode printing
    if (debug_mode)
    {
        printf("Debug: File %s successfully examined and mapped\n", filename);
    }
}

// Task 1
void print_section_names()
{
    if (map_start[0] != NULL)
    {
        print_sections(0);
    }
    if (map_start[1] != NULL)
    {
        print_sections(1);
    }
    if (map_start[0] == NULL && map_start[1] == NULL)
    {
        printf("Enter a file first!\n");
    }
}

// Task 1 helper function
void print_sections(int index)
{
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start[index];
    Elf32_Shdr *sections = (Elf32_Shdr *)((void *)header + header->e_shoff);
    Elf32_Shdr *entry = &sections[header->e_shstrndx];
    char *sectionsname = (char *)((void *)header + entry->sh_offset);

    printf("\nSection Headers:\n");

    for (int i = 0; i < header->e_shnum; i++)
    {
        Elf32_Shdr *section_entry = &sections[i];
        char *section_name = &sectionsname[section_entry->sh_name];
        printf("[%d] %s 0x%x 0x%x %d %u\n",
               i,
               section_name,
               section_entry->sh_addr,
               section_entry->sh_offset,
               section_entry->sh_size,
               section_entry->sh_type);
    }
}

// Task 2 
void print_symbols()
{
    if (map_start[0] == NULL && map_start[1] == NULL)
    {
        printf("Inter a file name first (use the first option)\n");
    }
    for (int i = 0; i < MAX_ELF_FILES; i++)
    {
        if (map_start[i] != NULL) // applying the function to all the available files 
        {
            printf("Num: Value Size Type Bind Vis Ndx Name\n");
            Elf32_Ehdr *header = (Elf32_Ehdr*)map_start[i];
            printSymbolTableInfo(header);
            printf("\n\n");
        }
    }
}

// helper function for Task 2
void printSymbolTableInfo(Elf32_Ehdr *header)
{
    Elf32_Shdr *sections = (Elf32_Shdr *)((void *)header + header->e_shoff);

    // Find the symbol table section and its associated string table
    Elf32_Shdr *symbolSection = NULL;
    Elf32_Shdr *stringSection = NULL;
    for (int idx = 0; idx < header->e_shnum; idx++)
    {
        Elf32_Shdr *section = &sections[idx];
        if (section->sh_type == SHT_SYMTAB) // Locate symbol table
        {
            symbolSection = section;
            stringSection = &sections[section->sh_link];
            break;
        }
    }

    if (symbolSection == NULL)
    {
        printf("No symbol table found.\n");
        return;
    }

    Elf32_Sym *symbols = (Elf32_Sym *)((void *)header + symbolSection->sh_offset);
    char *strTable = (char *)((void *)header + stringSection->sh_offset);

    
    int numSymbols = symbolSection->sh_size / symbolSection->sh_entsize;
    for (int i = 0; i < numSymbols; i++)
    {
        Elf32_Sym *sym = &symbols[i];

        char *symName = &strTable[sym->st_name];

        char *secName;

        if (sym->st_shndx == 0xFFF1)
            secName = "ABS";
        else if (sym->st_shndx == 0x0)
            secName = "UND";
        else
        {
            Elf32_Shdr *defSec = &sections[sym->st_shndx];
            secName = &strTable[defSec->sh_name];
        }
        printf("[%d] 0x%x \t %d \t %s \t %s\n", i, sym->st_value, sym->st_shndx, secName, symName);
    }
}

// Task 3.1 
void CheckMerge() {
    // Check if both ELF files are loaded
    if (map_start[0] == NULL || map_start[1] == NULL) {
        printf("Error: Two ELF files must be opened and mapped.\n");
        return;
    }

    // Cast ELF headers for both files
    Elf32_Ehdr *ehdr1 = (Elf32_Ehdr *)map_start[0];
    Elf32_Ehdr *ehdr2 = (Elf32_Ehdr *)map_start[1];

    // Section header offsets
    Elf32_Shdr *shdr1 = (Elf32_Shdr *)((void *)ehdr1 + ehdr1->e_shoff);
    Elf32_Shdr *shdr2 = (Elf32_Shdr *)((void *)ehdr2 + ehdr2->e_shoff);

    // Strings tables for section names
    char *strtab1 = (char *)((void *)ehdr1 + shdr1[ehdr1->e_shstrndx].sh_offset);
    char *strtab2 = (char *)((void *)ehdr2 + shdr2[ehdr2->e_shstrndx].sh_offset);

    
    printf("Section names:\n");
    for (int i = 0; i < ehdr1->e_shnum; i++) {
        printf("section %d : %s\n",i ,&strtab1[shdr1[i].sh_name]);
    }

    
    for (int i = 0; i < ehdr2->e_shnum; i++) {
        printf("section %d : %s\n", i, &strtab2[shdr2[i].sh_name]);
    }

    // Symbol tables
    Elf32_Shdr *symtab1 = NULL;
    Elf32_Shdr *symtab2 = NULL;

    // Find symbol tables in both ELF files
    for (int i = 0; i < ehdr1->e_shnum; i++) {
        if (shdr1[i].sh_type == SHT_SYMTAB) {
            symtab1 = &shdr1[i];
            break;
        }
    }
    for (int i = 0; i < ehdr2->e_shnum; i++) {
        if (shdr2[i].sh_type == SHT_SYMTAB) {
            symtab2 = &shdr2[i];
            break;
        }
    }

    // Check if both ELF files have symbol tables
    if (symtab1 == NULL || symtab2 == NULL) {
        printf("Feature not supported: Each file must contain exactly one symbol table.\n");
        return;
    }

    // Symbol tables and string tables for symbols
    Elf32_Sym *sym1 = (Elf32_Sym *)((void *)ehdr1 + symtab1->sh_offset);
    Elf32_Sym *sym2 = (Elf32_Sym *)((void *)ehdr2 + symtab2->sh_offset);
    char *symstr1 = (char *)((void *)ehdr1 + shdr1[symtab1->sh_link].sh_offset);
    char *symstr2 = (char *)((void *)ehdr2 + shdr2[symtab2->sh_link].sh_offset);

    
    printf("Symbols are :\n");
    for (int i = 1; i < symtab1->sh_size / sizeof(Elf32_Sym); i++) { // Start from 1 to skip the null symbol
        printf("symbol %d : %s\n", i,&symstr1[sym1[i].st_name]);
    }

    
    for (int i = 1; i < symtab2->sh_size / sizeof(Elf32_Sym); i++) { // Start from 1 to skip the null symbol
        printf("symbol %d : %s\n",i, &symstr2[sym2[i].st_name]);
    }
}


void merge_elf_files()
{
    printf("I didn't implement this (BONUS)\n");
}

// Quit function
void quit()
{
    for (int i = 0; i < MAX_ELF_FILES; i++)
    {
        if (fd[i] != -1)
        {
            munmap(map_start[i], lseek(fd[i], 0, SEEK_END));
            close(fd[i]);
        }
    }
    if (debug_mode)
    {
        printf("Quitting...\n");
    }
    exit(0);
}

// helper function ... 
long getFileSize(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Error opening file");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);

    return size;
}
