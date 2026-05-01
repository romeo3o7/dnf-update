#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int task(char*path,char *argument[]);
void checkUpdate();
void update();

int main() {
    // first check if update exits
    checkUpdate();
    // if update is not found exit otherwise contiune
    update();
    // delete cache
    char *clean[] = {"dnf", "clean", "packages" , NULL};
    task("dnf",clean);

    printf("done!\n");
    // check if a service needs restarting
    // check if a reboot is recommended
    // exit

    return 0;
}

int task(char*path,char *argument[]) {
 pid_t childProcess = fork();
	if (childProcess == 0) {
	    execv(path, argument); // here child is gone if succesful
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

    void checkUpdate() {
        printf("Checking update...\n");
        char *updateChecking[] = {"dnf" , "check-update" , "--refresh" , NULL};
        int checkStatus = task("sudo",updateChecking);
        if (checkStatus != 100 && checkStatus != 0) { fprintf(stderr ," updating failed"); exit(EXIT_FAILURE); }
        else if (checkStatus == 0) { printf("Up to date\n"); exit(EXIT_SUCCESS); }
    }

void update() {
    printf("\nupdate found!\nwould you like an offline or online update?\noffline if the update includes : kernel,libc,systemd,mesa.\n'o' for online and 'f' for offline\n");
    char *offline[] = {"sudo","dnf" , "upgrade" , "--offline" , NULL};
    char *online[] = {"sudo", "dnf" , "upgrade" , NULL};
    char **update;
    char buffer[2];
    while(1) {
    char *input= fgets(buffer, sizeof(buffer) , stdin);
    if (input != NULL && (buffer[0] == 'o' || buffer[0] == 'f')) { // if pointer input is not null and first value of buffer is either 'o' or 'f'
       if (buffer[0] == 'f') { update = offline; }
       else { update = online; }
      break;
        }
    else { printf("wrong input, try again\n"); continue;}
    }
    int updateStatus = task("dnf",update);
    if (updateStatus != 0) { fprintf(stderr,"upgrade failed"); exit(EXIT_FAILURE); }

    printf("Update is done ... \n");
}
