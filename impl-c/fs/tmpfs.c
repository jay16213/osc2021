#include "fs/tmpfs.h"
#include "fs/vfs.h"

#include "cpio.h"
#include "mm.h"
#include "stdint.h"
#include "string.h"
#include "uart.h"
#include <stddef.h>

#include "config.h"
#include "log.h"

#ifdef CFG_LOG_TMPFS
static const int _DO_LOG = 1;
#else
static const int _DO_LOG = 0;
#endif

// TODO: use linked-list to replace staticaly allocate space
#define TMPFS_DIR_CAPACITY 10

#define TMPFS_MX_FILE_NAME_LEN 256

#define TMPFS_NODE_TYPE_DIR 1
#define TMPFS_NODE_TYPE_FILE 2

#define RING_BFR_SIZE 256

// Private content for vnode in tmpfs
typedef struct {
  char *name;
  uint8_t node_type;
  size_t size;

  // Differernt node could access differnt attribute
  union {
    // Dir node
    struct {
      struct vnode **children;
    };
    // File node
    struct {};
  };
} Content;

// lazy init
int tmpfs_initialized = false;
struct vnode_operations *tmpfs_v_ops = NULL;
struct file_operations *tmpfs_f_ops = NULL;

struct filesystem tmpfs = {
    .name = "tmpfs", .setup_mount = tmpfs_setup_mount, .next = NULL};

#define content_ptr(vnode) (Content *)((vnode)->internal)

static void build_root_tree(struct vnode *root_dir);

const char *node_name(struct vnode *dir) {
  Content *cnt = content_ptr(dir);
  return cnt->name;
}

/**
 * @brief Find the parent vnode in the tmpfs given a file's fullpath.
 *  This function is only used for building rootfs. A success function call
 *  means user need to build a vnode(with name $ret_name) under the returned
 *  parent vnode
 * @param fullpath fullpath of the file in CPIO filename. e.g., "./folder/a.txt"
 * @param ret_name buffer to write the component name.  e.g., "a.txt"
 * @retval parent vnode of the target handle
 */
static struct vnode *build_root_find_parent_dir(const char *fullpath,
                                                struct vnode *root_dir,
                                                char *ret_name);
static void tmpfs_dumpdir(struct vnode *dir_node, int cur_level);

// Create & Initialize a empty vnode for TMPFS
static struct vnode *create_vnode(const char *name, uint8_t node_type);

// Convert CPIO mode to TMPFS node_type
static int parse_node_type(int64_t cpio_mode) {
  if (s_ISFILE(cpio_mode)) {
    return TMPFS_NODE_TYPE_FILE;
  } else if (s_ISDIR(cpio_mode)) {
    return TMPFS_NODE_TYPE_DIR;
  } else {
    return -1;
  }
}

void wait() {
  for (uint64_t i = 0; i < (1 << 23); i++) {
    ;
  }
}

void tmpfs_dev() {
  struct mount my_root;
  tmpfs_setup_mount(&tmpfs, &my_root);
  uart_println("dev finish");
  uart_println("root tree:");
  tmpfs_dumpdir(my_root.root, 0);
  uart_println("done...");
  while (1) {
    ;
  }
}

// bind operations
int tmpfs_init() {
  tmpfs_v_ops = kalloc(sizeof(struct vnode_operations));
  tmpfs_v_ops->lookup = tmpfs_lookup;
  tmpfs_v_ops->create = tmpfs_create;

  tmpfs_f_ops = kalloc(sizeof(struct file_operations));
  tmpfs_f_ops->read = tmpfs_read;
  tmpfs_f_ops->write = tmpfs_write;
  return 0;
}

static struct vnode *create_vnode(const char *name, uint8_t node_type) {
  struct vnode *node = (struct vnode *)kalloc(sizeof(struct vnode));

  // This node is not mounted by other directory
  node->mnt = NULL;
  node->v_ops = tmpfs_v_ops;
  node->f_ops = tmpfs_f_ops;

  Content *cnt = kalloc(sizeof(Content));
  {
    cnt->name = (char *)kalloc(sizeof(char) * strlen(name));
    strcpy(cnt->name, name);
    cnt->node_type = node_type;
    cnt->size = 0;
    if (node_type == TMPFS_NODE_TYPE_DIR) {
      cnt->children = kalloc(sizeof(struct vnode *) * TMPFS_DIR_CAPACITY);
    }
  }
  node->internal = cnt;
  return node;
}

