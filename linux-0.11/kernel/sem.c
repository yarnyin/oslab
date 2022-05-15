#define __LIBRARY__
#include <unistd.h>
#include <asm/segment.h>
#include <string.h>
#include <asm/system.h>
#define NR_SEM 32
//typedef struct {
//    int value;
//    int occupied;
//    char name[16];
//    struct task_struct *wait_queue;
//} sem_t;*/

sem_t semaphores[NR_SEM];

sem_t *sys_sem_open(const char *name, int value);
int sys_sem_wait(sem_t *sem);
int sys_sem_post(sem_t *sem);
int sys_sem_unlink(const char *name);


sem_t *sys_sem_open(const char *name, int value)
{
    char c, buf[16];
    int cnt = 0, i;
    while (1) {/*get name from user*/
        c = get_fs_byte(name++);
        buf[cnt++] = c;
        if (c == '\0')
            break;
        if (cnt >= 16)
            return NULL;
    }
    /*open occupied*/
    for (i = 0; i < NR_SEM; ++i) {
        if (semaphores[i].occupied) {
            if (!strcmp(buf, semaphores[i].name)) {
                cli();
                semaphores[i].occupied += 1;
                sti();
                return &semaphores[i];
            }
        }
    }
    /*create new*/
    for (i = 0; i < NR_SEM; ++i) {
        if (!semaphores[i].occupied) {
            /*init*/
            semaphores[i].occupied = 1;
            semaphores[i].value = value;
            strcpy(semaphores[i].name, buf);
            semaphores[i].wait_queue = NULL;  

            return &semaphores[i];
        }
    }

    return NULL;
}

int sys_sem_wait(sem_t *sem)
{
    cli();
    while (sem->value == 0)
        sleep_on(&sem->wait_queue);
    sem->value -= 1;
    sti();
    return 0;
}

int sys_sem_post(sem_t *sem)
{
    cli();
    sem->value += 1;
    wake_up(&sem->wait_queue);
    sti();
    return 0;
}

int sys_sem_unlink(const char *name)
{
    char c, buf[16];
    int cnt = 0, i;
    while (1) {/*get name from user*/
        c = get_fs_byte(name++);
        buf[cnt++] = c;
        if (c == '\0')
            break;
        if (cnt >= 16)
            return NULL;
    }
    /*find sem by name*/
    for (i = 0; i < NR_SEM; ++i) {
        if (semaphores[i].occupied)
            if (!strcmp(buf, semaphores[i].name)) {
                cli();
                semaphores[i].occupied -= 1;
                sti();
                return 0;
            }
    }

    return -1; /*not found*/
}
