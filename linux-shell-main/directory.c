// Directory manipulation functions.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "bitmap.h"
#include "blocks.h"
#include "inode.h"
#include "slist.h"
#include "directory.h"
#include <string.h>

typedef struct direct {
  char name[DIR_NAME_LENGTH];
  int inum;
  char _reserved[12];
} direct_t;

void directory_init() {
  void *bim = get_inode_bitmap();
  bitmap_put(bim, 0, 1); //claim inode 0 for root directory
	
  inode_t *rn = get_inode(0);
  rn->refs = 0;
  rn->mode = 040777;
  rn->size = 0;
  rn->block = alloc_block(); // will take block 3 for root dir
  alloc_block();//claim block 4 too
  direct_t *home = (direct_t *) blocks_get_block(rn->block);
  home->inum = 0;
  home->name[0] = *"/";
}

//lookup file in home directory from path, return the inode number
int home_lookup(const char *path) {//return inode number for file path
  const char * token = (const char*) strtok((char*) path, "/");
  
  if (strcmp(path, "/") == 0) {
    return 0;
  }

  direct_t * dir = (direct_t *) blocks_get_block(3);
  for (int i = 0; i < 128; ++i) {
    if (strcmp(dir->name, path) == 0) {
      return dir->inum;
    }
    dir = (dir + 1);
  }
  
  return -1;//didn't find it
}

// add the given name/inode pair to the directory
int directory_put(inode_t *dd, const char *name, int inum) {
  //inum is new entry's inode, name is the name of the file/dir we're adding
  
  direct_t *dir = (direct_t *) blocks_get_block(3);
  
  for (int i = 0; i < 128; ++i) {
	
    if (dir->name[0] == '\0') {//if the name field is uninitialized
      memmove(&dir->name, name, strlen(name));
      dir->inum = inum;
      return 1;
    }
    dir = (dir + 1);
  }

  return -1; //fail
}

int directory_delete(const char *name){

direct_t *dir = (direct_t *) blocks_get_block(3);

  for (int i = 0; i < 128; ++i) {
    if (strcmp(name, dir->name) == 0) {//match names
      memmove(&dir->name, &"\0", strlen(name));
      dir->inum = -1; //reset everything
      return 1;
    }
    dir = (dir + 1);
  }
  return -1; //fail
}

//rename the directory with the inodeNum to have the name name
int rename_dir(int targetInodeNum, const char *name) {
  if (targetInodeNum <= 0) {
    return -1;
  }
    direct_t * dir = (direct_t *) blocks_get_block(3);
    for (int i = 0; i < 128; ++i) {
      if (targetInodeNum == dir->inum) {
        memmove(&dir->name, name, strlen(name));
        return 1;//renamed ok
      }
      dir = (dir + 1);
    }

  return -1;//failed to rename
}

//list files in home dir
slist_t *listHome() {

  direct_t *entry = (direct_t *) blocks_get_block(3);//home dir block 3, 4
  slist_t *toRet = NULL;
  for (int i = 0; i < 128; ++i) {

    if (entry->name[0] == '\0') {
      //do nothing, skip the entry because its not set up
      entry = (entry + 1);
    } else {
      toRet = s_cons(entry->name , toRet);
      entry = (entry + 1);
    }
  }
  return toRet;
}

direct_t *get_dir(slist_t *list) {

  inode_t *i = get_inode(0);
    int block = i->block;
    direct_t *dir = (direct_t *) blocks_get_block(block);

    for (int i = 0; i < 64; ++i) {
      if (0 == strcmp(list->data, dir->name)) {
        if (list->next == NULL) {
          return dir;
        } else {
          return get_dir(list->next);
        }
      }
      dir = (dir + 1);
    }
    return (direct_t *) -1;
}


#endif
