/*
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  There are a couple of symbols that need to be #defined before
  #including all the headers.
*/

#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 26
#define FUSE_USE_VERSION 26

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

// maintain bbfs state in here
#include <limits.h>
#include <stdio.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

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
	unsigned long ino_filesize_block; //file size in blocks
	superblock *ino_sb; //superblock associated

} inode;

typedef struct ino_bitmap{
	int bitmap[1024];
} ino_bitmap;

typedef struct data_bitmap{
	int bitmap[1024];
} data_bitmap;

typedef struct ino_table{
	int ino_table[1024];
}ino_table;

//first level of pointers
typedef struct indirect_pointer{
	superblock * index_pointer;
	struct indrect_pointer * next;
} indirect_pointer;

//simple filesystem state
typedef struct sfs_state {
    FILE *logfile;
    char *diskfile;

    pid_t pid;
    char * pid_path;
} sfs_state;

#define SFS_DATA ((struct sfs_state *) fuse_get_context()->private_data)

#endif
