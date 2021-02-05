#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLINE 1024

int 
main(int argc, char *argv[])
{
    int inputArgc = 0;
    char *newArgv[MAXLINE];
    char elem[MAXLINE], *p;
    p = elem;
    char buf;
    while (read(0, &buf, 1))
    {
        if (buf != ' ' || buf != '\0') {
            *p = buf;
            p++; 
        }
        else {
            *p = '\0'; // terminates the string
            char elem[MAXLINE], *p;
            p = elem;

            fprintf(2, "%s\n", elem);
            newArgv[inputArgc++] = elem;
        }
    }
    *p = ' ';
    

    if (argc < 2) {
        fprintf(2, "too few args\n");
    }
    fprintf(2, "argv[1]:%s\n", argv[1]);

    char *new[MAXLINE];
    if (memcpy(new, &argv[1], sizeof(char*)*(argc-1)) <= 0) {
        fprintf(2, "move failed for argv\n");
    }
    if (memcpy(new, newArgv, sizeof(char*)*(inputArgc)) <= 0) {
        fprintf(2, "move failed for argv\n");
    }

    return 0;

}