#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "my_vm.h"
#include "vm_utils.h"
#include "scv.h"

void wipe_pagetable() {
    int num_elements = PGSIZE / sizeof(pde_t), i;

    for(i = 0; i < num_elements; i++) {
        if(metadata.outer_pagetable[i]) {
            free((void *) metadata.outer_pagetable[i]);
        }
    }

    memset(metadata.outer_pagetable, 0, PGSIZE);
}

void test_translate() {
    pde_t *page_table = (pde_t *) calloc(
            PGSIZE / sizeof(pde_t),
            sizeof(pde_t));
    void *va = (void *) 0xffffffff;

	int numOffsetBits = (int) log2(PGSIZE),
        numSecondBits = min((32 - numOffsetBits) / 2, numOffsetBits);

    numSecondBits -= (int) log2(sizeof(pde_t));
	
	pde_t mask2 = (1 << numSecondBits) - 1,
          offsetMask = (1 << numOffsetBits) - 1;
    
    pde_t virtualAddr = (pte_t) va,
          firstBits = (virtualAddr >> (numSecondBits + numOffsetBits)) & mask2,
          secondBits = (virtualAddr >> numOffsetBits) & mask2,
          offset = virtualAddr & offsetMask;

	pde_t *result = translate(page_table, va);
    assert(!result);

    pte_t *second_page = (pte_t *) calloc(
            PGSIZE / sizeof(pte_t),
            sizeof(pte_t));
    page_table[secondBits] = (pte_t) second_page;

    result = translate(page_table, va);
    assert(!result);

    void *val = (void *) 0x1000;
    ((pte_t *) page_table[firstBits])[secondBits] = (pte_t) val;

    result = translate(page_table, va);
    assert(result == (pde_t *) ((pte_t) val + offset));

    printf("Passed unittests for translate\n");
}

void test_page_map() {
	pde_t *page_table = (pde_t*) calloc(
            PGSIZE / sizeof(pde_t),
            sizeof(pde_t));
	void* va = (void*) 0xffffffff;
	
	int numOffsetBits = (int) log2(PGSIZE),
	    numSecondBits = min(
                (32 - numOffsetBits) / 2,
                numOffsetBits);

    numSecondBits -= (int) log2(sizeof(pde_t));
	long mask = (1 << numSecondBits) - 1;

	pde_t virtualAddr = (pte_t) va,
          firstBits = virtualAddr >> (numSecondBits + numOffsetBits) & mask,
          secondBits = (virtualAddr >> numOffsetBits) & mask;

    void *pa = (void *) 0x100;

	page_map(page_table, va, pa);

	pde_t *firstAddr = (pde_t*)page_table[firstBits];
	pte_t secondAddr = firstAddr[secondBits];

	assert((void *) secondAddr == pa);

    printf("Passed unittests for page_map\n");
}

void test_get_next_avail() {
    scv_t *bitmap = scv_new(
            sizeof(char),
            1);

    scv_t *ret = scv_new(
              sizeof(int),
              1);

    char a = 0xff;
    scv_push_back(bitmap, &a);
    
    a = 0x9f;
    scv_push_back(bitmap, &a);

    a = 0xf0;
    scv_push_back(bitmap, &a);

    get_next_avail(bitmap, ret, 4, 0);

    assert(scv_size(ret) == 4);

    assert(*((int *) scv_at(ret, 0)) == 13);
    assert(*((int *) scv_at(ret, 1)) == 14);
    assert(*((int *) scv_at(ret, 2)) == 16);
    assert(*((int *) scv_at(ret, 3)) == 17);

    printf("Passed unittests for get_next_avail\n");

    scv_delete(bitmap);
    scv_delete(ret);
}

void test_find_consec_pages() {
    scv_t *bitmap = scv_new(
            sizeof(char),
            1);

    char a = 0xff;
    scv_push_back(bitmap, &a);

    assert(find_consec_pages(bitmap, 1) == -1);

    char *ptr = (char *) scv_back(bitmap);
    *ptr = 0x33;

    assert(find_consec_pages(bitmap, 2) == 2);
    scv_delete(bitmap);

    printf("Passed unittests for find_consec_pages\n");
}

void test_resize_bitmap() {
    scv_t *bitmap = scv_new(
            sizeof(char),
            1);

    char a = 0x5;
    scv_push_back(bitmap, &a);

    resize_bitmap(bitmap, 20);
    assert(scv_size(bitmap) == 4);

    printf("Passed unittests for resize_bitmap\n");
    scv_delete(bitmap);
}

