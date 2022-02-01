#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h> // pid_t
#include <sys/stat.h>
#include <unistd.h> // fork
#include <dirent.h>

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

void change_directory(char *path){
    char  gdir[2048];
    char slash_start[2048] = "/";
    char path_start;
    path_start = path[0];

    
    getcwd(gdir, 2048);
    if (path_start != "/"){
        strcat(slash_start, path);
        strcat(gdir, slash_start);


        printf("%s\n", gdir);

        chdir(gdir);
    }

}


int main(){
    char input[2048];
    char command[256];
    char args[256];

    while (strcmp(input, "exit\n") != 0){
        // Each line in the shell starts with : so that the user knows to enter a command
        printf(": ");
        fgets(input, 2048, stdin);

        // Tokenize the line , delimeted by " ", and save each section into an array with the follow rules
        // Index 0 = command
        // Index 1 = arguments
        // Index 2 = input file
        // Index 3 = output file
        // Index 4 = ampersand (to determine background processes)
        int i = 0;
        char *token = strtok(input, " ");
        char *input_array[6] = {NULL, NULL, NULL, NULL, NULL};
        while (token != NULL){
            input_array[i++] = token;
            token = strtok(NULL, " ");
        }
        strcpy(command, input_array[0]);
        
        if (strcmp(command, "#\n") == 0){
            continue;
        }
        if (strcmp(command, "\n") == 0){
            continue;
        }
        if (strcmp(command, "ls\n") == 0){
            list_contents();
        }
        if (strcmp(command, "cd\n") == 0){
            chdir(getenv("HOME"));
        }
        if (strcmp(command, "cd") == 0){
            input_array[1][strcspn(input_array[1], "\n")] = 0;
            change_directory(input_array[1]);
        }
        
    }
    return 0;

}

