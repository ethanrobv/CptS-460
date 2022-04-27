#include "ucode.c"
main(int argc, char *argv[])
{
    int pid, status;

    while (1)
    {
        printf("SH: Enter a command:\n");
        char *cmdline = geti();

        token(cmdline);

        geti();

        // first arg in vector should be the file name
        char *cmd = argv[0];

        if (!strcmp(cmd, "exit"))
        {
            exit(0);
        }

        // fork child proc to exec the cmd
        pid = fork();
        if (pid)    // parent waits for child to die
        {
            pid = wait(&status);
        }
        else    // child exec cmd
        {
            int r = exec(cmdline);
            if (r == -1)    // exec fails
                exit(1);
        }
    }
}
