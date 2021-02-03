#include "kernel/types.h"
#include "user/user.h"

int 
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(2, "# of argument should be one\n");
        exit(1);
    }
    char *time = argv[1];
    int timeInt = atoi(time);
    sleep(timeInt);
    exit(0);

}