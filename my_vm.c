#include <assert.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scv.h"
#include "vm_utils.h"
#include "my_vm.h"

//Globals
int flag = 0;
data mdata;


int min(int i, int j){
	return i < j ? i : j;
}
void SetPhysicalMem() {
	mdata.physicalmemory = scv_new(PGSIZE, PAGENUM);
    	setmem(mdata.physicalmemory,0);	
    	mdata.physicalmap = scv_new(sizeof(char), 1);
    	resize(mdata.physicalmap, PAGENUM);
    	mdata.virtualmap = scv_new(sizeof(char),1);
    	resize(mdata.virtualmap, PAGENUM);
//	mdata.pagedir = (pde_t *) calloc(PGSIZE / sizeof(pde_t), sizeof(pde_t));
	mdata.pagedir = malloc(1024 * PAGENUM);
}


pte_t * translate(pde_t *pgdir, void *va){  
	int os = 12;
        int secOS;
	if (((32 - os) / 2) <= os){
		secOS = ((32 - os) / 2);
	}
	else {
		secOS = os;
	}	
    	if(secOS== os) {
        	secOS-= (int) log2((double) sizeof(pde_t));
    	}
	long mask = (1 << secOS) - 1;
	long osMask = (1 << os) - 1; 
    	pde_t virt = (pte_t) va;
        pde_t first = (virt >> (secOS+ os)) & mask;
        pde_t second = (virt >> os) & mask;
        pde_t offset = virt & osMask;
    	pde_t *addr = (pde_t *) pgdir[first];
    	if(!addr){
   		return NULL;
    	}
	pte_t new = addr[second];
	if(!new){
        	return NULL;
   	}
    	pte_t final_return = new + offset;
	return (pte_t *) final_return;
}

int PageMap(pde_t *pgdir, void *va, void *pa){
    	int os = 12;
        int secOS;
	if (((32 - os) / 2) <= os){
       		secOS = ((32 - os) / 2);
	}
	else {
       		secOS = os;
       	}
    	if(secOS== os) {
        	secOS-= (int) log2(sizeof(pde_t));
    	}
	long mask = (1 << secOS) - 1;
    	pde_t virt = (pte_t) va;
        pde_t first = virt >> (secOS+ os) & mask;
        pde_t second = (virt >> os) & mask;
    	if(!pgdir[first]) {
		pte_t *new = (pte_t *) malloc(sizeof(pte_t));
        	pgdir[first] = (pte_t) new;
    	}
    	((pte_t *) pgdir[first])[second] = (pte_t) pa;
    	return 0;
}

void *m_alloc(unsigned int num_bytes) {
	if (flag == 0){
		SetPhysicalMem();
		flag = 1;
	}
    	int pages = (int) ceil(((double) num_bytes) / PGSIZE);
    	scv_t *physical = findpages(pages);
    	void *va = findvirtual(physical) + 1;
    	return va;
}

void a_free(void *va, int size) {
    	assert(size);
    	scv_t *phys_pages_freed = scv_new(sizeof(int), 1);
    	int os = 12;
        int osMask = (1 << os) - 1;
	int freed = 0;
        int offset = ((pde_t) va) & osMask;
    	size += offset;
    	char *page_va = (char *) ((pde_t) va & ~osMask);
    	int virtpage = (int) ((pde_t) page_va >> os);
    	for(; size > 0; size -= PGSIZE, freed++, page_va += PGSIZE) {
        	char *pa = (char *) translate(mdata.pagedir, (void *) page_va);
        	if(pa){
            		int page_num = (int) (pa - (char *) scv_front(mdata.physicalmemory)) / PGSIZE;
            		scv_push_back(phys_pages_freed, &page_num);
			PageMap(mdata.pagedir, page_va, NULL);
        	}
    	}
    	setpages(mdata.physicalmap,phys_pages_freed,0,0,true);
    	setpages(mdata.virtualmap, NULL, virtpage, freed, true);
}

void PutVal(void *va, void *val, int size) {
    	void *pa = (void *) translate(mdata.pagedir, va);
    	int mask = (1 << 12) - 1;
        int offset = ((pde_t) va) & mask;
        int copy;
	if (size <= PGSIZE - offset){
		copy = size;
	}
	else {
		copy = (PGSIZE - offset);
	}
    	while(size) {
        	memcpy(pa, val, copy);
        	size -= copy;
        	va += copy;
        	pa = translate(mdata.pagedir,va);
        	val += copy;
        	copy = min(size, PGSIZE);
		if (size <= PGSIZE){
			copy = size;
		}
		else {
			copy = PGSIZE;
		}
    	}		
}

void GetVal(void *va, void *val, int size) {
    // put the values pointed to by va inside the physical memory at given val address
    // assume you can access val address directly by derefencing them
    // always check first the presence of translation inside the tlb before proceeding forward

    void *pa = (void *) translate(
            mdata.pagedir,
            va);

    int mask = (1 << (int) ceil(log2(PGSIZE))) - 1,
        offset = ((pde_t) va) & mask,
        copy_bytes = min(size, PGSIZE - offset);

    while(size) {
        memcpy(val, pa, copy_bytes);

        size -= copy_bytes;
        va += copy_bytes;
        pa = translate(
                mdata.pagedir,
                va);

        val += copy_bytes;
        copy_bytes = min(size, PGSIZE);
    }
}

void MatMult(void *mat1, void *mat2, int size, void *answer) {
    // given two arrays of length: size * size
    // multiply them as matrices and store the computed result in answer
 
    // Hint: You will do indexing as [i * size + j] where i, j are the indices of matrix being accessed
    int i = 0;
    for(; i < size; i++) {
        int j, k;

        for(j = 0; j < size; j++) {
            int *ans = (int *) translate(
                    mdata.pagedir,
                    ((int *) answer) + i * size + j);
            *ans = 0;

            for(k = 0; k < size; k++) {
                int *mt1 = (int *) translate(
                        mdata.pagedir,
                        ((int *) mat1) + i * size + k),
                    *mt2 = (int *) translate(
                            mdata.pagedir,
                            ((int *) mat2) + k * size + j);
               
                assert(*mt1 == 1);
                assert(*mt2 == 1);
                *ans += *mt1 * *mt2;
            }
        }
    }
}
