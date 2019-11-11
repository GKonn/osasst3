#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scv.h"
#include "vm_utils.h"

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need
#define PGSIZE 4096
#define MAX_MEMSIZE 4ULL*1024*1024*1024
#define MEMSIZE 1024*1024*1024
#define TLB_SIZE 64
#define PAGENUM 32768

typedef unsigned long pte_t;
typedef unsigned long pde_t;
typedef struct tlb {
    void *pa;
    pte_t addr;
    bool valid;
} tlb;
typedef struct data {
	scv_t *physicalmap;
	scv_t *virtualmap;
       	scv_t *physicalmemory;
    	pde_t *pagedir;
    tlb *tlba;    
} data;
extern data mdata;
extern struct tlb tlb_store;
void set_physical_mem();
pte_t* translate(pde_t *pgdir, void *va);
int PageMap(pde_t *pgdir, void *va, void* pa);
void *a_malloc(unsigned int num_bytes);
void a_free(void *va, int size);
void put_value(void *va, void *val, int size);
void get_value(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);
#endif
