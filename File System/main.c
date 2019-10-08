#include <stdio.h>
#include <string.h>

struct r_d
{
    int nDirectories;	//How many subdirectories are in the root
    //Needs to be less than MAX_DIRS_IN_ROOT
    struct directory
    {
        char dname[100];	//directory name (plus space for nul)
        long nStartBlock;				//where the directory block is on disk
    } __attribute__((packed)) directories[100];	//There is an array of these

    //This is some space to get this to be exactly the size of the disk block.
    //Don't use it for anything.
    char padding[100 * sizeof(struct directory) - sizeof(int)];
} ;

int main() {
//    struct r_d r = {1};
//    strcpy(r.directories[0].dname,"Yes");
//    struct directory d = r.directories[0];
//    printf("Name: %s\n",d.dname);
//    char path[]="/test.txt";
//    char directory[100+1];
//    char filename[100+1];
//    char extension[100+1];
//    strcpy(directory, "");
//    strcpy(filename, "");
//    strcpy(extension, "");
//    sscanf(path, "/%[^/]/%[^.].%s", directory, filename, extension);
//    printf("Dir: %s\n",directory);
//    printf("Fil: %s\n",filename);
//    printf("Ext: %s\n",extension);
//    char a[10];
//    printf(a);
//    printf("%d\n",strcmp(a,""));
//    printf("%d\n",strcmp(a,"yes"));
//    printf("%d\n",strcmp(a,"yes"));
//    char a[] = "";
//    char* destination = strtok(a, "/");
//    char* filename = strtok(NULL, "");
//    char* extension = strtok(NULL, "");
//    printf("Dest: %s\n",destination);
//    printf("Fil: %s\n",filename);
//    printf("Ext: %s\n",extension);
    printf("%d\n",3+4>5?1:0);
    return 0;
}