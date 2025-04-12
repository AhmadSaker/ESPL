#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#define Menu_Length 9

// struct from task 0b
typedef struct
{
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;

    char display_mode; // task 1b
} state;

void print_menu(state *s);
static char *hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char *dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

// function from 0b
void set_file_name(state *s);
void quit(state *s);
void toggle_debug_mode(state *s);
void set_unit_size(state *s);

// task 1a Load Into Memory
void load_into_memory(state *s);
// task 1b Toggle Display Mode
void toggle_display_mode(state *s);
// task 1c Memory Display
void memory_display(state *s);
// task 1d Save Into File
void save_into_file(state *s);
// task 1e Modify Memory
void memory_modify(state *s);

// to remove
void not_implemented(state *s);

typedef void (*menu_function)(state *);
// struct from LAB 1
typedef struct fun_desc
{
    char *name;
    menu_function func;
} fundesc;

fundesc menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"Load Into Memory", load_into_memory},
    {"Toggle Display Mode", toggle_display_mode},
    {"Memory Display", memory_display},
    {"Save Into File", save_into_file},
    {"Memory Modify", memory_modify},
    {"Quit", quit},
    {NULL, NULL}};

int main(int argc, char const *argv[])
{
    state s = {0, "", 1, {0}, 0, 0};

    while (1)
    {
        print_menu(&s);
        printf("Option: ");
        int choice;
        if (scanf("%d", &choice) == 1 && choice >= 0 && choice <= 8)
        {
            while (getchar() != '\n')
                ; // Clear the input buffer
            menu[choice].func(&s);
        }
        else
        {
            printf("Invalid option\n");
            while (getchar() != '\n')
                ; // Clear the input buffer
        }
    }
    return 0;
}

// Functions from 0b

void set_file_name(state *s)
{
    printf("Enter file name: ");
    fgets(s->file_name, sizeof(s->file_name), stdin);

    // Remove the newline character if it exists
    size_t len = strlen(s->file_name);
    if (len > 0 && s->file_name[len - 1] == '\n')
    {
        s->file_name[len - 1] = '\0';
    }

    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void quit(state *s)
{
    if (s->debug_mode)
    {
        printf("quitting\n");
    }
    exit(0);
}

void print_menu(state *s)
{
    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: unit_size = %d file_name = %s mem_count = %zu\n", s->unit_size, s->file_name, s->mem_count);
    }
    printf("Choose action:\n");
    for (int i = 0; menu[i].name != NULL; i++)
    {
        fprintf(stdout, "%d. %s\n", i, menu[i].name);
    }
}

void set_unit_size(state *s)
{
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    if (scanf("%d", &size) == 1 && (size == 1 || size == 2 || size == 4))
    {
        s->unit_size = size;
        if (s->debug_mode)
        {
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
        }
    }
    else
    {
        printf("Invalid unit size\n");
    }
    while (getchar() != '\n')
        ; // Clear the input buffer
}
void toggle_debug_mode(state *s)
{
    if (s->debug_mode)
    {
        s->debug_mode = 0;
        printf("Debug flag now off\n");
    }
    else
    {
        s->debug_mode = 1;
        printf("Debug flag now on\n");
    }
}

// task 1a
void load_into_memory(state *s)
{
    // checking the file name
    if (strcmp(s->file_name, "") == 0)
    {
        printf("Error: file name is empty\n");
        return;
    }

    FILE *file = fopen(s->file_name, "rb");
    // checking the file
    if (!file)
    {
        perror("Error opening file");
        return;
    }

    // vars for the input
    char input[256];
    unsigned int location;
    int length;

    printf("Please enter <location> <length>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &location, &length);

    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: file name is: %s\n", s->file_name);
        fprintf(stderr, "Debug: location is: %x\n", location);
        fprintf(stderr, "Debug: length is: %d\n", length);
    }

    fseek(file, location, SEEK_SET);
    size_t bytes_read = fread(s->mem_buf, s->unit_size, length, file);
    if (bytes_read != length)
    {
        printf("Error: Could only read %zu units\n", bytes_read);
    }
    else
    {
        printf("Loaded %zu units into memory\n", bytes_read);
        s->mem_count = bytes_read;
    }

    fclose(file);
}

// task 1b
void toggle_display_mode(state *s)
{
    if (s->display_mode)
    {
        s->display_mode = 0;
        printf("Display flag now off, decimal representation\n");
    }
    else
    {
        s->display_mode = 1;
        printf("Display flag now on, hexadecimal representation\n");
    }
}

void memory_display(state *s)
{
    char input[256];
    unsigned int addr;
    int length;
    void *start_addr;

    printf("Enter address and length: ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &addr, &length);

    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: addr = 0x%X, length = %d\n", addr, length);
    }

    if (addr == 0)
    {
        start_addr = &(s->mem_buf);
    }
    else
    {
        start_addr = &addr;
    }

    int Limit = start_addr + s->unit_size * length;
    
    while (start_addr < Limit)
    {
        if (s->display_mode)
        { // hexadecimal representation
            printf(hex_formats[s->unit_size - 1], *((int *)start_addr));
        }
        else
        { // decimal representation
            printf(dec_formats[s->unit_size - 1], *((int *)start_addr));
        }
        start_addr = start_addr + s->unit_size;
    }
    /*
    256
    0
    12032
    26988
    12130
    */
    printf("\n");
}

// task 1d
void save_into_file(state *s)
{
    char input[256];
    unsigned int source_addr, target_offset;
    int length;
    unsigned char *source_addr_ptr;

    printf("Please enter <source-address> <target-location> <length>\n");

    // using fgets and sscanf ...
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x %d", &source_addr, &target_offset, &length);

    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: source-address = 0x%X, target-location = 0x%X, length = %d\n", source_addr, target_offset, length);
    }

    if (source_addr == 0)
    {
        source_addr_ptr = s->mem_buf;
    }
    else
    {
        source_addr_ptr = (unsigned char *)source_addr;
    }

    int fd = open(s->file_name, O_RDWR); // opening the file for reading and writing without truncating
    if (fd < 0)
    {
        perror("Error opening file");
        return;
    }

    off_t file_size = lseek(fd, 0, SEEK_END); // using the lseek to determene the file size

    if (file_size < 0)
    {
        perror("Error determining file size");
        close(fd);
        return;
    }

    if (target_offset > file_size)
    {
        fprintf(stderr, "Error: target location is beyond file size\n");
        close(fd);
        return;
    }

    if (lseek(fd, target_offset, SEEK_SET) < 0)
    {
        perror("Error seeking in file");
        close(fd);
        return;
    }

    int bytes_to_write = length * s->unit_size; // the bytes to write depends on the unit size
    ssize_t written = write(fd, source_addr_ptr, bytes_to_write);

    // fprintf(stderr, "DEBUGGING : written is %zd\n", written);

    if (written < 0) // the function stops here when testing
    {
        perror("Error writing to file");
    }
    else if (written != bytes_to_write)
    {
        fprintf(stderr, "Warning: not all bytes were written to file\n");
    }
    close(fd);
}

void memory_modify(state *s)
{
    char input[256];
    int location, val;

    printf("Please enter <location> <val>: ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x", &location, &val);

    if (s->debug_mode)
    {
        fprintf(stderr, "Debug: location = 0x%X, val = 0x%X\n", location, val);
    }

    // location can't be after the maximum ...
    int max = sizeof(s->mem_buf);

    if (location >= max)
    {
        printf("location is off bounds");
        return;
    }
    // after checking , we do what we are asked to in this function
    memcpy(&s->mem_buf[location], &val, s->unit_size);
}


