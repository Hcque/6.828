#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLINE 1024

int 
main(int argc, char *argv[])
{
    
    int newArgc = 0;
    char *buf[MAXLINE], *p;
    char one[32];
    // p = one;
    char *params[MAXARG];
    int argInd = 0;
    char* cmd = argv[1];
    for (int i = 1; i < argc; ++i) params[argInd++] = argv[i];
    // fprintf(2, "%s\n", argv[0]);
    int k = 0;
    while ( 1 ) {
        int n = read(0, buf, MAXLINE);
        if (n == 0)
            break;
        // fprintf(2, "%d", n);
        // fprintf(2, "%s\n", p);
        for (int i = 0; i < n; ++i) {

        }
        if (buf[i] == " " || buf[i] == "\n") {
            buf[newArgc++] = one;
            fprintf(2, "%s\n", one);
            char one[32];
            // p = one;
        }
        else {
            one[k] = *p;

        }
        k++;
    }

    for (int i = 0; i < newArgc; ++i) {
        fprintf(2, "%s  ", buf[i]);
    }


    char *newAgrv[MAXARG], **pp;
    pp = newAgrv;
    int originSize = sizeof(char)*argc;
    memcpy(newAgrv, argv, originSize);
    memcpy(pp + originSize , buf, sizeof(char*)*newArgc);
    *pp++ = 0;


    for (int i = 0; i < newArgc; ++i) {
        fprintf(2, "%s  ", newAgrv[i]);
    }

    if (fork() == 0) {
        exec(cmd, newAgrv);
        exit(0);
    }
    else {
        exit(0);

    }


}