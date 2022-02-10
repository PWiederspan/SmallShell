#define _POSIX_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> // pid_t
#include <sys/stat.h>
#include <unistd.h> // fork
#include <dirent.h>
#include <sys/wait.h> // for waitpid
#include <fcntl.h>


// Initialize flags used in Signal Handlers
volatile sig_atomic_t sig_flag = 0;
volatile sig_atomic_t rm_flag = 0;
volatile sig_atomic_t interupt_flag = 0;
volatile sig_atomic_t bg_flag = 0;

// A linked list node for PID linked list
struct Node {
    int data;
    struct Node* next;
};


//Iterate through linked list of Nodes and collects any zombie children 
void cleanList(struct Node* head)
{
    head = head->next;
    while (head!= NULL)  {
        head = head->next;
        int status;
        pid_t pid;
        pid = waitpid(-1, &status, WNOHANG);
        //printf("Exited with status: %ld\n",  WEXITSTATUS(status));
    }
}

// Add a Pid Node to the linked list
struct Node* add_node(struct Node* n, int data){
    struct Node* new_pid = NULL;
    new_pid = (struct Node*)malloc(sizeof(struct Node));
    new_pid->data = data;
    new_pid->next = NULL;
    while (n->next != NULL){
        if(n->next == NULL)
            {
                n->next = new_pid;
                break;
                }
        n = n->next;
    }
    n->next = new_pid;
}

void remove_node(struct Node** n, int delete){
    struct Node *temp = *n, *prev;
 
    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->data == delete) {
        *n = temp->next; // Changed head
        free(temp); // free old head
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->data != delete) {
        prev = temp;
        temp = temp->next;
    }
 
    // If key was not present in linked list
    if (temp == NULL)
        return;
 
    // Unlink the node from linked list
    prev->next = temp->next;
 
    free(temp); // Free memory
    printf("Removed %ld\n", delete);
}

void off(int);

void inter_sig(int sig){
    int status;
    pid_t pid;
    pid = waitpid(-1, &status, WNOHANG);
    char message[] = "Process Terminated by signal";
    write(STDERR_FILENO, message, sizeof message - 1);
    interupt_flag = pid;
    signal(SIGINT, SIG_DFL);
}

void on(int sig){
    sig_flag = 1;
    char message[]  = "\nNow Entering Foreground Only Mode (& is now ignored)\n";
    write(STDERR_FILENO, message, sizeof message - 1);
    signal(SIGTSTP, &off);
}

void off(int sig){
  sig_flag = 0;
    char message[] = "\nNow Exiting Foreground Only Mode (& is no longer ignored)\n";
    write(STDERR_FILENO, message, sizeof message - 1);
    signal(SIGTSTP, &on);
}

// Called when the user inputs ls command
void list_contents(){
    // Open the current directory
    DIR *currDir = opendir(".");
    struct dirent *aDir;
    struct stat dirStat;

    // Go through all the entries
    while ((aDir = readdir(currDir)) != NULL)
    {
        // Get meta-data for the current entry
        stat(aDir->d_name, &dirStat);
        printf("%s      ", aDir->d_name);
    }
    // Close the directory
    closedir(currDir);
    printf("\n");
}

// Changes directory base on the given relative or absolute path
void change_directory(char *path){
    char  gdir[2048];
    char slash_start[2048] = "/";
    char path_start;
    path_start = path[0];

    //get the path to the current directory
    getcwd(gdir, 2048);

    // if the path give does not start with "/" add it to complete the path then change directory
    if (path_start != '/'){
        strcat(slash_start, path);
        strcat(gdir, slash_start);
        //printf("%s\n", gdir);

        chdir(gdir);
    } else {
        strcat(gdir, path);
        //printf("%s\n", gdir);

        chdir(gdir);
    }

}

// Save ls results to file specified by command
void list_to_file(char *name){
    char path[2048];
    char slash[2048] = "/";
    getcwd(path, 2048);

    strcat(slash, name);
    strcat(path, slash);

    FILE *new_file;
    new_file = fopen(path, "w");
    
    // Open the current directory
    DIR *currDir = opendir(".");
    struct dirent *aDir;
    struct stat dirStat;

    // Go through all the entries
    while ((aDir = readdir(currDir)) != NULL)
    {
        // Get meta-data for the current entry
        stat(aDir->d_name, &dirStat);
        fprintf(new_file,"%s\n", aDir->d_name);
    }
    fclose(new_file);
    // Close the directory
    closedir(currDir);
}

// Get the status when a background child process is completed
void amber_alert(){
    int status;
    pid_t pid;
    pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) {
        rm_flag = pid;
    }
}

// Start by creating a new shell process in the terminal window which returns a : to let the
// user know to start typing commands.