void test_set_used_pages() {
    scv_t *bitmap = scv_new(sizeof(char), 4),
          *pages = scv_new(sizeof(int), 4);

	// 4 cases
		// malloc virtual bitmap
		// free virtual bitmap
		// malloc phys bitmap
		// free phys bitmap
		
    char a = 0x3f;
    scv_push_back(bitmap, &a);

    a = 0x1f;
    scv_push_back(bitmap, &a);

    a = 0xf;
    scv_push_back(bitmap, &a);

    a = 0;
    scv_push_back(bitmap, &a);

    int b = 6;
    scv_push_back(pages, &b);

    b = 15;
    scv_push_back(pages, &b);

    b = 23;
    scv_push_back(pages, &b);

    b = 30;
    scv_push_back(pages, &b);

	// malloc phys bitmap
    set_used_pages(
            bitmap,
            pages,
			0,
			0,
			false);

	assert(*((char *) scv_at(bitmap, 0)) == (char) 0x7f);
    assert(*((char *) scv_at(bitmap, 1)) == (char) 0x9f);
    assert(*((char *) scv_at(bitmap, 2)) == (char) 0x8f);
    assert(*((char *) scv_at(bitmap, 3)) == (char) 0x40);

	// malloc virtual bitmap 
	// change 0th byte of bitmap
	*((char *) scv_at(bitmap, 0)) = 0xf;
	
	set_used_pages(
			bitmap,
			NULL,
			4,
			4,
			false);
	
	assert(*((char *) scv_at(bitmap, 0)) == (char) 0xff);

	// free phys bitmap
    // need different page indexes (ones that are NOT free)

	*((char*)scv_at(bitmap, 0)) = 0x7f; // reset 0th byte in bitmap
	*((char*)scv_at(bitmap, 3)) = 0x10;
	*((int*)scv_at(pages, 0)) = 5;
	*((int*)scv_at(pages, 1)) = 12;
	*((int*)scv_at(pages, 2)) = 17;	
	*((int*)scv_at(pages, 3)) = 28;
	
	set_used_pages(
			bitmap,
			pages,
			0,
			0,
			true);

	assert(*((char *) scv_at(bitmap, 0)) == (char) 0x5f);
	assert(*((char *) scv_at(bitmap, 1)) == (char) 0x8f);
	assert(*((char *) scv_at(bitmap, 2)) == (char) 0x8d);
	assert(*((char *) scv_at(bitmap, 3)) == (char) 0x0);
	
	// free virtual bitmap (?????????????????????????????????)
	set_used_pages(
			bitmap,
			NULL,
			0,
			5,
			true);

	assert(*((char *) scv_at(bitmap, 0)) == (char) 0x40);
    printf("Passed unittests for set_used_pages\n");
    scv_delete(bitmap);
    scv_delete(pages);
}

void test_map_addresses() {
    scv_t *phys_pages = scv_new(
            sizeof(int),
            1);

    int a = 2;
    scv_push_back(
            phys_pages,
            &a);

    a = 5;
    scv_push_back(
            phys_pages,
            &a);
    
    a = 6;
    scv_push_back(
            phys_pages,
            &a);

    pde_t *pgdir = (pde_t *) calloc(
            PGSIZE / sizeof(pde_t),
            sizeof(pde_t));

    int num_offset_bits = (int) ceil(log2(PGSIZE));

    map_addresses(
            phys_pages,
            pgdir,
            NULL);

    assert(translate(pgdir, NULL) == scv_at(metadata.base_ptr, 2));
    assert(translate(pgdir, (void *) (1 << num_offset_bits)) == scv_at(metadata.base_ptr, 5));
    assert(translate(pgdir, (void *) (2 << num_offset_bits)) == scv_at(metadata.base_ptr, 6));

    printf("Passed unittests for map_addresses\n");
    scv_delete(phys_pages);
    free(pgdir);
}

void test_get_phys_pages() {
    scv_t *phys_pages = get_phys_pages(3);

    assert(scv_size(phys_pages) == 3);

    assert(*((int *) scv_at(phys_pages, 0)) == 0);
    assert(*((int *) scv_at(phys_pages, 1)) == 1);
    assert(*((int *) scv_at(phys_pages, 2)) == 2);

    assert(*((char *) scv_front(metadata.phys_bitmap)) == (char) 0x7);
    
    scv_t *other_pages = get_phys_pages(6);

    assert(scv_size(other_pages) == 6);

    assert(*((int *) scv_at(other_pages, 0)) == 3);
    assert(*((int *) scv_at(other_pages, 1)) == 4);
    assert(*((int *) scv_at(other_pages, 2)) == 5);

    assert(*((int *) scv_at(other_pages, 3)) == 6);
    assert(*((int *) scv_at(other_pages, 4)) == 7);
    assert(*((int *) scv_at(other_pages, 5)) == 8);

    assert(*((char *) scv_at(metadata.phys_bitmap, 0)) == (char) 0xff);
    assert(*((char *) scv_at(metadata.phys_bitmap, 1)) == (char) 0x1);

    printf("Passed unittests for get_phys_pages\n");

    scv_delete(phys_pages);
    scv_delete(other_pages);
    scv_zero(metadata.phys_bitmap); // zero out metadata so you can run unittests on functions that use it again
}

