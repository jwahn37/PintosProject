#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

//define func pointer
//typedef tid_t (*PROC_EXE)(const char *file_name);
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

void parse_filename (char **argv_list, char **file_name, char *tp_ptr); //jinu 2017-10-10
bool construct_ESP(void **esp, char *argv_list); //jinu 2017-10-10
//jinu4 move for page_fault to use
bool install_page (void *upage, void *kpage, bool writable);


#endif /* userprog/process.h */
