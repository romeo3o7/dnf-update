#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define PATH "/usr/bin/dnf"

int task(char * argument[]);

int main() {
    if ( geteuid() != 0 ) { fprintf(stderr, "you Must run it with sudo\n"); exit(EXIT_FAILURE); }
    // first check if update exits
    printf("Checking update...\n");
    char *updateChecking[] = {"dnf" , "check-update" , "--refresh" , NULL};
    int checkStatus = task(updateChecking);
    if (checkStatus != 100 && checkStatus != 0) { fprintf(stderr ," updating failed"); exit(EXIT_FAILURE); }
    else if (checkStatus == 0) { printf("Up to date\n"); exit(EXIT_SUCCESS); }
    // second update
    char *offline[] = {"dnf" , "upgrade" , "--offline" , NULL};
    char *online[] = {"dnf" , "upgrade" , NULL};
    char **update;
    printf("\nupdate found!\nwould you like an offline or online update?\noffline if the update includes : kernel,libc,systemd,mesa,shell.\n'o' for online and 'f' for offline\n");
    char buffer[2];
    while(1) {
    char *input= fgets(buffer, sizeof(buffer) , stdin);
    if (input != NULL && (buffer[0] == 'o' || buffer[0] == 'f')) {
       if (buffer[0] == 'f') { update = offline; }// offline
       else { update = online; }
      break;
    }
    else { printf("wrong input, try again\n"); continue;}
    }
    int updateStatus = task(update);
    if (updateStatus != 0) { fprintf(stderr,"upgrade failed"); exit(EXIT_FAILURE); }

    if (buffer[0] == 'f') {
        char *offlineUpdate[] = {"dnf" , "upgrade" , "--offline" , NULL};
        task(offlineUpdate);
    }

    printf("Update is done ... cache cleaning now ... \n");
    // delete cache
    char *clean[] = {"dnf", "clean", "all" , NULL};
    task(clean);

    printf("done!\n");
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
	// parent
	int childProcessStatus;
	waitpid(childProcess, &childProcessStatus , 0); // wait for the child to run and read its exit status
	if (WIFEXITED(childProcessStatus)) { // if the child exited normally
	    return WEXITSTATUS(childProcessStatus); //return its status to be read in main
	}
	if (WIFSIGNALED(childProcessStatus)) { // if child interputted
	    fprintf(stderr, "child killed by signal %d\n", WTERMSIG(childProcessStatus));
	    return -1;
	}
	return -1;
    }
