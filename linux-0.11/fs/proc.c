#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <sys/types.h>
#include <asm/segment.h>
#include <stdarg.h>
#define ORIG_ROOT_DEV (*(unsigned short *)0x901FC)
#define set_bit(nr,addr) ({\
register int res ; \
__asm__ __volatile__("btsl %2,%3\n\tsetb %%al": \
"=a" (res):"0" (0),"r" (nr),"m" (*(addr))); \
res;})

static char proc_buf[PAGE_SIZE] = {'\0'};
int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}

int proc_read(int dev, char * buf, int count, off_t *pos)
{
    int i;
    if (*pos % PAGE_SIZE == 0) {
        if (dev == 0)
            psinfo();
        else if (dev == 1)
            hdinfo();
    }
    //put_fs_byte();
    for (i = 0; i < count; ++i) {
        if (proc_buf[i + *pos] == '\0') break;
        put_fs_byte(proc_buf[i+*pos], buf+i);
    }
    *pos += i;
    return i;
}


int psinfo()
{
    struct task_struct **p;
    int buf_pos = sprintf(proc_buf, "pid\tstate\tfather\tcounter\tstart_time\n"); 

    for (p = &LAST_TASK; p >= &FIRST_TASK; p--)
        if (*p != NULL) {
            buf_pos += sprintf(proc_buf + buf_pos, "%d\t%d\t%d\t%d\t%d\n", (*p)->pid, (*p)->state, (*p)->father, (*p)->counter, (*p)->start_time); 
        }

    return buf_pos;
}

int hdinfo()
{
 //   unsigned short root_dev = ORIG_ROOT_DEV;
    int buf_pos = 0, used = 0, i;
    struct super_block *sb = get_super(ORIG_ROOT_DEV);
    buf_pos += sprintf(proc_buf, "Total blocks:%d\n", sb->s_nzones);
    i = sb->s_nzones + 1; 
    while (--i >= 0) {
        if (set_bit(i&8191, sb->s_zmap[i>>13]->b_data))
            used++;
    }
    buf_pos += sprintf(proc_buf+buf_pos, "Used blocks:%d\n", used);
    buf_pos += sprintf(proc_buf+buf_pos, "Free blocks:%d\n", sb->s_nzones - used);
    buf_pos += sprintf(proc_buf+buf_pos, "Total inodes:%d\n", sb->s_ninodes);
    used = 0;
    i = sb->s_ninodes + 1;
    while (--i >= 0) {
        if (set_bit(i&8191, sb->s_imap[i>>13]->b_data))
            used++;
    }
    buf_pos += sprintf(proc_buf+buf_pos, "Used inodes:%d\n", used);
    buf_pos += sprintf(proc_buf+buf_pos, "Free inodes:%d\n", sb->s_ninodes - used);
    return buf_pos;    
}
