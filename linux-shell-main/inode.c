// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "blocks.h"
#include "bitmap.h"

typedef struct inode {
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int block; // single block pointer (if max file size <= 4K)
} inode_t;

#define INODE_SIZE sizeof(inode_t)

//for debug, not used
void print_inode(inode_t *node) {
  printf("inode: mode %d: refs: %d , size: %d, block: %d \n", node->mode, node->refs, node->size, node->block);  
}

//get the inode at the given inum
inode_t *get_inode(int inum) {
  return (inode_t *) (blocks_get_block(1) + INODE_SIZE * inum);
}

//allocate the next available inode
int alloc_inode() {
  void *bim = get_inode_bitmap();

  for(int i = 0; i < BLOCK_SIZE - BLOCK_COUNT; ++i) {
    if (!bitmap_get(bim, i)) {
      bitmap_put(bim, i, 1);
      printf("+ alloc_inode -> %d\n", i);
      return i;
    }
  }
  return -1;
}

//free the inode
void free_inode(int inum) {
  printf("+ free inode(%d)\n", inum);
  void *bim = get_inode_bitmap();
  bitmap_put(bim, inum, 0);
}

#endif
