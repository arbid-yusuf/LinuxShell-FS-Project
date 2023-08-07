// based on cs3650 starter code

#include <assert.h>
#include <bsd/string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "bitmap.h"
#include "blocks.h"
#include "storage.h"
#include "directory.h"

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  int rv = -ENOENT;
  int inum = home_lookup(path);
  if (inum != -1) {
    rv = 0;
  }
  
  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = 0;
		           
  int inum = home_lookup(path);//find the file's inode
  if (inum == -1) {
    rv = -ENOENT;
    return rv;
  } 
  
  inode_t *i = get_inode(inum);//get the info
  st->st_mode = i->mode;
  st->st_size = i->size;
  st->st_uid = getuid();
  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  slist_t *home = listHome();	
  	
  struct stat st;
  int rv;
  
  rv = nufs_getattr("/", &st);
  assert(rv == 0);
  filler(buf, ".", &st, 0);
  
  while (home != NULL) {
    const char *f = home->data;
    rv = nufs_getattr(f, &st);
    assert(rv == 0);
    
    char * token = strtok(f, "/");
    filler(buf, token, &st, 0);
    
    home = home->next; 
  } 
  s_free(home);
  
  printf("readdir(%s) -> %d\n", path, rv);
  return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = 0;
  int inode = alloc_inode();
  int d = directory_put(NULL, path, inode);
  
  if (d != 1) {
    return -ENOENT;
  }

  inode_t *i = get_inode(inode);
  i->mode = mode;
  i->block = alloc_block();

  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

//unlink the path
int nufs_unlink(const char *path) {
  int rv = 0;

  int inum = home_lookup(path);
  int d = directory_delete(path);
  if (d == -1) {
    return -ENOENT;
  }
  inode_t *i = get_inode(inum);
  int block = i->block;
  i->block = -1;
  free_inode(inum);
  free_block(block);

  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_link(const char *from, const char *to) {
  int rv = -1;
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_rmdir(const char *path) {
  int rv = -1;
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  int rv = 0;
  int inum = home_lookup(from);
  rename_dir(inum, to);
  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

//set file size to size
int nufs_truncate(const char *path, off_t size) {
  int rv = -1;
  int inum = home_lookup(path);
  if (inum != -1) {
    rv = 0;
  }
  inode_t *i = get_inode(inum);
  i->size = size;
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = 0;
  if (nufs_access(path, 0) != 0) {
	  rv = -1;
    printf("open(%s) -> %d\n", path, rv);
    return rv;
  }
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = -ENOENT;
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  int r = storage_read(path, buf, size, offset);
  
  return r;
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {

  int r = storage_write(path, buf, size, offset);
  int inode = home_lookup(path);
  inode_t *i = get_inode(inode);
  i->size = r;
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, r);
  return r;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int rv = 0;
  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return rv;//not supported, but changed the rv to avoid annoying error messages
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = -1;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  printf("TODO: mount %s as data file\n", argv[--argc]);
  storage_init(argv[argc]);//set up FS structure
  //Our fs is designed to be operated from the mnt directory...
  //When testing, please cd into mnt and run commands from there. 
  //It seems like some auto tests run commands that we don't handle because of this 
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
