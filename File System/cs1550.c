/*
	FUSE: Filesystem in Userspace
	Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

	This program can be distributed under the terms of the GNU GPL.
	See the file COPYING.
*/

#define	FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

//size of a disk block
#define	BLOCK_SIZE 512

//we'll use 8.3 filenames
#define	MAX_FILENAME 8
#define	MAX_EXTENSION 3

//How many files can there be in one directory?
#define MAX_FILES_IN_DIR (BLOCK_SIZE - sizeof(int)) / ((MAX_FILENAME + 1) + (MAX_EXTENSION + 1) + sizeof(size_t) + sizeof(long))

//The attribute packed means to not align these things
struct cs1550_directory_entry
{
	int nFiles;	//How many files are in this directory.
				//Needs to be less than MAX_FILES_IN_DIR

	struct cs1550_file_directory
	{
		char fname[MAX_FILENAME + 1];	//filename (plus space for nul)
		char fext[MAX_EXTENSION + 1];	//extension (plus space for nul)
		size_t fsize;					//file size
		long nStartBlock;				//where the first block is on disk
	} __attribute__((packed)) files[MAX_FILES_IN_DIR];	//There is an array of these

	//This is some space to get this to be exactly the size of the disk block.
	//Don't use it for anything.  
	char padding[BLOCK_SIZE - MAX_FILES_IN_DIR * sizeof(struct cs1550_file_directory) - sizeof(int)];
} ;

typedef struct cs1550_root_directory cs1550_root_directory;

#define MAX_DIRS_IN_ROOT (BLOCK_SIZE - sizeof(int)) / ((MAX_FILENAME + 1) + sizeof(long))

struct cs1550_root_directory
{
	int nDirectories;	//How many subdirectories are in the root
						//Needs to be less than MAX_DIRS_IN_ROOT
	struct cs1550_directory
	{
		char dname[MAX_FILENAME + 1];	//directory name (plus space for nul)
		long nStartBlock;				//where the directory block is on disk
	} __attribute__((packed)) directories[MAX_DIRS_IN_ROOT];	//There is an array of these

	//This is some space to get this to be exactly the size of the disk block.
	//Don't use it for anything.  
	char padding[BLOCK_SIZE - MAX_DIRS_IN_ROOT * sizeof(struct cs1550_directory) - sizeof(int)];
} ;


typedef struct cs1550_directory_entry cs1550_directory_entry;

//How much data can one block hold?
#define	MAX_DATA_IN_BLOCK (BLOCK_SIZE - sizeof(long))

struct cs1550_disk_block
{
	//The next disk block, if needed. This is the next pointer in the linked 
	//allocation list
	long nNextBlock;

	//And all the rest of the space in the block can be used for actual data
	//storage.
	char data[MAX_DATA_IN_BLOCK];
};

typedef struct cs1550_disk_block cs1550_disk_block;

static cs1550_root_directory read_root(){
    FILE* disk = fopen(".disk", "r+b");
    fseek(disk, 0, SEEK_SET);
    cs1550_root_directory root;
    fread(&root, BLOCK_SIZE, 1, disk);
    fclose(disk);
    return root;
}

static void write_root(cs1550_root_directory* root_on_disk){
    FILE* disk = fopen(".disk", "r+b");
    fwrite(root_on_disk, BLOCK_SIZE, 1, disk);
    fclose(disk);
}

#define MAX_FAT_ENTRIES 1024
#define BLOCKS_FOR_FAT MAX_FAT_ENTRIES*sizeof(short)/BLOCK_SIZE
#define START_ALLOC_BLOCK 1+BLOCKS_FOR_FAT     // start block allocation

struct cs1550_fat {
    short table[MAX_FAT_ENTRIES];
};


typedef struct cs1550_fat cs1550_fat;

static cs1550_fat read_fat(){
    FILE* disk = fopen(".disk", "r+b");
    fseek(disk, BLOCK_SIZE, SEEK_SET);
    cs1550_fat fat;
    fread(&fat, BLOCKS_FOR_FAT*BLOCK_SIZE, 1, disk);
    fclose(disk);
    return fat;
}

