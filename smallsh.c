#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> // pid_t
#include <sys/stat.h>
#include <unistd.h> // fork
#include <dirent.h>
#include <sys/wait.h> // for waitpid



static int sig_flag = 0;

// A linked list node for PID linked list
struct Node {
    int data;
    struct Node* next;
};

// Set head as global object
static struct Node* head = NULL;

//Iterate through linked list of 
void printList(struct Node* head)
{
    while (head != NULL) {
        int status;
        pid_t pid;
        pid = waitpid(-1, &status, WNOHANG);
        printf("Process %ld status: %ld\n", head->data, WEXITSTATUS(pid));
        head = head->next;
        
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

// Get the status when a background child process is completed, then remove that node from the linked list
void amber_alert(){
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status))
            {
                fprintf(stdout,"background pid %d exited with status %d\n", pid, WEXITSTATUS(status));
                //printf("removing node %i\n", spawnPid);
                remove_node(&head, pid);
                fprintf(stdout,": ");
                fflush(stdout);
            }
    }
}

// Handle SIGTSTP to toggle Foreground Mode on and off
void sig_handle(){
    if(sig_flag == 0){
        fprintf(stdout,"\nNow Entering Foreground Only Mode (& is now ignored)\n");
        fflush(stdout);
        sig_flag = 1;
    } else {
        fprintf(stdout,"\nNow Exiting Foreground Only Mode (& is no longer ignored)\n");
        fflush(stdout);
        sig_flag = 0;
    }
}

// Start by creating a new shell process in the terminal window which returns a : to let the
// user know to start typing commands.

int main(){
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, sig_handle);
    // Get the Pid of the main process and convert that 
    // into an array to be used incase of $$
    int mainPid = getpid();
    head = (struct Node*)malloc(sizeof(struct Node));
    head->data = mainPid;
    head->next = NULL;

    signal(SIGCHLD, amber_alert);

    int digit = 0;
    int n = mainPid;
    char path[2048];
    while (n != 0)
    {
        n/=10;
        digit++;
    }
    int Pid_Array[digit];

    digit = 0;
    n = mainPid;
    while (n != 0) 
    {
        Pid_Array[digit] = n % 10;
        n/=10;
        digit++;
    }

    char input[2048];
    char pass_input[2048];
    char command[256];
    char args[256];

    while (strcmp(input, "exit\n") != 0){
        // Each line in the shell starts with : so that the user knows to enter a command
        printf(": ");
        fgets(input, 2048, stdin);
        strcpy(pass_input, input);
        

        // Tokenize the line , delimeted by " ", and save each section into an array
        int i = 0;
        int write_flag = 0;
        int bg_flag = 0;
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
        i = 0;
        int j = 0;
        size_t len = strlen(command);
        for (i=0; i<len; i++){
            if (command[i]=='$'){
                char num;
                num = Pid_Array[j] + '0';
                command[i] = num;
                j++;
            }
        }
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
            /*
            Linked List of PID nodes, when a process completes node is removed

            iterate through linked list and for each node print the status.

            run before prompting for a command

            */
            printList(head);
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
            char *token2 = strtok(pass_input, " \n");
            char *pass_array[200] = {NULL};
            while (token2 != NULL){
                // Check for < mark for input, pass if found
                if (strcmp(token2, "<")==0){
                    token2 = strtok(NULL, " ");
                    continue;
                }
                // Check for > mark for output, set write_flag to index
                if (strcmp(token2, ">")==0){
                    write_flag = i;
                    token2 = strtok(NULL, " ");
                    continue;
                }
                // Check for & mark to specify process should happen in background
                // If in forground mode ignore &
                if (strcmp(token2, "&\n")==0){
                    if (sig_flag != 0){
                        token2 = NULL;
                        continue;
                    } else{
                        bg_flag = 1;
                        token2 = strtok(NULL, " ");
                        continue;
                    }
                }
                count++;
                pass_array[i++] = token2;
                token2 = strtok(NULL, " ");
            }
            //printf("Finished While Loop.\n");
            for(i=0;i<count;i++){
                if (i == count-1){
                    pass_array[i][strcspn(pass_array[i], "\n")] = 0;
                }
                strcpy(temp, pass_array[i]);
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
                //printf("Before calling execvp()\n");
                if (write_flag > 0){
                    // If write flag is not 0, open/create the necessary file and write
                    // output to file
                    continue;

                }
                //printf("Executing execvp\n");
                if (bg_flag == 0){
                    //printf("Waiting on Child Process\n");
                    
                }
                signal(SIGINT, SIG_DFL);
                int status_code = execvp(command, pass_array);
                perror("execve");
                exit(-1);

            default:
                // In the parent process
                // Wait for child's termination if background flag is not 0
                if (bg_flag == 0){
                    //printf("Adding node for process: %ld\n", spawnPid);
                    add_node(head, spawnPid);
                    spawnPid = waitpid(spawnPid, &childStatus, 0);
                    if (WIFEXITED(childStatus))
                    {
                        //printf("forground pid %d exited with status %d\n", spawnPid, WEXITSTATUS(childStatus));
                        //printf("removing node %i\n", spawnPid);
                        remove_node(&head, spawnPid);
                    }
                } else{
                    printf("Background pid is %ld\n", spawnPid);
                    add_node(head, spawnPid);
                    //printf("Child Process in Background");
                
                }
                //printf("PARENT(%d): child(%d) terminated.\n", getpid(), spawnPid);
                continue;
            }
        }
    }
    return 0;

}
