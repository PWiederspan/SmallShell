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
    int closedir(currDir);
    printf("\n");
}


int main(){
    char command[256];
    while (strcmp(command, "exit") != 0){
        printf(": ");
        scanf("%s", command);

        if (strcmp(command, "ls") == 0){
            list_contents();
        }
    }
    return 0;

}

