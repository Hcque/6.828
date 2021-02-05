#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLINE 1024

int 
main(int argc, char *argv[])
{

    int inputArgc = 0;
    char *newArgv[MAXLINE];
            fprintf(2, "%s\n", "df");

    char *elem = (char*) malloc(MAXLINE);
    int elemInd = 0;
    char buf = '0';
            fprintf(2, "%s\n", buf);

    while (read(0, &buf, 1))
    {
        fprintf(2, "%s\n", buf);
        if (buf != ' ' || buf != '\0') {
            elem[elemInd++] = buf;
        }
        else {
            elem[elemInd++] = '\0'; // terminates the string
            char elem[MAXLINE];
            elemInd = 0;

            fprintf(2, "%s\n", elem);
            newArgv[inputArgc++] = elem;
        }
    }
    

    if (argc < 2) {
        fprintf(2, "too few args\n");
    }
    fprintf(2, "argv[1]:%s\n", argv[1]);

    char *new[MAXLINE];
    if (memcpy(new, &argv[1], sizeof(char*)*(argc-1)) <= 0) {
        fprintf(2, "move failed for argv\n");
    }
    char **mid = new + sizeof(char*)*(argc-1);
    if (memcpy(mid, newArgv, sizeof(char*)*(inputArgc)) <= 0) {
        fprintf(2, "move failed for argv\n");
    }
    for (int i = 0; i < inputArgc + argc-1; i++) {
        fprintf(2, "%s\n", new[i]);
    }

    return 0;

}