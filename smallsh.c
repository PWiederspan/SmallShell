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

void go_to_home(){
    chdir(getenv("HOME"));
}

int main(){
    char input[2048];
    char command[256];

    while (strcmp(input, "exit\n") != 0){
        printf(": ");
        fgets(input, 2048, stdin);
        int i = 0;
        char *token = strtok(input, " ");
        char *input_array[6];
        ///printf("2");
        while (token != NULL){
            input_array[i++] = token;
            token = strtok(NULL, " ");
        }
        strcpy(command, input_array[0]);
        
        if (strcmp(command, "ls\n") == 0){
            list_contents();
        }
        if (strcmp(command, "cd\n") == 0){
            go_to_home();
        }
        
    }
    return 0;

}