static void write_fat(cs1550_fat* fat_on_disk){
    FILE* disk = fopen(".disk", "r+b");
    fseek(disk, BLOCK_SIZE, SEEK_SET);
    fwrite(fat_on_disk, BLOCKS_FOR_FAT*BLOCK_SIZE, 1, disk);
    fclose(disk);
}

/*
 * Called whenever the system wants to know the file attributes, including
 * simply whether the file exists or not. 
 *
 * man -s 2 stat will show the fields of a stat structure
 */
static int cs1550_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
   
	//is path the root dir?
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
        char directory[MAX_FILENAME+1];
        char filename[MAX_FILENAME+1];
        char extension[MAX_EXTENSION+1];
        strcpy(directory, "");
        strcpy(filename, "");
        strcpy(extension, "");
        sscanf(path, "/%[^/]/%[^.].%s", directory, filename, extension);
        if(directory==NULL || strcmp(directory,"")==0){
            res = -ENOENT;
            return res;
        }
        cs1550_root_directory root = read_root();
        int i;
        for(i=0;i<MAX_DIRS_IN_ROOT;i++){
            struct cs1550_directory current_directory = root.directories[i];
            if(strcmp(current_directory.dname,directory) == 0){       // find the directory
                if(strcmp(filename,"") == 0){       // if only looking for subdirectory attributes
                    res = 0;
                    stbuf->st_mode = S_IFDIR | 0755;
                    stbuf->st_nlink = 2;
                    return res; //Return a success
                }else{  // try to find the file
                    FILE* disk = fopen(".disk", "r+b");
                    int start_block = BLOCK_SIZE*current_directory.nStartBlock;
                    fseek(disk, start_block, SEEK_SET);
                    cs1550_directory_entry dir_entry;
                    int read_success = fread(&dir_entry, BLOCK_SIZE, 1, disk);
                    fclose(disk);
                    if(read_success){     // if read block success
                        int j;
                        for(j=0;j<MAX_FILES_IN_DIR;j++){
                            struct cs1550_file_directory current_file = dir_entry.files[j];
                            if(strcmp(current_file.fname,filename) == 0 && strcmp(current_file.fext,extension) == 0){
                                res = 0;
                                stbuf->st_mode = S_IFREG | 0666;
                                stbuf->st_nlink = 1;
                                stbuf->st_size = current_file.fsize;
                                return res; //Return success
                            }
                        }
                    }
                }
            }
        }
		//Else return that path doesn't exist
		res = -ENOENT;
	}
	return res;
}

/* 
 * Called whenever the contents of a directory are desired. Could be from an 'ls'
 * or could even be when a user hits TAB to do autocompletion
 */
static int cs1550_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	//Since we're building with -Wall (all warnings reported) we need
	//to "use" every parameter, so let's just cast them to void to
	//satisfy the compiler
	(void) offset;
	(void) fi;


	//the filler function allows us to add entries to the listing
	//read the fuse.h file for a description (in the ../include dir)
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

    if(strcmp(path,"/") == 0){
        cs1550_root_directory root = read_root();
        int i;
        for(i=0;i<MAX_DIRS_IN_ROOT;i++){
            char* name = root.directories[i].dname;
            if(strcmp(name,"")!=0)  filler(buf,name,NULL,0);
        }
        return 0;
    } else{
        int path_length = strlen(path);
        char path_copy[path_length];
        strcpy(path_copy, path);

        char* directory = strtok(path_copy, "/");
        char* leftover = strtok(NULL, "");

        if(directory == NULL || strcmp(directory,"")== 0 || strlen(directory) > MAX_FILENAME){ //check if name is too long
            return -ENOENT;
        }
        else if(leftover && leftover[0]){ //check if the path is a directory
            return -ENOENT;
        }
        cs1550_root_directory root = read_root();
        int i;
        for(i=0;i<MAX_DIRS_IN_ROOT;i++){
            struct cs1550_directory dir = root.directories[i];
            if(strcmp(directory,dir.dname) == 0){
                FILE* disk = fopen(".disk", "rb+");
                int location_on_disk = dir.nStartBlock*BLOCK_SIZE;
                fseek(disk, location_on_disk, SEEK_SET);

                cs1550_directory_entry directory;   // read the corresponding directory entry
                fread(&directory, BLOCK_SIZE, 1, disk); //Read the directory data from memory to iterate over its files
                fclose(disk);

                int j;
                for(j=0;j<MAX_FILES_IN_DIR;j++){
                    struct cs1550_file_directory file_dir = directory.files[j];
                    char filename_copy[MAX_FILENAME+1];
                    strcpy(filename_copy, file_dir.fname);
                    if(strcmp(file_dir.fext, "") != 0){ //Append the file extension
                        strcat(filename_copy, ".");
                    }
                    strcat(filename_copy, file_dir.fext); //Append file extension
                    if(strcmp(file_dir.fname, "") != 0){ //If the file is not empty, add it to the filler buffer
                        filler(buf, filename_copy, NULL, 0);
                    }
                }
                return 0;
            }
        }
        return -ENOENT;
    }
	/*
	//add the user stuff (subdirs or files)
	//the +1 skips the leading '/' on the filenames
	filler(buf, newpath + 1, NULL, 0);
	*/
	return 0;
}

