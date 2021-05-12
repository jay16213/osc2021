#include "printf.h"
#include "sys_call.h"

int main(){
    char buf[256];
    int a = open("hello", O_CREAT);
    int b = open("world", O_CREAT);
    write(a, "Hello ", 6);
    write(b, "World!", 6);
    close(a);
    close(b);
    b = open("hello", 0);
    a = open("world", 0);
    int sz;
    sz = read(b, buf, 100);
    sz += read(a, buf + sz, 100);
    buf[sz] = '\0';
    printf("%s\r\n", buf); // should be Hello World!
}