static struct vnode *build_root_find_parent_dir(const char *fullpath,
                                                struct vnode *root_dir,
                                                char *ret_name) {

  const char *cur_path;
  struct vnode *parent_dir, *child_node;
  int start_idx, end_idx, name_size;
  char *file_name = NULL;

  cur_path = fullpath;
  parent_dir = root_dir;
  // Most find path should return in this while loop
  while (0 == get_component(cur_path, &start_idx, &end_idx)) {
    name_size = end_idx - start_idx + 1;
    if (name_size >= TMPFS_MX_FILE_NAME_LEN) {
      uart_println("Err: component name too large");
      while (1) {
        ;
      }
    }
    file_name = kalloc(sizeof(char) * (name_size + 1));
    memcpy(file_name, &cur_path[start_idx], name_size);
    file_name[name_size] = '\0';

    if (tmpfs_lookup(parent_dir, &child_node, file_name) == 0 &&
        child_node != NULL) {
      parent_dir = child_node;
      cur_path = &cur_path[end_idx + 1];
      kfree(file_name);
    } else {
      // this is the component we want to build for
      strcpy(ret_name, file_name);
      kfree(file_name);
      return parent_dir;
    }
  }

  // The only reason we're not return yet is...
  // the fullpath is equal to '.'(we don't want to build this node)
  // So here we return a NULL to ask caller not building this node
  ret_name[0] = '\0';
  return NULL;
}

/**
 * Because we build our CPIO by using `find .` command, this rebuild process
 * is just exploit some feauture of the `find` command's output
 **/
void build_root_tree(struct vnode *root_dir) {
  CpioNewcHeader *next;
  const char *path;
  void *fileContent;
  int err;
  uint64_t mode, filesize;
  struct vnode *child_node;

  Content *node_content;
  CpioNewcHeader *dir_header;

  dir_header = (CpioNewcHeader *)RAMFS_ADDR;

  char component_name[TMPFS_MX_FILE_NAME_LEN];
  int node_type;

  struct vnode *parent_dir;
  Content *parent_content;
  log_println("[tmpfs] start building rootfs from CPIO archive");

  for (;; dir_header = next) {
    if ((err = cpioParseHeader(dir_header, &path, &filesize, &mode,
                               &fileContent, &next)) != 0) {
      break;
    }
    // Find the parent vnode of the target file to create
    parent_dir = build_root_find_parent_dir(path, root_dir, component_name);

    // Do not build for child without parent
    // example: the `.` under '/'
    if (parent_dir == NULL) {
      log_println("[tmpfs] cpio: skip creation for %s", path);
      continue;
    }

    if (-1 == (node_type = parse_node_type(mode))) {
      continue;
    }
    child_node = create_vnode(component_name, node_type);
    node_content = content_ptr(child_node);
    node_content->size = filesize;
    parent_content = content_ptr(parent_dir);
    parent_content->children[parent_content->size++] = child_node;
    log_println("[tmpfs] %s: create `%s` under `%s`", path,
                node_name(child_node), node_name(parent_dir));
  }
};

int tmpfs_setup_mount(struct filesystem *fs, struct mount *mount) {
  log_println("[tmpfs] setup mount");
  if (tmpfs_initialized == false) {
    tmpfs_init();
    tmpfs_initialized = true;
  }

  struct vnode *root = create_vnode("/", TMPFS_NODE_TYPE_DIR);
  build_root_tree(root);

  // Officially mount this root vnode onto the mount point
  mount->root = root;
  return 0;
}

int tmpfs_write(struct file *f, const void *buf, unsigned long len) {
  // TODO
  return 0;
}

int tmpfs_read(struct file *f, void *buf, unsigned long len) {
  // TODO
  return 0;
}

int tmpfs_lookup(struct vnode *dir_node, struct vnode **target,
                 const char *component_name) {
  Content *dir_content = content_ptr(dir_node);

#ifdef CFG_LOG_TMPFS_LOOKUP
  uart_println("[tmpfs] lookup: `%s` under folder: `%s`", component_name,
               dir_content->name);
#endif

  // "." means the current directory
  if (strcmp(component_name, ".") == 0) {
    *target = dir_node;
    return 0;
  }

  Content *child_content;
  struct vnode *child_vnode;
  for (int i = 0; i < dir_content->size; i++) {
    child_vnode = dir_content->children[i];
    child_content = content_ptr(child_vnode);
    if (strcmp(child_content->name, component_name) == 0) {
      *target = child_vnode;
      return 0;
      ;
    }
  }
  *target = NULL;
  return -1;
}

void tmpfs_dumpdir(struct vnode *dir_node, int cur_level) {
  Content *dir_content = content_ptr(dir_node);
  // for (int i = 0; i < cur_level; i++) {
  //   uart_puts("  ");
  // }
  // uart_println("+ %s", dir_content->name);

  Content *child_content;
  struct vnode *child_vnode;
  for (int i = 0; i < dir_content->size; i++) {
    child_vnode = dir_content->children[i];
    child_content = content_ptr(child_vnode);
    for (int i = 0; i < cur_level; i++) {
      uart_puts("  ");
    }
    uart_printf(" +- %s", child_content->name);
    if (child_content->node_type == TMPFS_NODE_TYPE_FILE) {
      uart_println(" (file)");
    } else if (child_content->node_type == TMPFS_NODE_TYPE_DIR) {
      uart_println(" (dir)");
      tmpfs_dumpdir(child_vnode, cur_level + 1);
    }
  }
}
int tmpfs_create(struct vnode *dir_node, struct vnode **target,
                 const char *component_name) {
  // TODO
  return 0;
}