/* 
 * Creates a directory. We can ignore mode since we're not dealing with
 * permissions, as long as getattr returns appropriate ones for us.
 */
static int cs1550_mkdir(const char *path, mode_t mode)
{
	(void) path;
	(void) mode;
    char* directory; //The directory in the 2-level file system
    char* leftover;

    //Parse the two strings
    int path_length = strlen(path);
    char path_copy[path_length];
    strcpy(path_copy, path);

    directory = strtok(path_copy, "/");
    leftover = strtok(NULL, "");
    printf("Dir: %s Leftover: %s\n",directory,leftover);
    if(strlen(directory) > MAX_FILENAME){ //check if name is too long
        return -ENAMETOOLONG;
    } else if(leftover && leftover[0]){ //check if the directory is in root
        printf("This is not a directory\n");
        return -EPERM;
    }

    cs1550_root_directory root = read_root();
    cs1550_fat fat = read_fat();

    if(root.nDirectories >= MAX_DIRS_IN_ROOT){
        printf("Reaching maximum\n");
        return -EPERM; //Can't add anymore directories
    }

    int i;
    for(i = 0; i < MAX_DIRS_IN_ROOT; i++){ //check if the directory already exists
        if(strcmp(root.directories[i].dname, directory) == 0){
            printf("Directory already exist\n");
            return -EEXIST;
        }
    }

    for(i=0;i<MAX_DIRS_IN_ROOT;i++){
        if(strcmp(root.directories[i].dname,"") == 0){  // if find an empty slot
            struct cs1550_directory new_dir_in_root;
            strcpy(new_dir_in_root.dname, directory); //Copy the user's new directory name into this struct
            int j;
            for(j=START_ALLOC_BLOCK;j<MAX_FAT_ENTRIES;j++){ // allocate a block to contain directory entry
                if(fat.table[j] == 0){
                    fat.table[j] = -1;
                    new_dir_in_root.nStartBlock = j;
                    break;
                }
            }
            FILE* disk = fopen(".disk", "r+b");
            int location_on_disk = BLOCK_SIZE*new_dir_in_root.nStartBlock;
            fseek(disk, location_on_disk, SEEK_SET);
            cs1550_directory_entry dir;
            dir.nFiles = 0;     // initialize the directory entry
            if(fread(&dir, BLOCK_SIZE, 1, disk)){
                fwrite(&dir, BLOCK_SIZE, 1, disk); //Write the new directory data
                root.nDirectories++;
                root.directories[i] = new_dir_in_root;
                write_root(&root);
                write_fat(&fat);
            }
            fclose(disk);
            printf("Created an directory:%s\n",new_dir_in_root.dname);
            return 0;
        }
    }
    return 0;
}

/* 
 * Removes a directory.
 */
static int cs1550_rmdir(const char *path)
{
	(void) path;
    return 0;
}

/* 
 * Does the actual creation of a file. Mode and dev can be ignored.
 *
 */
