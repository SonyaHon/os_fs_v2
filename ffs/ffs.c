#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ffs_io.c"
#include "ffs_flags.c"

short magic[3] = {0x79, 0x66, 0x73};

typedef struct __attribute__((__packed__)){
    short check_code[3];
    long files_c;
    long files_s;
} ffs_header;



typedef struct __attribute__((__packed__)){
    long unsigned char_c;
    char name[UINT8_MAX];
    char flags;
    long size_of_file;
} ffs_file_header;

typedef struct __attribute__((__packed__)){
    int file_desc;
    ffs_header* header;
    ffs_file_header* current_file;
    off_t current_file_offset;
} ffs_system;


typedef struct __attribute__((__packed__)){
    ffs_file_header* header;
    char* data;
} ffs_file_to_save;

typedef struct __attribute__((__packed__)){
    long file_no;
    off_t file_offset;
} ffs_iterator;


/**
 *
 * @return a new crafted pointer to initial ffs_header structure
 */
ffs_header* ffs_create_ffs_header() {
    ffs_header* header = malloc(sizeof(ffs_header));
    header->check_code[0] = 0x79;
    header->check_code[1] = 0x66;
    header->check_code[2] = 0x73;
    header->files_c = 0;
    header->files_s = 0;
    return  header;
}

/**
 *
 * @param filename - name of the file
 * @return pointer to the ffs_file_header structure
 */
ffs_file_header* ffs_create_file_header(char *filename, long file_size) {
    ffs_file_header* header = malloc(sizeof(ffs_file_header));
    header->char_c = strlen(filename);
    int i = 0;
    for(i; i < header->char_c; ++i) {
        header->name[i] = filename[i];
    }
    header->flags = FFS_FLAGS_NONE;
    header->size_of_file = file_size;
    return header;
}
/**
 *
 * Creates a file, and write ffs_header structure to it with default values
 *
 * @param filename - name of file to create
 * @return - 0 if all is ok and -1 if there where any errors
 */
int ffs_create_(char * filename) {

    int f = open(filename, O_CREAT | O_RDWR, 0666);
    if(f < 0) return -1;

    ffs_header* header = ffs_create_ffs_header();

    if (write(f, header, sizeof(ffs_header)) < 0) {
           return -1;
    }

    close(f);
    printf("File system %s created!\n", filename);
    return 0;
}

/**
 *
 * @param system - opened file system
 * @return the header of the system
 */
ffs_header* ffs_read_header(ffs_system* system) {
    ffs_header* header = malloc(sizeof(ffs_header));
    lseek(system->file_desc, 0, SEEK_SET);
    read(system->file_desc, header, sizeof(ffs_header));
    return header;
}


/**
 *
 * @param system - opened file system
 * @return 0 - if file is ok and -1 otherwise
 */
int ffs_check_file(ffs_system* system) {
    short *code = system->header->check_code;
    if(code[0] == magic[0] && code[1] == magic[1] && code[2] == magic[2])
        return 0;
    else
        return -1;
}

/**
 *
 * @param filename - name of the fs file
 * @return  - pointer to the ffs_system structure of the opened fs
 */
ffs_system* ffs_open_ffs(char * filename) {
    ffs_system* sys = malloc(sizeof(ffs_system));
    sys->file_desc = open(filename, O_RDWR);
    printf("Opened %s\n", filename);
    sys->header = ffs_read_header(sys);
    sys->current_file = 0;
    sys->current_file_offset = -1;
    if(ffs_check_file(sys) == -1) {
        printf("File %s is broken or it is not a file system file", filename);
        exit(-1);
    }
    printf("File %s is OK\n", filename);
    return sys;
}

/**
 *
 * @param system - opened file system
 * @return 0 - if closed successesfuly and -1 otherwise
 */
