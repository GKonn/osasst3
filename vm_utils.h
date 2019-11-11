#ifndef VM_UTILS
#define VM_UTILS

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scv.h"

typedef struct _tlb_t tlb_t;
#define LOCK(m) pthread_mutex_lock(find_mutex(m))
#define UNLOCK(m) pthread_mutex_unlock(find_mutex(m))

int min(int a, int b);
int max(int a, int b);

inline void init_mutex(pthread_mutex_t **mutex);
inline void destroy_mutex(pthread_mutex_t **mutex);

inline pthread_mutex_t *find_mutex(void *data_struct);

void insert_in_tlb(
        tlb_t *tlb,
        void *va,
        void *pa);

tlb_t *check_tlb(
        tlb_t *tlb,
        void *va,
        bool invalidate);

void map_addresses(
        scv_t *phys_pages,
        unsigned long *pgdir,
        void *init_va);
void set_base_ptr(
        scv_t *base_ptr,
        int pages_needed);
void destroy_inner_pagetable();

void get_next_avail(
        scv_t *bitmap,
        scv_t *ret,
        int pages,
        int offset);
void resize_bitmap(
        scv_t *bitmap,
        int pages);
void set_used_pages(
        scv_t *bitmap,
        scv_t *pages,
        int virt_page,
        int virt_pages_needed,
        bool free);

int find_consec_pages(
        scv_t *bitmap,
        int pages_needed);

scv_t *get_phys_pages(int pages_needed);
void *get_virt_addr(scv_t *phys_pages);

#endif