static int cs1550_mknod(const char *path, mode_t mode, dev_t dev)
{
	(void) mode;
	(void) dev;
    char* directory; //The first directory in the 2-level file system
    char* filename; //The directory within the root's directory
    char* extension;

    //Parse the two strings
    int path_length = strlen(path);
    char path_copy[path_length];
    strcpy(path_copy, path);

    directory = strtok(path_copy, "/");
    filename = strtok(NULL, ".");
    extension = strtok(NULL, ".");

    if(directory == NULL || strcmp(directory,"")==0)  return -EPERM;    // if input is null
    if(filename == NULL || strcmp(filename,"")==0)    return -EPERM;    // if creating file under root
    if(strlen(filename)>MAX_FILENAME || strlen(extension)>MAX_EXTENSION) return -ENAMETOOLONG; // if name invalid

    cs1550_root_directory root = read_root();
    cs1550_fat fat = read_fat();
    int i;
    for(i=0;i<MAX_DIRS_IN_ROOT;i++){
        struct cs1550_directory current_directory = root.directories[i];

        if(strcmp(current_directory.dname,directory) == 0){     // if find the directory
            long dir_location_on_disk = BLOCK_SIZE*current_directory.nStartBlock;
            FILE* disk = fopen(".disk", "r+b");
            fseek(disk, dir_location_on_disk, SEEK_SET);
            cs1550_directory_entry directory_entry;
            int read_success = fread(&directory_entry,BLOCK_SIZE,1,disk);
            if (directory_entry.nFiles >= MAX_FILES_IN_DIR)     return -EPERM;

            if(read_success){
                int free_index = -1;
                int j;
                for(j=0;j<MAX_FILES_IN_DIR;j++){       // search for free location or existed file
                    struct cs1550_file_directory current_file = directory_entry.files[j];
                    if(strcmp(current_file.fname,"") == 0 && strcmp(current_file.fext,"")==0 && free_index == -1)
                        free_index = j;
                    if(strcmp(current_file.fname,filename)==0 && strcmp(current_file.fext,extension)==0){
                        fclose(disk);
                        return -EEXIST;
                    }
                }
                short start_block = -1;
                int k;
                for(k=START_ALLOC_BLOCK;k<MAX_FAT_ENTRIES;k++){         // allocate a block in fat
                    if(fat.table[k] == 0){
                        start_block = k;
                        fat.table[k] = -1;
                        break;
                    }
                }
                struct cs1550_file_directory new_file_dir;      // create a file directory
                strcpy(new_file_dir.fname, filename);
                if(extension && extension[0]) strcpy(new_file_dir.fext, extension); //Add the file extension to the file entry
                else strcpy(new_file_dir.fext, "");
                new_file_dir.fsize = 0;
                new_file_dir.nStartBlock = start_block;

                directory_entry.files[free_index] = new_file_dir;   // insert into the directory entry
                directory_entry.nFiles++;
                fseek(disk, dir_location_on_disk, SEEK_SET);
                fwrite(&directory_entry, BLOCK_SIZE, 1, disk);
                fclose(disk);
                write_root(&root);
                write_fat(&fat);
            }

        }

    }
	return 0;
}

/*
 * Deletes a file
 */
static int cs1550_unlink(const char *path)
{
    (void) path;

    return 0;
}

/* 
 * Read size bytes from file into buf starting from offset
 *
 */