int ffs_close_ffs(ffs_system* system) {
    printf("Closing fs\n");


    ffs_file_to_save* final_build[system->header->files_c];
    lseek(system->file_desc, sizeof(ffs_header), SEEK_SET);

    int i = 0;
    ffs_file_header* header = malloc(sizeof(ffs_file_header));
    for(i; i < system->header->files_c; ++i) {
        read(system->file_desc, header, sizeof(ffs_file_header));
        if(header->flags == FFS_FLAGS_NONE) {
            //  add to final build
            ffs_file_to_save* sav = malloc(sizeof(ffs_file_to_save));
            sav->header = header;
            sav->data = malloc(header->size_of_file);
            read(system->file_desc, sav->data, header->size_of_file);
            final_build[i] = sav;
        }
        else {
            i -= 1; // add 1 more iteration
        }
    }



    lseek(system->file_desc, sizeof(ffs_header), SEEK_SET);
    off_t size = sizeof(ffs_header);
    for(i = 0; i < system->header->files_c; ++i) {
        write(system->file_desc, final_build[i]->header, sizeof(ffs_file_header));
        size += sizeof(ffs_file_header);
        write(system->file_desc, final_build[i]->data, final_build[i]->header->size_of_file);
        size += final_build[i]->header->size_of_file;
    }

    ftruncate(system->file_desc, size);

    if(close(system->file_desc) == -1) {
        perror("Error while closing fs file");
    }
}

void ffs_save_ffs(ffs_system* system) {
    ffs_file_to_save* final_build[system->header->files_c];
    lseek(system->file_desc, sizeof(ffs_header), SEEK_SET);

    int i = 0;
    ffs_file_header* header = malloc(sizeof(ffs_file_header));
    for(i; i < system->header->files_c; ++i) {
        read(system->file_desc, header, sizeof(ffs_file_header));
        if(header->flags == FFS_FLAGS_NONE) {
            //  add to final build
            ffs_file_to_save* sav = malloc(sizeof(ffs_file_to_save));
            sav->header = header;
            sav->data = malloc(header->size_of_file);
            read(system->file_desc, sav->data, header->size_of_file);
            final_build[i] = sav;
        }
        else {
            i -= 1; // add 1 more iteration
        }
    }

    lseek(system->file_desc, sizeof(ffs_header), SEEK_SET);
    off_t size = sizeof(ffs_header);
    for(i = 0; i < system->header->files_c; ++i) {
        write(system->file_desc, final_build[i]->header, sizeof(ffs_file_header));
        size += sizeof(ffs_file_header);
        write(system->file_desc, final_build[i]->data, final_build[i]->header->size_of_file);
        size += final_build[i]->header->size_of_file;
    }

    ftruncate(system->file_desc, size);
    printf("File system saved!\n");
}

/**
 *
 * @param filename - file to add
 * @param system - opened system
 * @return 0 if file added, -1 otherwise
 */
int ffs_append_file(char * filename, ffs_system* system) {
    struct stat fileStat;
    stat(filename, &fileStat);
    fflush(stdout);
    printf("Are u sure to append %s to the file system? (y/n)\n", filename);
    char ans;
    scanf("%c", &ans);
    scanf("%c", &ans);
    fflush(stdout);
    if(ans == 'y') {
        char name_save[UINT8_MAX];
        printf("Name: ");
        scanf("%s",name_save);

        ffs_file_header* header = ffs_create_file_header(name_save, fileStat.st_size);
        char buffer[header->size_of_file];
        int file = open(filename, O_RDONLY);
        read(file, buffer, header->size_of_file);
        close(file);

        // Write new system header
        system->header->files_c += 1;
        system->header->files_s += header->size_of_file;
        lseek(system->file_desc, 0, SEEK_SET);
        if(write(system->file_desc, system->header, sizeof(ffs_header)) == -1) return -1;
        // Write file header
        lseek(system->file_desc, 0, SEEK_END);
        if(write(system->file_desc, header, sizeof(ffs_file_header)) == -1) return -1;
        // Write the file
        if(write(system->file_desc, buffer, header->size_of_file) == -1) return -1;
        printf("File '%s' is appended as %s.\n", filename, name_save);
    }
    else {
        return 0;
    }
    return 0;
}

