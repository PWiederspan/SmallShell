#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> // pid_t
#include <sys/stat.h>
#include <unistd.h> // fork
#include <dirent.h>
#include <sys/wait.h> // for waitpid

// Start by creating a new shell process in the terminal window which returns a : to let the
// user know to start typing commands.

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

int main(){
    // Get the Pid of the main process and convert that into an array to be used incase of $$
    int mainPid = getpid();
    //printf("%i\n", mainPid);
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
        

        // Tokenize the line , delimeted by " ", and save each section into an array with the follow rules
        // Index 0 = command
        // Index 1 = arguments
        // Index 2 = input file
        // Index 3 = output file
        // Index 4 = ampersand (to determine background processes)
        int i = 0;
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

            Need to code a function which keeps track of Pid status before ending
            each call. This command will print the status of each process.

            */
            continue;
        }
        if (strcmp(command, "ls\n") == 0){
            list_contents();
            continue;
        }
        if (strcmp(command, "ls") == 0){
            if (*input_array[1] = '>'){
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
            int write_flag = 0;
            int bg_flag = 0;

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
                if (strcmp(token2, "&")==0){
                    bg_flag = 1;
                    token2 = strtok(NULL, " ");
                    continue;
                }
                count++;
                pass_array[i++] = token2;
                token2 = strtok(NULL, " ");
        }
        for(i=0;i<count;i++){
            if (i == count-1){
                pass_array[i][strcspn(pass_array[i], "\n")] = 0;
            }
            strcpy(temp, pass_array[i]);
        }

            int childStatus;

	        // Fork a new process
	        pid_t spawnPid = fork();

            switch(spawnPid){
	        case -1:
                perror("fork()\n");
		        exit(1);
		        break;
            case 0:
                printf("Before calling execvp()\n");
                if (write_flag > 0){
                    // If write flag is not 0, open/create the necessary file and write
                    // output to file
                    continue;

                }
                int status_code = execvp(command, pass_array);
                perror("execve");
                exit(2);
		        break;
            default:
                // In the parent process
                // Wait for child's termination
                spawnPid = waitpid(spawnPid, &childStatus, 0);
                printf("PARENT(%d): child(%d) terminated.\n", getpid(), spawnPid);
		        break;
            }
        }
    }
    return 0;

}