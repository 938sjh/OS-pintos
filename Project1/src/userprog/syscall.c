#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/exception.h"
#include "userprog/process.h"
#include "devices/shutdown.h"


static void syscall_handler (struct intr_frame *);
void syscall_halt(void);
void syscall_exit(int status);
tid_t syscall_exec(const char *cmd_line);
int syscall_wait(tid_t pid);
void syscall_read(int fd, void *buffer, unsigned size);
void syscall_write(int fd, void *buffer, unsigned size);

void
syscall_init (void) 
{
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
    void *stack_ptr = f->esp;
    uint32_t syscall_number = *(uint32_t *) stack_ptr;

    switch (syscall_number){
        case SYS_HALT: {
            syscall_halt();
            break;
        }
        case SYS_EXIT: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            syscall_exit(*(int*)(stack_ptr+4));
            break;
        }
        case SYS_EXEC: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            f->eax = syscall_exec(*(char**)(stack_ptr+4));
            break;
        }
        case SYS_WAIT: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            f->eax = syscall_wait((tid_t*)*(uint32_t*)(stack_ptr+4));
            break;
        }
        case SYS_READ: {
            for(int i = 1; i < 4; i++)
                if(!is_user_vaddr(stack_ptr+i*4))
                    syscall_exit(-1);
            syscall_read(*(int*)(stack_ptr+4), (void*)*(uint32_t*)(stack_ptr+8), (unsigned)*(uint32_t*)(stack_ptr+12));
            break;
        }
        case SYS_WRITE: {
            for(int i = 1; i < 4; i++)
                if(!is_user_vaddr(stack_ptr+i*4))
                    syscall_exit(-1);
            syscall_write(*(int*)(stack_ptr+4), (void*)*(uint32_t*)(stack_ptr+8), (unsigned)*(uint32_t*)(stack_ptr+12));
            break;
        }
        case SYS_FIBONACCI: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            f->eax = fibonacci(*(int*)(stack_ptr+4));
            break;
        }
        case SYS_MAX_OF_FOUR_INT: {
            for(int i = 1; i < 5; i++)
                if(!is_user_vaddr(stack_ptr+i*4))
                    syscall_exit(-1);
            int a, b, c, d;
            a = *(int*)(stack_ptr+4);
            b = *(int*)(stack_ptr+8);
            c = *(int*)(stack_ptr+12);
            d = *(int*)(stack_ptr+16);
            f->eax = max_of_four_int(a, b, c, d);
            break;
        }
        default:
            break;
    }
}

void syscall_halt(void)
{
    shutdown_power_off();
}

void syscall_exit(int status)
{
    thread_current()->exit_status = status;
    printf("%s: exit(%d)\n", thread_name(), status);
    thread_exit();
}

tid_t syscall_exec(const char *cmd_line)
{
    return process_execute(cmd_line);
}

int syscall_wait(tid_t pid)
{
    return process_wait(pid);
}

void syscall_read(int fd, void *buffer, unsigned size)
{
    if(fd != 0)
        return;

    for(int i = 0; i < size; i++)
        *(char*)(buffer+i) = input_getc();
}

void syscall_write(int fd, void *buffer, unsigned size)
{
    if(fd != 1)
        return;

    putbuf(buffer, size);
}

int fibonacci(int n)
{
    int x = 0, y = 1, z = 1;
    
    for(int i = 0; i < n; i++)
    {
        z = x + y;
        x = y;
        y = z;
    }

    return x;
}

int max_of_four_int(int a, int b, int c, int d)
{
    int max_num = a;

    if(max_num < b)
        max_num = b;
    if(max_num < c)
        max_num = c;
    if(max_num < d)
        max_num = d;

    return max_num;
}
