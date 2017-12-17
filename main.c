
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ffs/ffs.c"

int main(int argc, char  ** argv) {

    //-c(reate) [name] --create [filename] flag - creates the file system and opens it
    //-o(pen) [path] --open [path] flag - opens already existing file system by path
    // if launched without params - just exit

    if(argc == 1) {
        printf("No flags are specified, use --help to see more...\n");
        exit(0);
    }
    else {
        if(argc == 2 && strcmp(argv[1], "--help") == 0) {
            printf("\t -c [filename] --create [filename] - creates new file system with file name and opens it."
                           "\n\t -o [filename] --open [filename] - opens existing file system\n");
        }
        else if(argc == 3) {
            char * command = argv[1];
            char * arg = argv[2];
            if(strcmp(command, "-c") == 0 || strcmp(command, "--create") == 0) { // create a file
                if(ffs_create_(arg) == -1) {
                    perror("Error: ");
                    return -1;
                }
                ffs_system* sys = ffs_open_ffs(arg);


                ffs_close_ffs(sys);
            }
            else if(strcmp(command, "-o") == 0 || strcmp(command, "--open") == 0) { // open a file
                ffs_system* sys = ffs_open_ffs(arg);
                while(1) {
                    char cmd[UINT8_MAX];
                    printf("Command: ");
                    scanf("%s", cmd);

                    if(strcmp(cmd, "quit") == 0) {
                        ffs_close_ffs(sys);
                        exit(0);
                    }
                    else if(strcmp(cmd, "list") == 0) {
                        ffs_list_all_files(sys);
                    }
                    else if (strcmp(cmd, "open") == 0){
                        printf("Specify file name: ");
                        char filename[UINT8_MAX];
                        scanf("%s", filename);
                        ffs_open_file_by_name(filename, sys);
                    }
                    else if(strcmp(cmd,  "append") == 0) {
                        printf("Specify file name: ");
                        char filename[UINT8_MAX];
                        scanf("%s", filename);
                        ffs_append_file(filename, sys);
                    }
                    else if(strcmp(cmd, "delete") == 0) {
                        ffs_delete_file(sys);
                    }
                    else if(strcmp(cmd, "save") == 0) {
                        ffs_save_ffs(sys);
                    }
                    else {
                        printf("\topen\topen a file\n\tlist\tshow all files\n\tdelete\tdelete currently opened file\n\tquit\tclose the file system\n");
                    }

                }

            }
        }
        else {
            printf("No flags are specified, use --help to see more...\n");
            exit(0);
        }

    }

    return 0;
}