static int cs1550_read(const char *path, char *buf, size_t size, off_t offset,
			  struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	(void) path;

	//check to make sure path exists
	//check that size is > 0
	//check that offset is <= to the file size
	//read in data
	//set size and return, or error
    //path will be in the format of /directory/sub_directory
    char* directory; //The first directory in the 2-level file system
    char* filename; //The directory within the root's directory
    char* extension;

    //Parse the two strings
    int path_length = strlen(path);
    char path_copy[path_length];
    strcpy(path_copy, path);
    directory = strtok(path_copy, "/");
    filename = strtok(NULL, "."); //NULL indicates to continue where strtok left off at
    extension = strtok(NULL, ".");

    if(directory==NULL || strcmp(directory,"") == 0)    return -EISDIR;
    if(filename == NULL || strcmp(filename,"") == 0)    return -EISDIR;
    if(strlen(filename)>MAX_FILENAME || strlen(extension)>MAX_EXTENSION) return -ENAMETOOLONG; // if name invalid

    cs1550_root_directory root = read_root();
    cs1550_fat fat = read_fat();

    int i;
    for(i=0;i<MAX_DIRS_IN_ROOT;i++){
        struct cs1550_directory curr_dir = root.directories[i];
        if(strcmp(directory,curr_dir.dname) == 0){      // find the directory
            long dir_location_on_disk = BLOCK_SIZE*curr_dir.nStartBlock;
            FILE* disk = fopen(".disk", "r+b");
            fseek(disk, dir_location_on_disk, SEEK_SET);
            cs1550_directory_entry dir_entry;
            int read_success = fread(&dir_entry, BLOCK_SIZE, 1, disk);  // read the directory file
            fclose(disk);

            if(read_success){
                struct cs1550_file_directory file_dir;
                int j;
                for(j = 0; j < MAX_FILES_IN_DIR; j++){
                    struct cs1550_file_directory curr_file_dir = dir_entry.files[j];
                    if(strcmp(curr_file_dir.fname, filename) == 0){
                        if(extension && extension[0]){
                            if(strcmp(curr_file_dir.fext, extension) == 0){ // find the file
                                file_dir = curr_file_dir;
                                break;
                            }
                        } else{ //File extension null check
                            if(strcmp(curr_file_dir.fext, "") == 0){ // find the file
                                file_dir = curr_file_dir;
                                break;
                            }
                        }
                    }
                }
                /* find the file */
                if(strcmp(file_dir.fname, "") != 0) { //Filename is empty, so don't continue
                    if (offset >= file_dir.fsize) { //Offset is bigger than the file size
                        return -EFBIG;
                    }
                    int skip_blocks = offset/BLOCK_SIZE;
                    int block_offset = offset - skip_blocks*BLOCK_SIZE;
                    int curr_block = file_dir.nStartBlock;
                    while(skip_blocks > 0){
                        curr_block = fat.table[curr_block];
                        skip_blocks--;
                    }
                    /* read the file */
                    FILE* disk = fopen(".disk", "r+b");
                    fseek(disk, BLOCK_SIZE*curr_block+block_offset, SEEK_SET);
                    cs1550_disk_block new_data;
                    fread(&new_data.data, BLOCK_SIZE-block_offset, 1, disk);
                    int curr_buffer_size = 0;
                    int size_to_read = file_dir.fsize - offset;
                    if(size_to_read >= BLOCK_SIZE-block_offset){       // need read >= actual read
                        memcpy(buf, &new_data.data, BLOCK_SIZE-block_offset);
                        curr_buffer_size += BLOCK_SIZE - block_offset; //Increase the size of the buffer
                        size_to_read -= BLOCK_SIZE - block_offset;
                    } else{
                        memcpy(buf, &new_data.data, size_to_read);
                        curr_buffer_size = size_to_read; //Increase the size of the buffer
                        size_to_read = 0;
                    }
                    while(fat.table[curr_block] != -1){
                        curr_block = fat.table[curr_block];
                        cs1550_disk_block data;
                        fseek(disk, BLOCK_SIZE*curr_block, SEEK_SET);
                        fread(&data.data, BLOCK_SIZE, 1, disk);
                        if(size_to_read >= BLOCK_SIZE){
                            memcpy(buf+curr_buffer_size, &new_data.data, BLOCK_SIZE);
                            curr_buffer_size += BLOCK_SIZE;
                            size_to_read -= BLOCK_SIZE;
                        } else{
                            memcpy(buf, &new_data.data, size_to_read);
                            curr_buffer_size += size_to_read;
                            size_to_read = 0;
                        }
                    }
                    fclose(disk);

                    //Write the root and FAT back to disk
                    write_root(&root);
                    write_fat(&fat);
                    return size;
                }else      return -EISDIR;  /* end reading the file */
            }else 	return -EPERM;  /* end reading directory file */
        }
    }
    return -EPERM;      // directory does not exist
}

/* 
 * Write size bytes from buf into file starting from offset
 *
 */
