#include "bootloader.h"
#include "uart.h"
#include "utils.h"

#define CPIO_ARRD 0x8000000
#define CPIO_MAGIC_BYTES 6
#define CPIO_OTHERS_BYTES 8
#define CPIO_SIZE (CPIO_MAGIC_BYTES + CPIO_OTHERS_BYTES * 13)

struct cpio_newc_header {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_check[8];
};

int cpio_info(struct cpio_newc_header **cpio_ptr, char **cpio_addr,
              char **context) {
  *cpio_ptr = (struct cpio_newc_header *)(*cpio_addr);

  /* get filename size */
  unsigned long long int c_namesize =
      atoHex_size((*cpio_ptr)->c_namesize, CPIO_OTHERS_BYTES);

  /* get cpio header size */
  unsigned long long int header_size =
      CPIO_SIZE + c_namesize + align(CPIO_SIZE + c_namesize, 4);

  /* get cpio context size */
  unsigned long long int context_size =
      atoHex_size((*cpio_ptr)->c_filesize, CPIO_OTHERS_BYTES);
  unsigned long long int size = context_size + align(context_size, 4);
  *context = (*cpio_addr) + header_size;

  /* move to next cpio header */
  *cpio_addr += header_size + size;
  return context_size;
}