void test_get_virt_addr() {
    scv_t *phys_pages = get_phys_pages(4);
    char *va = (char *) get_virt_addr(phys_pages);

    assert(!va);
    assert(translate(metadata.outer_pagetable, va) == 
            scv_at(metadata.base_ptr, *((int *) scv_at(phys_pages, 0))));

    assert(translate(metadata.outer_pagetable, va + PGSIZE) == 
                scv_at(metadata.base_ptr, *((int *) scv_at(phys_pages, 1))));
    assert(translate(metadata.outer_pagetable, va + 2 * PGSIZE) == 
                scv_at(metadata.base_ptr, *((int *) scv_at(phys_pages, 2))));
    assert(translate(metadata.outer_pagetable, va + 3 * PGSIZE) == 
                scv_at(metadata.base_ptr, *((int *) scv_at(phys_pages, 3))));

    assert(*((char *) scv_at(metadata.virt_bitmap, 0)) == (char) 0xf);

    printf("Passed unittests for get_virt_addr\n");

    scv_delete(phys_pages);
    scv_zero(metadata.phys_bitmap);
    scv_zero(metadata.virt_bitmap);

    wipe_pagetable();
}

void test_a_malloc() {
    void *a = a_malloc(200);

    assert((void *) translate(metadata.outer_pagetable, a) == scv_front(metadata.base_ptr));
    assert(*((char *) scv_at(metadata.phys_bitmap, 0)) == (char) 0x1); 
    assert(*((char *) scv_at(metadata.virt_bitmap, 0)) == (char) 0x1); 

    void *b = a_malloc(PGSIZE + 2);

    assert((void *) translate(metadata.outer_pagetable, b) == scv_at(metadata.base_ptr, 1));
    assert((void *) translate(metadata.outer_pagetable, b + PGSIZE) == scv_at(metadata.base_ptr, 2));

    assert(*((char *) scv_at(metadata.phys_bitmap, 0)) == (char) 0x7);
    assert(*((char *) scv_at(metadata.virt_bitmap, 0)) == (char) 0x7);

    printf("Passed unittests for a_malloc\n");

    scv_zero(metadata.phys_bitmap);
    scv_zero(metadata.virt_bitmap);

    wipe_pagetable();
}

void test_a_free() {
    void *first = a_malloc(200),
         *second = a_malloc(PGSIZE + 1),
         *third = a_malloc(PGSIZE + 4);

    a_free(first, 200);

    assert(!translate(metadata.outer_pagetable, first));
    assert(*((char *) scv_front(metadata.phys_bitmap)) == (char) 0x1e);
    assert(*((char *) scv_front(metadata.virt_bitmap)) == (char) 0x1e);

    void *fourth = a_malloc(100);
    assert(translate(metadata.outer_pagetable, fourth) == scv_front(metadata.base_ptr));
    assert(*((char *) scv_front(metadata.phys_bitmap)) == (char) 0x1f);
    assert(*((char *) scv_front(metadata.virt_bitmap)) == (char) 0x1f);

    a_free(second, PGSIZE);

    assert(translate(metadata.outer_pagetable, second + PGSIZE) ==
            scv_at(metadata.base_ptr, 2));
    assert(*((char *) scv_front(metadata.phys_bitmap)) == (char) 0x1d);
    assert(*((char *) scv_front(metadata.virt_bitmap)) == (char) 0x1d);

    void *fifth = a_malloc(PGSIZE * 5);

    assert(*((char *) scv_front(metadata.phys_bitmap)) == (char) 0xff);
    assert(*((char *) scv_at(metadata.phys_bitmap, 1)) == (char) 0x1);

    assert(*((char *) scv_front(metadata.virt_bitmap)) == (char) 0xfd);
    assert(*((char *) scv_at(metadata.virt_bitmap, 1)) == (char) 0x3);

    assert(translate(metadata.outer_pagetable, fifth) == scv_at(metadata.base_ptr, 1));
    assert(translate(metadata.outer_pagetable, fifth + PGSIZE) == scv_at(metadata.base_ptr, 5));
    assert(translate(metadata.outer_pagetable, fifth + PGSIZE * 2) == scv_at(metadata.base_ptr, 6));
    assert(translate(metadata.outer_pagetable, fifth + PGSIZE * 3) == scv_at(metadata.base_ptr, 7));
    assert(translate(metadata.outer_pagetable, fifth + PGSIZE * 4) == scv_at(metadata.base_ptr, 8));

    printf("Passed unittests for a_free\n"); 

    scv_zero(metadata.phys_bitmap);
    scv_zero(metadata.virt_bitmap);

    wipe_pagetable();
}

int main(int argc, char **argv) {
    test_translate();
    test_page_map();
    test_get_next_avail();

    test_find_consec_pages();
    test_resize_bitmap();
    test_set_used_pages();

    test_map_addresses();

    test_get_phys_pages();
    test_get_virt_addr();

    test_a_malloc();
    test_a_free();

    printf("Passed all unittests\n");
    return 0;
}
