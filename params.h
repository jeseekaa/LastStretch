/*
  Jessica Guo (jg900) 
  Shrey Desai (sjd166)
  Douglas Judice (dij9)

  Assignment03 due May 1, 2017

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


//simple filesystem state
typedef struct sfs_state {
    FILE *logfile;
    char *diskfile;

    pid_t pid;
    char * pid_path;
} sfs_state;

#define SFS_DATA ((struct sfs_state *) fuse_get_context()->private_data)

#endif
