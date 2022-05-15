#include <linux/kernel.h>
//#include <linux/sched.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <asm/segment.h>
#define MAX_LEN 23
static char my_name[MAX_LEN + 1] = "Yarn";
static int len = 4;
int sys_iam(const char * name)
{
   unsigned int i = 0;
   unsigned int namelen = 0;
   while (get_fs_byte(name + namelen) != '\0')
     ++namelen;
   if (namelen > MAX_LEN) {
    errno = EINVAL;
    return -1;
   } 
   while (i < namelen) {
    my_name[i] = get_fs_byte(name+i);
    ++i;
   }
   my_name[i] = '\0';
   len = i;
   printk("my_name: %s\n", my_name);
//   printk("sizeof(*current) = %d\n", sizeof(*current));
   return namelen;
}


int sys_whoami(char* name, unsigned int size)
{
    unsigned int i = 0;
    if (len > size - 1) {
        errno = EINVAL;
        return -1;
    }
    while (i < len) {
        put_fs_byte(my_name[i], name + i);
        ++i;
    }
    put_fs_byte(0, name + i);
    return len;
}