static int cs1550_write(const char *path, const char *buf, size_t size, 
			  off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	(void) path;

	//check to make sure path exists
	//check that size is > 0
	//check that offset is <= to the file size
	//write data
	//set size (should be same as input) and return, or error
    char* directory; //The first directory in the 2-level file system
    char* filename; //The directory within the root's directory
    char* extension;
    //Parse the two strings
    int path_length = strlen(path);
    char path_copy[path_length];
    strcpy(path_copy, path);

    directory = strtok(path_copy, "/");
    filename = strtok(NULL, "."); //NULL indicates to continue where strtok left off at
    extension = strtok(NULL, ".");

    if(directory == NULL || strcmp(directory,"") == 0)  return -EEXIST;
    if(filename == NULL || strcmp(filename,"")==0)    return -EEXIST;    // if file is null
    if(strlen(filename)>MAX_FILENAME || strlen(extension)>MAX_EXTENSION) return -ENAMETOOLONG; // if name invalid
    if(size <= 0)   return -EPERM;

    cs1550_root_directory root = read_root();
    cs1550_fat fat = read_fat();
    int i;
    for(i=0;i<MAX_DIRS_IN_ROOT;i++){
        struct cs1550_directory current_directory = root.directories[i];
        if(strcmp(directory,current_directory.dname)==0){           // the directory is found
            long dir_location_on_disk = BLOCK_SIZE*current_directory.nStartBlock;
            FILE* disk = fopen(".disk", "r+b");
            fseek(disk, dir_location_on_disk, SEEK_SET);
            cs1550_directory_entry dir_entry;
            int read_success = fread(&dir_entry, BLOCK_SIZE, 1, disk);
            fclose(disk);
            if(read_success){   // the directory entry is read
                int found = 0;
                struct cs1550_file_directory file_dir;
                int file_directory_index = -1;
                int j;

                for(j=0;j<MAX_FILES_IN_DIR;j++){
                    struct cs1550_file_directory curr_file_dir = dir_entry.files[j];
                    if(strcmp(curr_file_dir.fname, filename) == 0){ //Found a matching filename
                        if(extension && extension[0]){ //Check the extention
                            if(strcmp(curr_file_dir.fext, extension) == 0){
                                file_dir = curr_file_dir;
                                file_directory_index = j;
                                found = 1;
                                break;
                            }
                        } else{ //File we're looking for doesn't have an extension
                            if(strcmp(curr_file_dir.fext, "") == 0){ //Found the file!
                                file_dir = curr_file_dir;
                                file_directory_index = j;
                                found = 1;
                                break;
                            }
                        }
                    }
                }

                if(found){
                    printf("file name: %s, file extension: %s, start block: %d\n", file_dir.fname, file_dir.fext, file_dir.nStartBlock);
                    printf("offset: %d, file size: %d, buffer size: %d, size variable: %d", offset, file_dir.fsize, sizeof(buf)/sizeof(char*), size);
                    printf(", strlen(buf): %d\n", strlen(buf));
                    if(offset > file_dir.fsize){ //Offset is greater than the file size, so don't write
                        return -EFBIG;
                    }
                    int buffer_size = size; //Buffer size is the length of the buf
                    int skip_block = offset/BLOCK_SIZE;    //determine how many block to skip
                    int block_offset = offset - skip_block*BLOCK_SIZE; //determine the offset of the start block
                    int curr_block = file_dir.nStartBlock;
                    int prev_block = -1;
                    while(skip_block-- > 0){
                        prev_block = curr_block;
                        curr_block = fat.table[curr_block];
                    }
                    if(curr_block==-1){     // if we skipped all blocks, allocate a new block
                        for(j=START_ALLOC_BLOCK;j<MAX_FAT_ENTRIES;j++){
                            if(fat.table[j] == 0){
                                curr_block = j;
                                fat.table[j] = -1;
                                fat.table[prev_block] = curr_block;
                                break;
                            }
                        }
                    }

                    /*write the file */
                    int buffer_bytes_remaining = buffer_size;
                    FILE* disk = fopen(".disk", "r+b");
                    fseek(disk, BLOCK_SIZE*curr_block+block_offset, SEEK_SET);
                    if(buffer_bytes_remaining > BLOCK_SIZE-block_offset){  //if buffer is too big to fit in this block
                        fwrite(buf,BLOCK_SIZE-block_offset,1,disk);
                        buffer_bytes_remaining -= (BLOCK_SIZE-block_offset);
                    }else{
                        printf("buf: %p, buf[0]: %c, buffer_size: %d, size: %d\n", buf, buf[0], buffer_size, size);
                        fwrite(buf, buffer_size, 1, disk);
                        char null_array[BLOCK_SIZE-buffer_size];
                        int m = 0;
                        for(m = 0; m < BLOCK_SIZE-buffer_size; m++){
                            null_array[m] = '\0';
                        }
                        fwrite(null_array, BLOCK_SIZE-buffer_size, 1, disk);
                        buffer_bytes_remaining -= buffer_size;
                    }
                    while(buffer_bytes_remaining > 0){
                        /* find next block */
                        if(fat.table[curr_block] == -1){   //if we need a new block
                            int free_block_found = 0;
                            int k;
                            for(k=START_ALLOC_BLOCK;k<MAX_FAT_ENTRIES;k++){
                                if (fat.table[k] == 0){
                                    fat.table[curr_block] = k;
                                    fat.table[k] = -1;
                                    prev_block = curr_block;
                                    curr_block = k;
                                    free_block_found = 1;
                                    break;
                                }
                            }
                            if(!free_block_found)   return -EPERM; //no more blocks available
                        }else{
                            prev_block = curr_block;
                            curr_block = fat.table[curr_block];
                        }
                        /* keep writing */
                        fseek(disk, BLOCK_SIZE*curr_block, SEEK_SET);
                        if(buffer_bytes_remaining > BLOCK_SIZE){
                            char* new_buf_address = buf + (strlen(buf) - buffer_bytes_remaining);
                            fwrite(new_buf_address, BLOCK_SIZE, 1, disk);
                            buffer_bytes_remaining -= BLOCK_SIZE;
                        }else{
                            char* new_buf_address = buf + (strlen(buf) - buffer_bytes_remaining);
                            fwrite(new_buf_address, buffer_bytes_remaining, 1, disk);
                            buffer_bytes_remaining = 0;
                        }
                    }
                    int appended_bytes = offset+size-file_dir.fsize>0?offset+size-file_dir.fsize:0;
                    file_dir.fsize += appended_bytes;
                    dir_entry.files[file_directory_index] = file_dir;
                    fseek(disk, current_directory.nStartBlock*BLOCK_SIZE, SEEK_SET);
                    fwrite(&dir_entry, BLOCK_SIZE, 1, disk);
                    fclose(disk);
                    write_root(&root);
                    write_fat(&fat);
                    return size;
                }
                else    return -EPERM; //if file not found, return not exist
            }
        }
    }
    return -EPERM;  //if directory not found, return not exist
}

