#include "kernel/types.h"
#include "user/user.h"

int 
main(int argc, char *argv[])
{
    int pParent[2], pChild[2];
    pipe(pParent);
    pipe(pChild);

    int pid;
    if (( pid = fork()) == 0) {
        close(pParent[1]);
        char buf[2];
        read(pParent[0], buf, 2);
        int cid = getpid();
        fprintf(2, "%d: received ping\n", cid);
        close(pChild[0]);
        write(pChild[1], buf, sizeof(buf));
        exit(0);

    } else {
        close(pParent[0]);
        write(pParent[1], "p\n", 2);

        char buf[2];
        close(pChild[1]);
        read(pChild[0], buf, 2);
        fprintf(2, "%d: received pong\n", getpid());
        exit(0);
                
    }

}