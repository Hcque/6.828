#include "kernel/types.h"
#include "user/user.h"

const int UPBOUND  = 35;
int p[2];

int testPrime(int n)
{
    for (int i = 2; i < n; ++i) 
        if (n % i == 0) return 0;
    return 1;
}

void pipePrimeHelper(int i)
{
    for (int k = i; k < UPBOUND+1; ++k) {
        if (testPrime(k)) {
            pipe(p);
            fprintf(2, "prime %d\n", k);
            // child process
            if (fork() == 0) {
                int ans;
                close(p[1]);
                read(p[0], &ans, sizeof(int));
                close(p[0]);
                // fprintf(2, "%d", ans);
                
                // fprintf(2, "prime %d\n", atoi(buf));
                pipePrimeHelper(ans+1);
                exit(0);
            }
            // parent process
            else {
                close(p[0]);
                write(p[1], &k, sizeof(k));
                wait(0);
                exit(0);
            }
        }
    }
}

int 
main(int argc, char *argv[])
{
    pipePrimeHelper(2);
    return 0;
}