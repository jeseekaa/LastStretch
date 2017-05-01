/*
  Simple File System

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.

*/

#include "params.h"
#include "block.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"

//Additional Structs for this project:

// superblock
typedef struct superblock{

  struct superblock * next; //to traverse through directories
  int total_ino; 
  int total_data_entries; //total number of data entries
  
  unsigned long sb_blocksize; //in bytes
  
  int ino_index; //index of inode in list
  int length; 
  int offset; //offset from root
  char file_name[500]; //limit filename to 500 characters 

} superblock;

// our i-node struct is based on the actual thing is found in <linux/fs.h>
typedef struct inode{

  char * data; // the data the inode holds
  char type; //the type of data
  unsigned int ino_id; //i-node number
  uid_t ino_uid; //user_id of owner process
  gid_t ino_gid; //group_id of owner process
  int ino_size; //filesize in bloks
  unsigned long ino_filesize_block; //file size in blocks
  superblock *ino_sb; //superblock associated

} inode;

/*typedef struct ino_bitmap{
  int bitmap[1024];
} ino_bitmap;

typedef struct data_bitmap{
  int bitmap[1024];
} data_bitmap;

typedef struct ino_table{
  int ino_table[1024];
}ino_table;*/

//first level of pointers
/*typedef struct indirect_pointer{

  superblock * index_pointer;
  struct indrect_pointer * next;

} indirect_pointer;*/


///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
//

inode * ino_list;
superblock * sb_root;


/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *sfs_init(struct fuse_conn_info *conn)
{
    fprintf(stderr, "in bb-init\n");
    log_msg("\nsfs_init()\n");

    
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    /*sfs_state * state = SFS_DATA;
    state->pid = getpid();

    disk_open((SFS_DATA)->diskfile); //from block.h
    //I CANT EVEN OPEN A DISK WITHOUT IT BREAKING WHY

    //initializing the inode struct
    ino_list = malloc(500 * sizeof(struct inode));
    ino_list[0].data = NULL;

    int uid = getuid();
    ino_list[0].ino_uid=uid;

    int gid = getgid();
    ino_list[0].ino_gid = gid;

    //initializing superblock
    sb_root = (superblock *)malloc(sizeof(struct superblock));
    sb_root->next = NULL;
    sb_root->offset =0;
    sb_root->ino_index =0;
    strcpy(sb_root->file_name, "/root");

    const char* path = state->pid_path;*/



    log_msg("sfs_init() done\n");



    return SFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void sfs_destroy(void *userdata)
{
    log_msg("\nsfs_destroy(userdata=0x%08x)\n", userdata);
    disk_close();

    free(ino_list);
    free(sb_root);

    log_msg("destroy complete\n");

}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int sfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    

    log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);

    //sfs_fullpath(fpath, path);

    statbuf->st_uid = getuid(); //the user ID of the file's owner
    statbuf->st_gid = getgid(); //the group ID of the file

    statbuf->st_atime = time(NULL); //last access time
    statbuf->st_mtime = time(NULL); //last modification time

    if(strcmp(path, "/")==0){
      statbuf->st_mode = S_IFDIR | 0755; // set the mode of the file, directory
      statbuf->st_nlink =2; //set the hardlink of the file

    }else{
      statbuf->st_mode = S_IFREG | 0644; // regular file
      statbuf->st_nlink =1;
      statbuf->st_size = 1024;
    }
   // retstat = lstat(fpath, statbuf);
    log_msg("attributes aquired\n");
  
    
    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int sfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
	    path, mode, fi);

    int fd; //file descriptor

    fd=creat(path,mode);
    fi->fh = fd;
    
    log_msg("sfs_create done\n");
    
    return retstat;
}

/** Remove a file */
int sfs_unlink(const char *path)
{
    int retstat = 0;
    log_msg("sfs_unlink(path=\"%s\")\n", path);

    retstat = unlink(path);
    if (retstat<0){
      retstat = -errno;
    }

    log_msg("sfs_unlink done\n");
    
    return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int sfs_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_open(path\"%s\", fi=0x%08x)\n",
	    path, fi);

    int fd; //file descriptor

    fd = open (path, fi->flags);
    if(fd == -1){
      return -errno;
    }

    
    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int sfs_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_release(path=\"%s\", fi=0x%08x)\n",
	  path, fi);

    retstat= close(fi->fh);

    if(retstat<0){
      retstat = -errno;
    }

    log_msg("released\n");

    
    return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
int sfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);
/*
    //read the bytes
    int current_block = sb_root->ino_index;
    int n_bytes_read = 0;
    int block_offset = offset % BLOCK_SIZE;
    int total = 0;

    //read bytes
    while(n_bytes_read < size){

    //get the starting point of the file
      int start_index = BLOCK_SIZE * (1+current_block) + block_offset;

    //get the number of bytes to read
      int n_to_read = 0;
      if((size - n_bytes_read) > (BLOCK_SIZE - block_offset)){
        
        n_to_read = BLOCK_SIZE - block_offset;

      }else{
        n_to_read = total - n_bytes_read;
      }

    //update the pointer to buffer
      buf += n_to_read;

    //reset offset
      block_offset = 0;

    //update the block?

    //update the total number of bytes 
      n_bytes_read += n_to_read;

    } //end of while loop

    log_msg("sfs_read done\n");
    printf("sfs_read done\n");

    retstat = n_bytes_read;*/

    if(fi->fh){
      retstat=pread(fi->fh, buf, size-retstat, offset+retstat);
      if(retstat<0){
        retstat= -errno;
      }else{
        while(retstat<size){
          int curr = pread(fi->fh, buf, size-retstat, offset+retstat);
          if(curr<= 0){
            if(curr<0){
              retstat = -errno;
            }
            break;
          }
          retstat += curr;
        }
      }
    }


   
    return retstat;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
