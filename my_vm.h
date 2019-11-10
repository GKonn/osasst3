#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need

#define PGSIZE 4096
#define MAX_MEMSIZE 4ULL * 1024 * 1024 * 1024
#define MEMSIZE 1024 * 1024 * 1024
#define TLB_SIZE 64
#define PAGENUM MEMSIZE / PGSIZE
#define BITMAPSIZE PAGENUM / 8 

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

//Structure to represents TLB
typedef struct tlb {
	void *pa;
	pte_t pagestart;
	bool valid;
}tlb;
typedef struct data_t{
	char* physicalmap;
	char* virtualmap;
	char* physicalmemory;
	pde_t *pagedir;
	tlb *tlbchart;
}data_t;

    //Assume your TLB is a direct mapped TLB of TBL_SIZE (entries)
    // You must also define wth TBL_SIZE in this file.
    //Assume each bucket to be 4 bytes

struct tlb tlb_store;


void SetPhysicalMem();
pte_t* Translate(pde_t *pgdir, void *va);
int PageMap(pde_t *pgdir, void *va, void* pa);
bool check_in_tlb(void *va);
void put_in_tlb(void *va, void *pa);
void *m_alloc(unsigned int num_bytes);
void a_free(void *va, int size);
void PutVal(void *va, void *val, int size);
void GetVal(void *va, void *val, int size);
void MatMult(void *mat1, void *mat2, int size, void *answer);

#endif