int main(){
    // Get the Pid of the main process and set it as the head node
    int mainPid = getpid();
    struct Node* head = NULL;
    head = (struct Node*)malloc(sizeof(struct Node));
    head->data = mainPid;
    head->next = NULL;
    signal(SIGCHLD, amber_alert);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, &on);
    int chld_count[200];
    
    
    int last_process = mainPid;

    int digit = 0;
    int n = mainPid;
    char path[2048];

    char input[2048];
    char pass_input[2048];
    char command[256];
    char args[256];

    int status;
    int output;

    while (strcmp(input, "exit\n") != 0){
        if (rm_flag!=0){
            waitpid(rm_flag,&status,WNOHANG);
            printf("Exited with status %ld\n", WEXITSTATUS(status));
        }
        cleanList(head);
        // Each line in the shell starts with : so that the user knows to enter a command
        printf(": ");
        fgets(input, 2048, stdin);
        strcpy(pass_input, input);
        

        // Tokenize the line , delimeted by " ", and save each section into an array
        int i = 0;
        int write_flag = 0;
        int count = 0;
        char *token = strtok(input, " ");
        char *input_array[200] = {NULL};
        while (token != NULL){
            count++;
            input_array[i++] = token;
            token = strtok(NULL, " ");
        }
        strcpy(command, input_array[0]);

        // Iterate through command and replace any instances of $ with values from Pid in order.

        if (strcmp(command, "exit\n") == 0){
            continue;
        }
        if (strcmp(command, "#\n") == 0){
            continue;
        }
        if (strcmp(command, "\n") == 0){
            continue;
        }
        if (strcmp(command, "status\n") == 0){

            printf("Last Process: %ld\n");
            waitpid(last_process,&status, WNOHANG);
            if (WIFEXITED(status))
            {
                waitpid(last_process, &status, WNOHANG);
                printf("Exited with status %d\n", WEXITSTATUS(status));
                //printf("removing node %i\n", spawnPid);
            } else if (WIFSIGNALED(status))
            {
                int signum = WTERMSIG(status);
                printf("%d exited due to signal %d\n", signum);
            } else {
                printf("Something else happened.\n");
            }
            continue;
        }
        if (strcmp(command, "ls\n") == 0){
            list_contents();
            continue;
        }
        if (strcmp(command, "ls") == 0){
            if (*input_array[1] = '>'){
                input_array[2][strcspn(input_array[2], "\n")] = 0;
                list_to_file(input_array[2]);
            }else {
                list_contents();
            }
            continue;
        }
        if (strcmp(command, "cd\n") == 0){
            // if no argument is given with "cd" command, change directory to HOME directory
            chdir(getenv("HOME"));
            continue;
        }
        if (strcmp(command, "cd") == 0){
            input_array[1][strcspn(input_array[1], "\n")] = 0;
            change_directory(input_array[1]);
            continue;

        } else {
            int childPid;
            int status;
            i = 0;
            count = 0;
            char temp[2048];
            char *saveptr;
            char *token2 = strtok_r(pass_input, " \t\r\n", &saveptr);
            char *pass_array[200] = {NULL};
            while (token2 != NULL){
                // Check for < mark for input, pass if found
                if (strcmp(token2, "<")==0){
                    token2 = strtok_r(NULL, " \t\r\n", &saveptr);
                    continue;
                }
                // Check for > mark for output, set write_flag to index
                if (strcmp(token2, ">")==0){
                    write_flag = i;
                    token2 = strtok_r(NULL, " \t\r\n", &saveptr);
                    continue;
                }
                // Check for & mark to specify process should happen in background
                // If in forground mode ignore &
                if (strcmp(token2, "&")==0){
                    if (sig_flag != 0){
                        token2 = NULL;
                        continue;
                    } else{
                        bg_flag = 1;
                        token2 = NULL;
                        continue;
                    }
                }
                count++;
                pass_array[i++] = token2;
                token2 = strtok_r(NULL, " \t\r\n", &saveptr);
            }

            int childStatus;
            // Fork a new process
            //printf("Forked Child process\n");
            pid_t spawnPid = fork();


            
            
            switch(spawnPid){
            case -1:
                printf("Someting Messed up!\n");
                perror("fork()\n");
                exit(1);
                break;
            case 0:
                signal(SIGTSTP, SIG_IGN);
                if (write_flag > 0){
                    // If write flag is not 0, open/create the necessary file and write
                    // output to file
                    char filename[2048];
                    strcpy(filename, pass_array[write_flag]);
                    pass_array[write_flag] = NULL;
                    printf("%s\n", filename);
                    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    dup2(fd,STDOUT_FILENO);
                    signal(SIGINT, inter_sig);
                    int status_code = execvp(pass_array[0], pass_array);
                    dup2(fd,STDOUT_FILENO);
                    close(fd);
                    perror("execve");
                    exit(1);

                }else{
                    signal(SIGINT, inter_sig);
                    if (interupt_flag != 0){
                        interupt_flag = 0;
                        printf("interupted child process\n");
                    }
                    int status_code = execvp(pass_array[0], pass_array);
                    perror("execve");
                    exit(1);
                }

            default:
                // In the parent process
                // Wait for child's termination if background flag is not 0
                if (bg_flag == 0){
                    last_process = spawnPid;
                    spawnPid = waitpid(spawnPid, &childStatus, 0);
                } else{
                    printf("Background pid is %ld\n", spawnPid);
                    add_node(head, spawnPid);
                }
            }
        }
    }
    return 0;
}