int sfs_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
	    path, buf, size, offset, fi);

    /*
    //setup
    int n_traverses = (offset / BLOCK_SIZE);
    int current_block = sb_root -> ino_index;
    
    int old_block = -1;

    while(n_traverses > 0){
      old_block = current_block;
      n_traverses--;
    }

    //write total number of bytes
    int total_bytes_written =0;
    int block_offset = offset % BLOCK_SIZE;

    while(total_bytes_written < size){

    //get the starting point in file
      int start_index = BLOCK_SIZE * (1 + current_block) + block_offset;
    //get the numner of bytes to write
      int n_to_write = 0;
       if (size - total_bytes_written > BLOCK_SIZE - block_offset) {
           n_to_write = BLOCK_SIZE - block_offset;
       } else {
           n_to_write = size - total_bytes_written;
         }

    //update offset
         block_offset =0;

    //update blokc
         old_block =current_block;

    //update total bytes written
         total_bytes_written += n_to_write;

    } //end of while loop

    printf("sfs_write done\n");
    retstat = total_bytes_written;*/

    if(fi->fh){
      retstat = pwrite(fi->fh, buf, size, offset);
      if(retstat<0){
        retstat = -errno;
      }
    }else{
      while(retstat<size){
      int curr = pwrite(fi->fh, buf, size, offset+retstat);
      if(curr <= 0){
        if (curr<0){
          retstat = -errno;
        }
        break;
      }
      retstat +=curr;
    }
  }

    
    return retstat;
}


/** Create a directory */
int sfs_mkdir(const char *path, mode_t mode)
{
    int retstat = 0;
    log_msg("\nsfs_mkdir(path=\"%s\", mode=0%3o)\n",
	    path, mode);
   
    
    return retstat;
}


/** Remove a directory */
int sfs_rmdir(const char *path)
{
    int retstat = 0;
    log_msg("sfs_rmdir(path=\"%s\")\n",
	    path);
    
    
    return retstat;
}


/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int sfs_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    log_msg("\nsfs_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
    
    
    return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int sfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("readdir%s\n", path);

    //log_msg("Getting the list of files of %s \n", path);

    //filler(buf, ".", NULL, 0);
   // filler(buf, "..", NULL, 0);

    DIR *dir;
    struct dirent *entry;

    dir = (DIR *) fi->fh;
    errno =0;

    if(strcmp(path, "/")==0){
      log_msg("nothing in this directory\n");
      //call create?
      return retstat;
    }

    while(entry = readdir(dir)){

      struct stat stat;

      memset(&stat, 0, sizeof(stat));

      stat.st_ino = entry -> d_ino;
      stat.st_mode = entry -> d_type << 12;

      if(filler(buf, entry->d_name, &stat, 0)){
        log_msg("buffer full\n");
        retstat = -ENOMEM;

      }
    } //end of while loop

    if(entry == NULL && errno != 0){
      retstat = -errno;
    }
    
    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int sfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    
    return retstat;
}

struct fuse_operations sfs_oper = {
  .init = sfs_init,
  .destroy = sfs_destroy,

  .getattr = sfs_getattr,
  .create = sfs_create,
  .unlink = sfs_unlink,
  .open = sfs_open,
  .release = sfs_release,
  .read = sfs_read,
  .write = sfs_write,

  .rmdir = sfs_rmdir,
  .mkdir = sfs_mkdir,

  .opendir = sfs_opendir,
  .readdir = sfs_readdir,
  .releasedir = sfs_releasedir
};

void sfs_usage()
{
    fprintf(stderr, "usage:  sfs [FUSE and mount options] diskFile mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct sfs_state *sfs_data;
    
    // sanity checking on the command line
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	sfs_usage();

    sfs_data = malloc(sizeof(struct sfs_state));
    if (sfs_data == NULL) {
	perror("main calloc");
	abort();
    }

    // Pull the diskfile and save it in internal data
    sfs_data->diskfile = argv[argc-2];
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    sfs_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main, %s \n", sfs_data->diskfile);
    fuse_stat = fuse_main(argc, argv, &sfs_oper, sfs_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}
