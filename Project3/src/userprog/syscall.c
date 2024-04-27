#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/exception.h"
#include "userprog/process.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/directory.h"


static void syscall_handler (struct intr_frame *);
void syscall_halt(void);
void syscall_exit(int status);
tid_t syscall_exec(const char *cmd_line);
int syscall_wait(tid_t pid);
int syscall_read(int fd, void *buffer, unsigned size);
int syscall_write(int fd, void *buffer, unsigned size);

bool syscall_create(const char* file_name, unsigned initial_size);
bool syscall_remove(const char* file_name);
int syscall_open(const char* file_name);
void syscall_close(int fd);
int syscall_filesize(int fd);
void syscall_seek(int fd, unsigned position);
unsigned syscall_tell(int fd);


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
            f->eax = syscall_read(*(int*)(stack_ptr+4), (void*)*(uint32_t*)(stack_ptr+8), (unsigned)*(uint32_t*)(stack_ptr+12));
            break;
        }
        case SYS_WRITE: {
            for(int i = 1; i < 4; i++)
                if(!is_user_vaddr(stack_ptr+i*4))
                    syscall_exit(-1);
            f->eax = syscall_write(*(int*)(stack_ptr+4), (void*)*(uint32_t*)(stack_ptr+8), (unsigned)*(uint32_t*)(stack_ptr+12));
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
        case SYS_CREATE: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            if(!is_user_vaddr(stack_ptr+8))
                syscall_exit(-1);
            f->eax = syscall_create(*(char**)(stack_ptr+4), (unsigned)*(uint32_t*)(stack_ptr+8));
            break;
        }
        case SYS_REMOVE: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            f->eax = syscall_remove(*(char**)(stack_ptr+4));
            break;
        }
        case SYS_OPEN: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            f->eax = syscall_open(*(char**)(stack_ptr+4));
            break;
        }
        case SYS_CLOSE: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            syscall_close((int)*(uint32_t*)(stack_ptr+4));
            break;
        }
        case SYS_FILESIZE: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            f->eax = syscall_filesize((int)*(uint32_t*)(stack_ptr+4));
            break;
        }
        case SYS_SEEK: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            if(!is_user_vaddr(stack_ptr+8))
                syscall_exit(-1);
            syscall_seek((int)*(uint32_t*)(stack_ptr+4), (unsigned)*(uint32_t*)(stack_ptr+8));
            break;
        }
        case SYS_TELL: {
            if(!is_user_vaddr(stack_ptr+4))
                syscall_exit(-1);
            f->eax = syscall_tell((int)*(uint32_t*)(stack_ptr+4));
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

    for(int i = 3; i < 256; i++)
        if(thread_current()->fd_table[i] != NULL)
            syscall_close(i);

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

int syscall_read(int fd, void *buffer, unsigned size)
{
    if(!is_user_vaddr(buffer))
        syscall_exit(-1);

    if(fd < 0 || fd > 256)
        syscall_exit(-1);
    
    lock_acquire(&lock_file);

    int file_size = -1;

    if(fd == 0)
    {
        for(int i = 0; i < (int)size; i++)
        {
            *(char*)(buffer+i) = input_getc();
            if(*(char*)(buffer+i) == '\0')
            {
                file_size = i;
                break;
            }
        }
    }
    else if(fd >= 3)
    {
        struct file *current_file = thread_current()->fd_table[fd];

        if(current_file != NULL)
            file_size = file_read(current_file, buffer, size);
    }

    lock_release(&lock_file);

    return file_size;
}
int syscall_write(int fd, void *buffer, unsigned size)
{
    if(fd < 0 || fd > 256)
        syscall_exit(-1);

    lock_acquire(&lock_file);
    int file_size = -1;

    if(fd == 1)
    {
        putbuf(buffer, size);
        file_size = size;
    }
    else if(fd >= 3)
    {
        struct file *current_file = thread_current()->fd_table[fd];

        if(current_file != NULL)
        {
            if(current_file->deny_write)
            {
                file_deny_write(current_file);
                file_size = file_write(current_file, buffer, size);
                file_allow_write(current_file);
            }
            else
            {
                file_size = file_write(current_file, buffer, size);
            }
        }
    }
    lock_release(&lock_file);

    return file_size;
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

bool syscall_create(const char *file_name, unsigned initial_size)
{
    if(file_name == NULL)
        syscall_exit(-1);
    return filesys_create(file_name, initial_size);
}

bool syscall_remove(const char *file_name)
{
    if(file_name == NULL)
        syscall_exit(-1);
    return filesys_remove(file_name);
}

int syscall_open(const char *file_name)
{
    if(file_name == NULL)
        syscall_exit(-1);
    
    lock_acquire(&lock_file);
    struct file *current_file = filesys_open(file_name);
    if(current_file == NULL)
    {
        lock_release(&lock_file);
        return -1;
    }

    struct thread *t = thread_current();

    for(int i = 3; i < 256; i++)
    {
        if(t->fd_table[i] == NULL)
        {
            if(!strcmp(t->name, file_name))
            {
                file_deny_write(current_file);
            }
            t->fd_table[i] = current_file;
            lock_release(&lock_file);
            return i;
        }
    }
    return -1;
}

void syscall_close(int fd)
{
    struct thread *t = thread_current();
    
    if(t->fd_table[fd] == NULL)
        syscall_exit(-1);
    if(fd <= 2)
        syscall_exit(-1);

    file_close(t->fd_table[fd]);
    t->fd_table[fd] = NULL;
}

int syscall_filesize(int fd)
{
    if((thread_current()->fd_table[fd]) == NULL)
        syscall_exit(-1);

    return file_length(thread_current()->fd_table[fd]);
}

void syscall_seek(int fd, unsigned position)
{    
    if((thread_current()->fd_table[fd]) == NULL)
        syscall_exit(-1);

    file_seek(thread_current()->fd_table[fd], position);
}

unsigned syscall_tell(int fd)
{
    if((thread_current()->fd_table[fd]) == NULL)
        syscall_exit(-1);
    
    return file_tell(thread_current()->fd_table[fd]);
}
