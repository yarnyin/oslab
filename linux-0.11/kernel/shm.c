#include <linux/kernel.h>
#include <unistd.h>
#include <linux/sched.h>
#include <asm/system.h>
#define SHM_CNT 32

struct {
    int used;
    int nr_users;
    int key;
    unsigned long page;

} shm_table[SHM_CNT];


int sys_shmget(int key, size_t size, int shmflg);
void *sys_shmat(int shmid, const void *shmaddr, int shmflag);
int sys_shmdt(const void *);



int sys_shmget(int key, size_t size, int shmflg)
{
    int i;
    /*search used*/
    for (i = 0; i < SHM_CNT; ++i) {
        if (shm_table[i].used) {
            if (shm_table[i].key == key) {
                cli();
                shm_table[i].nr_users += 1;
                sti();
                return i;
            }
        }
    }
    /*create new*/
    for (i = 0; i < SHM_CNT; ++i) {
        if (!shm_table[i].used) {
            shm_table[i].used = 1;
            shm_table[i].nr_users = 0;
            shm_table[i].key = key;
            shm_table[i].page = get_free_page();
            printk("shmget: page = %ld\n", shm_table[i].page);
            return i;
        }
    }

    return -1;
}

void *sys_shmat(int shmid, const void *shmaddr, int shmflag)
{
    unsigned long data_base, brk;
    if (shmid < 0 || SHM_CNT < shmid)
        return NULL;
    if (!shm_table[shmid].used) {
        printk("SHM not used\n");
        return NULL;
    }
    shm_table[shmid].nr_users += 1;
    data_base = get_base(current->ldt[1]);
    brk = current->brk + data_base;
    current->brk += PAGE_SIZE;
    put_page(shm_table[shmid].page, brk);
    return (void *)(current->brk - PAGE_SIZE);
}



int sys_shmdt(const void *shmaddr)
{
    int i;
    unsigned long data_base, brk, page;
    data_base = get_base(current->ldt[1]);
    brk = shmaddr + data_base;
    page = get_page(brk);
    printk("shmdt: page = %ld\n", page);
    for (i = 0; i < SHM_CNT; ++i) {
        if (shm_table[i].used) {
            if (shm_table[i].page == page) {
                shm_table[i].nr_users -= 1;
                invalidate_page(brk);
                if (!shm_table[i].nr_users) {
                    shm_table[i].used = 0;
                    free_page(page);
                }
                return 0;
            }
        }
    }

/*
    if (!shm_table[shmid].used) {
        printk("SHM not used\n");
        return -1;
    }
    shm_table[shmid].nr_users -= 1;
   //should invalidate the page in current process(undue) 

    if (!shm_table[shmid].nr_users) {
        shm_table[shmid].used = 0;
        free_page(shm_table[shmid].page);
    }
*/
    return -1;
}