ffs_file_header* ffs_get_file_header_by_index(int idx, ffs_system* sys) {
    ffs_file_header* header = malloc(sizeof(ffs_file_header));
    lseek(sys->file_desc, sizeof(ffs_header), SEEK_SET);
    off_t offset = 0;
    int i = 0;
    for(i; i < idx; ++i) {
        read(sys->file_desc, header, sizeof(ffs_file_header));
        offset += sizeof(ffs_file_header);
        offset += header->size_of_file;
    }
    lseek(sys->file_desc, sizeof(ffs_header)+offset, SEEK_SET);
    read(sys->file_desc, header, sizeof(ffs_file_header));
    return header;
}


ffs_iterator* ffs_new_iterator() {
    ffs_iterator* iterator = malloc(sizeof(ffs_iterator));
    iterator->file_no = 0;
    iterator->file_offset = sizeof(ffs_header);
    return  iterator;
}

ffs_file_header* ffs_iter_threw(ffs_iterator* iter, ffs_system* system) {
    ffs_file_header* header = malloc(sizeof(ffs_file_header));

    lseek(system->file_desc, iter->file_offset, SEEK_SET);
    read(system->file_desc, header, sizeof(ffs_file_header));
    iter->file_no += 1;
    iter->file_offset += sizeof(ffs_file_header) + header->size_of_file;

    return header;
}

void ffs_list_all_files(ffs_system* sys) {
    printf("Total files count: %ld\nTotal files size: %ld\n----Filename---- ----Size----\n", sys->header->files_c, sys->header->files_s);
    ffs_iterator* it = ffs_new_iterator();
    int i = 0;
    for(i; i < sys->header->files_c; ++i) {
        ffs_file_header* file = ffs_iter_threw(it, sys);
        if(file->flags != FFS_FLAGS_DELETE)
        printf("%*s%*ld\n", 16, file->name, 12, file->size_of_file);
    }
    printf("-----------------------------\n");

}

void ffs_open_file_at_index(int index, ffs_system* system) {
    ffs_file_header* header = malloc(sizeof(ffs_file_header));
    header = ffs_get_file_header_by_index(index, system);
    system->current_file = header;
    system->current_file_offset = lseek(system->file_desc, 0, SEEK_CUR);
    printf("File: %s\n-----------------------------\n", header->name);
    char buff[header->size_of_file];
    read(system->file_desc, buff, header->size_of_file);
    printf("%s\n\n-----------------------------\n", buff);
}

void ffs_open_file_by_name(char* name, ffs_system* system) {
    ffs_iterator* it = ffs_new_iterator();
    int i = 0;
    for(i; i < system->header->files_c; ++i) {
        ffs_file_header* file = ffs_iter_threw(it, system);
        if(strcmp(name, file->name) == 0)  {
            system->current_file_offset = lseek(system->file_desc, 0, SEEK_CUR);
            system->current_file = file;
            printf("File: %s\n-----------------------------\n", file->name);
            char buff[file->size_of_file];
            read(system->file_desc, buff, file->size_of_file);
            printf("%s\n\n-----------------------------\n", buff);
            break;
        }
    }
}

int ffs_delete_file(ffs_system* system) {
    if(system->current_file == 0) {
        printf("No file choosen, please open a file before deleting\n");
        return 0;
    }
    else {
        ffs_file_header* file = system->current_file;
        file->flags = FFS_FLAGS_DELETE;
        system->header->files_c -= 1;
        system->header->files_s -= file->size_of_file;
        lseek(system->file_desc, 0, SEEK_SET);
        if(write(system->file_desc, system->header, sizeof(ffs_header)) == -1) return -1;
        lseek(system->file_desc, system->current_file_offset - sizeof(ffs_file_header), SEEK_SET);
        if(write(system->file_desc, file, sizeof(ffs_file_header)) == -1 ) return -1;
    }
    return 0;
}