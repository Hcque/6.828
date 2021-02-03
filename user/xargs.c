#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLINE 1024

int 
main(int argc, char *argv[])
{
    
    char buf[MAXLINE];
    char* one = (char*) malloc(32);
    char *params[MAXARG];
    int argInd = 0;
    char* cmd = argv[1];
    for (int i = 1; i < argc; ++i) params[argInd++] = argv[i];


    int k = 0;
    int oneInd = 0;
    while ( 1 ) {
        int n = read(0, buf, MAXLINE);
        if (n == 0)
            break;
        // fprintf(2, "%d", n);
        // fprintf(2, "%s\n", p);
        for (int i = 0; i < n; ++i) {
            if (buf[i] == ' ' || buf[i] == '\n') {
            params[argInd++] = one;
            // fprintf(2, "%s\n", one);
            one = (char*) malloc(32);
            oneInd = 0;

        }
        else {
            one[oneInd++] = buf[i];
        }
        }
        
        k++;
    }

    // for (int i = 0; i < k + argc-1; ++i) {
    //     fprintf(2, "%s  ", params[i]);
    // }


    // char *newAgrv[MAXARG], **pp;
    // pp = newAgrv;
    // int originSize = sizeof(char)*argc;
    // memcpy(newAgrv, argv, originSize);
    // memcpy(pp + originSize , buf, sizeof(char*)*newArgc);
    // *pp++ = 0;


    // for (int i = 0; i < newArgc; ++i) {
    //     fprintf(2, "%s  ", newAgrv[i]);
    // }

    if (fork() == 0) {
        exec(cmd, params);
        exit(0);
    }
    else {
        wait((int*)0);
        exit(0);

    }


}