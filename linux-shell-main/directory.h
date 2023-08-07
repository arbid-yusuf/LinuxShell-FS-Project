// Directory manipulation functions.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"

typedef struct direct {
  char name[DIR_NAME_LENGTH];
  int inum;
  char _reserved[12];
} direct_t;

void directory_init();
int directory_put(inode_t *dd, const char *name, int inum);
int directory_delete(const char *name);
int home_lookup(const char *path);
slist_t *listHome();
int rename_dir(int targetInodeNum, const char *name);
direct_t *get_dir(slist_t *list);
#endif
