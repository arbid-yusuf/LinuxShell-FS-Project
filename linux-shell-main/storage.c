// Disk storage manipulation.
//
// Feel free to use as inspiration.

// based on cs3650 starter code


#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "slist.h"
#include "blocks.h"
#include "storage.h"
#include "bitmap.h"
#include "inode.h"
#include "directory.h"

void storage_init(const char *path) {
	blocks_init("data.nufs");
	void *bbm = get_blocks_bitmap();
	int firstSetup = bitmap_get(bbm, 3);
	if (firstSetup == 0) {
          directory_init();
	}
}
int storage_stat(const char *path, struct stat *st) { //get info for path file and fill out stat, return 1 or error
  int inum = home_lookup(path);
  assert(inum != -1);
  inode_t *i = get_inode(inum);
  st->st_mode = i->mode;
  st->st_size = i->size;
  st->st_uid = getuid();  
  return 1;
}

int storage_read(const char *path, char *buf, size_t size, off_t offset) {
  //read from the file into the buffer for size bytes, starting at file index 0 + offset
  int inode = home_lookup(path);
  int block = get_inode(inode)->block;
  assert(block != -1);
  void *mem = blocks_get_block(block);
  memmove(buf, mem + offset, size);
  return size;
}

int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
	//similar to read but write from buffer into file at offset->size + offset
  int inode = home_lookup(path);
  
  int block = get_inode(inode)->block;
  assert(block != -1);
  void *mem = blocks_get_block(block);
  memmove(mem + offset, buf, size);
  
  return size;
}

int storage_truncate(const char *path, off_t size) {
	//set the size of the file to size
  int inode = home_lookup(path);
  assert(inode != -1);
  inode_t *i = get_inode(inode);
  i->size = size;
  return 0;
}
	
	
int storage_mknod(const char *path, int mode) {
	//make in inode storage
  int inode = home_lookup(path);
  assert(inode == -1);
  
  inode_t *i = get_inode(alloc_inode());
  i->mode = mode;  
}

