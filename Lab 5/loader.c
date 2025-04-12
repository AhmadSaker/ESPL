#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>

// helper function 
long getFileSize(const char *filename);
// the function we must implement in task 0 , iterator over program headers in the file
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg);
void print_phdr(Elf32_Phdr *phdr, int index);

// Task 1a , printing what readelf -l command prints + task 1b , printing the protection flags
void readelf_l(Elf32_Phdr *phdr, int index);
// Task 2b
void load_phdr(Elf32_Phdr *phdr, int fd);
// Task 2c
extern int startup(int argc, char **argv, void (*start)());


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Not enough args : %d \n", argc);
        exit(EXIT_FAILURE);
    }

    // in case we got the right number of arguments
    char *filename = argv[1];

    // finding the file descriptor
    int fd = open(filename, O_RDONLY);

    // usual check if we have opened the file successfully
    if (fd == -1)
    {
        perror("Failed to open file ");
        exit(EXIT_FAILURE);
    }

    long fsize = getFileSize(filename);

    // finding the start of the file in the memory by using mmap
    void *map_start = mmap(NULL, fsize , PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED)
    {
        perror("Failed to map file to memory");
        close(fd);
        return EXIT_FAILURE;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3)
    {
        fprintf(stderr, "Not a valid ELF file\n");
        munmap(map_start, fsize);
        close(fd);
        return EXIT_FAILURE;
    }

    foreach_phdr(map_start, print_phdr, fd);
    printf("\n\n\n");

    printf("Type  Offset  VirtAddr  PhysAddr  FileSiz  MemSiz  Flg  Align \n"); // just to look like the readelf -l
    foreach_phdr(map_start, readelf_l, fd);
    printf("\n\n\n");

    foreach_phdr(map_start , load_phdr , fd);

    // Task 2c 
    startup(argc-1, argv+1, (void *)(((Elf32_Ehdr *) map_start)->e_entry));

    munmap(map_start, fsize); // This releases the memory allocated for the ELF file.
    close(fd);
    return 0;
}

// Task 0
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *header = (Elf32_Phdr *)(map_start + ehdr->e_phoff); // e_phoff is the offset of the header

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        func(&header[i], arg);
    }
    return 0;
}

// debug function from task 0
void print_phdr(Elf32_Phdr *phdr, int index)
{
    printf("Program header number %d at address %p\n", index, (void *)phdr);
}

// Task 1a + 1b (extending 1a)
void readelf_l(Elf32_Phdr *phdr, int fd)
{
    const char *type;

    // checking the type ...
    if (phdr->p_type == PT_LOAD)
    {
        type = "LOAD";
    }
    else if (phdr->p_type == PT_DYNAMIC)
    {
        type = "DYNAMIC";
    }
    else if (phdr->p_type == PT_INTERP)
    {
        type = "INTERP";
    }
    else if (phdr->p_type == PT_NOTE)
    {
        type = "NOTE";
    }
    else if (phdr->p_type == PT_SHLIB)
    {
        type = "SHLIB";
    }
    else if (phdr->p_type == PT_PHDR)
    {
        type = "PHDR";
    }
    else
    {
        type = "UNKNOWN";
    }

    printf("%-5s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x ",
           type,
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz);

    printf("%c%c%c ",
           (phdr->p_flags & PF_R) ? 'R' : ' ',
           (phdr->p_flags & PF_W) ? 'W' : ' ',
           (phdr->p_flags & PF_X) ? 'E' : ' ');

    printf("0x%x\n", phdr->p_align);

    // task 1b
    const char *flag;

    // checking the 3 flags that given in reading materials
    if ((phdr->p_flags & PF_R) && (phdr->p_flags & PF_W) && (phdr->p_flags & PF_X))
        flag = "READ-WRITE-EXECUTE";
    else if ((phdr->p_flags & PF_R) && (phdr->p_flags & PF_W))
        flag = "READ-WRITE";
    else if ((phdr->p_flags & PF_R) && (phdr->p_flags & PF_X))
        flag = "READ-EXECUTE";
    else if (phdr->p_flags & PF_R)
        flag = "READ";
    else if (phdr->p_flags & PF_W)
        flag = "WRITE";
    else if (phdr->p_flags & PF_X)
        flag = "EXECUTE";
    else
        flag = "NONE";

    printf("The protection flags are : %s\n", flag);
}

// Task 2b
void load_phdr(Elf32_Phdr *phdr, int fd)
{
    // first we check if it is LOAD type
    if (phdr->p_type != PT_LOAD)
    {
        return;
    }

    int flag;

    // lets find the flags ...
    if ((phdr->p_flags & PF_R) && (phdr->p_flags & PF_W) && (phdr->p_flags & PF_X))
        flag = PROT_READ | PROT_WRITE | PROT_EXEC;
    else if ((phdr->p_flags & PF_R) && (phdr->p_flags & PF_W))
        flag = PROT_READ | PROT_WRITE;
    else if ((phdr->p_flags & PF_R) && (phdr->p_flags & PF_X))
        flag = PROT_EXEC | PROT_READ;
    else if (phdr->p_flags & PF_R)
        flag = PROT_READ;
    else if (phdr->p_flags & PF_W)
        flag = PROT_WRITE;
    else if (phdr->p_flags & PF_X)
        flag = PROT_EXEC;

    // laoding the file using mmap
        void *vaddr = (void*) (phdr->p_vaddr & 0xfffff000);
        int offset = phdr->p_offset & 0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;
        
        void *map_start = mmap(vaddr, phdr->p_memsz + padding, flag, MAP_FIXED | MAP_PRIVATE, fd, offset);

    // in case of failure 
    if (map_start == MAP_FAILED)
    {
        perror("mmap failed"); // error message 
        close(fd); // closing the file
        exit(EXIT_FAILURE); // exiting program
    }
    // readelf_l(phdr , 0);
}


// helper function 
long getFileSize(const char *filename) {
    FILE *file = fopen(filename, "rb"); // Open file in binary mode
    if (!file) {
        perror("Error opening file");
        return -1; // Return an error value
    }

    fseek(file, 0, SEEK_END); // Move file pointer to the end of the file
    long size = ftell(file);  // Get current file pointer position (file size)
    fclose(file);             // Close the file

    return size;
}