#include "ucode.c"
main(int argc, char *argv[])
{
    int sh, pid, status;
    sh = fork();
    if (sh) // P1 runs in a forever loop
    {
        while (1)
        {
            pid = wait(&status);    // wait for any child to die
            if (pid == sh)  // if sh died, fork another!
            {
                sh = fork();
                continue;
            }
            printf("P1 just buried an orphan pid=%d\n", pid);
        }
    }
    else
    {
        exec("sh start");
    }
}