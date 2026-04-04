        #include <stdint.h>
        #include <stdio.h>
        #include <stdlib.h>
        #include <unistd.h>
        #include <sys/wait.h>
        #define PATH "/bin/dnf"
        int task(char * argument[]);

        int main() {
            if ( geteuid() != 0 ) { fprintf(stderr, "you Must run it with sudo\n"); exit(EXIT_FAILURE); }
            // first check if update exits
            char *updateChecking[] = {"dnf" , "check-update" , "--refresh" , NULL};
            int checkStatus = task(updateChecking);
            if (checkStatus != 100 && checkStatus != 0) exit(EXIT_FAILURE);
            else if (checkStatus == 0) {printf("Up to date\n"); exit(EXIT_SUCCESS); }
            // second update
            char *update[] = {"dnf" , "upgrade" , NULL};
            int updateStatus = task(update);
            if (updateStatus != 0) { fprintf(stderr,"upgrade failed"); exit(EXIT_FAILURE); }
            // delete cache
            char *clean[] = {"dnf", "clean", "packages" , NULL};
            int cleanStatus = task(clean);
            // check if a service needs restarting
            // check if a reboot is recommended
            // exit

            return 0;
        }
        int task(char *argument[]) {
        pid_t childProcess = fork();
        if (childProcess == 0) {
            execv(PATH, argument); // here child is gone if succesful
            perror("child failed, execv stage\n");
            exit(EXIT_FAILURE);
        } else if (childProcess == -1) {
            fprintf(stderr, "child failed , fork stage\n");
            return 1;
            }
            int childProcessStatus;
            waitpid(childProcess, &childProcessStatus , 0);
            if (WIFEXITED(childProcessStatus)) {
                return WEXITSTATUS(childProcessStatus);
            }
            if (WIFSIGNALED(childProcessStatus)) {
                fprintf(stderr, "child killed by signal %d\n", WTERMSIG(childProcessStatus));
                return -1;
            }
            return -1;
        }