/******************************************************************************
 *
 *  DO NOT MODIFY ANYTHING BELOW THIS LINE
 *
 *****************************************************************************/

/*
 * truncate is called when a new file is created (with a 0 size) or when an
 * existing file is made shorter. We're not handling deleting files or 
 * truncating existing ones, so all we need to do here is to initialize
 * the appropriate directory entry.
 *
 */
static int cs1550_truncate(const char *path, off_t size)
{
	(void) path;
	(void) size;

    return 0;
}


/* 
 * Called when we open a file
 *
 */
static int cs1550_open(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;
    /*
        //if we can't find the desired file, return an error
        return -ENOENT;
    */

    //It's not really necessary for this project to anything in open

    /* We're not going to worry about permissions for this project, but 
	   if we were and we don't have them to the file we should return an error

        return -EACCES;
    */

    return 0; //success!
}

/*
 * Called when close is called on a file descriptor, but because it might
 * have been dup'ed, this isn't a guarantee we won't ever need the file 
 * again. For us, return success simply to avoid the unimplemented error
 * in the debug log.
 */
static int cs1550_flush (const char *path , struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;

	return 0; //success!
}


//register our new functions as the implementations of the syscalls
static struct fuse_operations hello_oper = {
    .getattr	= cs1550_getattr,
    .readdir	= cs1550_readdir,
    .mkdir	= cs1550_mkdir,
	.rmdir = cs1550_rmdir,
    .read	= cs1550_read,
    .write	= cs1550_write,
	.mknod	= cs1550_mknod,
	.unlink = cs1550_unlink,
	.truncate = cs1550_truncate,
	.flush = cs1550_flush,
	.open	= cs1550_open,
};

//Don't change this.
int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &hello_oper, NULL);
}
