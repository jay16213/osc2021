extern int get_pid(void);
extern void exec(char *filename, char **argv);
// extern void exec(char *filename, char **argv);
extern int fork(void);
extern void exit(void);
extern int uart_read(char buf[], int size);
extern int uart_write(const char buf[], int size);
extern int sigreturn(unsigned long __unused);
extern int kill(int pid, int signal);
extern int useless(void);
extern int open(const char *pathname, int flags);
extern int close(int fd);
extern int write(int fd, const void *buf, int count);
extern int read(int fd, void *buf, int count);
extern int stat_and_next(int fd, struct dentry *dent);
extern int mkdir(const char *path, int mode);
extern int chdir(const char *path);
extern int mount(const char* device,
	const char* mountpoint, const char* filesystem);
extern int umount(const char* mountpoint);
extern int sync();
extern int mknod(const char *pathname, int devnum);
extern void *sys_mmap(void* addr, int len, int prot,
    int flags, int fd, int file_offset);