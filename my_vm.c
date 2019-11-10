#include "my_vm.h"

/* Globals */
int flag = 0;
pde_t* pagedir;
char* physicalmap;
char* virtualmap;

void SetPhysicalMem() {	
	char *physicalmem = malloc(MEMSIZE); 
	physicalmap = malloc(BITMAPSIZE);
	virtualmap = malloc(BITMAPSIZE);
	//Zero out the bitmaps
	bzero(physicalmap, BITMAPSIZE);
	bzero(virtualmap, BITMAPSIZE);
	// Creates the page directory and zeroes it out
	pagedir = malloc(1024 * sizeof(pde_t));
	bzero(pagedir, 1024 * sizeof(pde_t));	
	
}

/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t * translate(pde_t *pgdir, void *va) {
    //HINT: Get the Page directory index (1st level) Then get the
    //2nd-level-page table index using the virtual address.  Using the page
    //directory index and page table index get the physical address
	int offset = 12;
	int secondoffset = 10;
	long mask = (1 << offset) - 1;
	long mask2 = (1 << secondoffset) - 1;
	pde_t virtual = (pde_t) va, 
	firstbits = (virtual >> (secondoffset + offset)) & mask,
	secondbits = (virtual >> offset) & mask,
	offset_t = virtual & mask2;
	pde_t *new = (pde_t *) pgdir[firstbits];
	if(!new){
		return NULL;
	}
	pte_t entry = new[secondbits];
	if(!entry){
		return NULL;
	}
	pte_t finaladdress = entry + offset_t;
	return (pte_t *) finaladdress;
	
/*	
	void *x = pagedir[offset >> va];//sets x to appropriate page table
	if(x==NULL){ return NULL;}
	return x[(12>>va)&0x3FF];	//returns address value at position in mem
*/
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int PageMap(pde_t *pgdir, void *va, void *pa) {
	int offset  = 12;
	int secondoffset = 10;
        long mask = (1 << secondoffset) - 1; 
     	pde_t virtualAddr = (pte_t) va,
        firstbits = virtualAddr >> (secondoffset + offset) & mask,
    	secondbits = (virtualAddr >> offset) & mask;
	if(!pgdir[firstbits]){
		pte_t *new = (pte_t *) calloc( PGSIZE / sizeof(pte_t), sizeof(pte_t));
	return -1;

/*
	if (Translate(pgdir,va) == NULL){
		int offset = 22;
		void *x = pagedir[offset >> va];//sets x to appropriate page table
		if(x==NULL){//creates pagetable
		pte_t *pagetable = malloc(sizeof(pte_t) * 1024);
		bzero(pagetable,1024*sizeof(pte_t));
		pagedir[offset>>va]=&pagetable;
		}
		x[(12>>va)&0x3FF]=&pa;//sets physical addr value to location in page table
   // HINT: Similar to Translate(), find the page directory (1st level)
   // and page table (2nd-level) indices. If no mapping exists, set the
   // virtual to physical mapping 

    return -1;
*/
}


/*
void *next_physical(){
	int temp, i = 0;
	for(i = 0; i < PAGENUM; i += PGSIZE){
		temp = physicalmap[i];
		if(getbitmap(physicalmap, i) == 0){
			return (void*) &physicalmem + i * PGSIZE;
		}
	}
}
*/
/*
scv_t *findpages(int pages, scv_t* freed){
         get_next_avail(freed, pages);
         setpages(freed);
         return freed;
 }
*/ 


void setpages(scv_t* pages){
	int index1 = 0;
	int index2 = 0;       
        for(index1 = 0; index1 < BITMAPSIZE; index1++) {
        	char *location = physicalmap[index1];
            	while(index2 < scv_size(pages)) {
                	int *val = (int *) scv_at(pages, index2);
                	if(*val / 8 != index1) {
                    		break;
                	}

        

                	if(free) {
                    		*location &= ~(1 << (*val % 8)); 
                	} 
			else {
                    		*location |= 1 << (*val % 8);
                	}
                	index2++;
            	}
            	if(index2 >= scv_size(pages)) {
                	break;
            	}
        }
}
void *get_next_avail(scv_t* freed, int num_pages){
	int pages = num_pages;
	int i = 0;
	int j = 0;
	for (i = 0; i < BITMAPSIZE; i++){
		char *location = virtualmap[i];
		for (j = 0; j < 8; j++){
			if(!((*location >> j) & 1)){
				int temp = i * 8 + j;
				scv_push_back(freed, &temp);	
				pages--;	
			}
			if(pages == 0){
				return;
			}
		}
	}
}
	/*
	int i, j, pages = 0;
	for (i = offset; i < BITMAPSIZE; i++){
		char *byte = bitmap[i];
		for(j = 0; j < 8; j++){
			if(!((*byte >> j) & 1)){
				int val = i * 8 + j;
				freep--;
				pages++;
			}
			if(pages == num_pages){
				return;
			}
		}
	}
*/
}

/*	int group = ((num_pages + 1)<< 1) - 1;
	for (i = 0; i < PAGE_NUM; i++){
		temp = virtualmap[i / 8];
		if(!(temp&group)){
		temp=4<<temp;
			for(j=num_pages;j<num_pages;j++){
				if(PageMap(pagedir ,virtualmap ,next_physical)==-1){
				return NULL;	
				}
			}
		return (va);
}
*/
/*
scv_t *findpages(int pages, scv_t* freed){
	get_next_avail(freed, pages);
	setpages(freed);
	return freed;
}
*/

void *m_alloc(unsigned int num_bytes) {
	/* Checks to see if physical memory has already been set */
	if (flag == 0){
		SetPhysicalMem();
		flag = 1;
	}
	if (num_bytes < 1){
		printf("%s\n", "Not a valid size");
		return;
	}
	int pages = (int) ceil(((double) num_bytes) / PGSIZE);
	scv_t* freed = scv_new(sizeof(char), pages);
	scv_t* ppages = findpages(pages,freed);	

   /* HINT: If the page directory is not initialized, then initialize the
   page directory. Next, using get_next_avail(), check if there are free pages. If
   free pages are available, set the bitmaps and map a new page. Note, you will 
   have to mark which physical pages are used. */

    return NULL;
}
/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void a_free(void *va, int size) {

    //Free the page table entries starting from this virtual address (va)
    // Also mark the pages free in the bitmap
    //Only free if the memory from "va" to va+size is valid
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void PutVal(void *va, void *val, int size) {
	void *pa = (void *) translate(pagedir, va);
	int mask = (1 << 12) - 1;
	int offset = ((pde_t) va) & mask;
	int copy_bytes;
	if (size <= PGSIZE - offset){
		copy_bytes = size;
	}
	else {
		copy_bytes = PGSIZE - offset;
	}
	while(size){
		memcpy(pa, val, copy_bytes);
		size -= copy_bytes;
		va += copy_bytes;
		pa = translate(pagedir, va);
		val += copy_bytes;
		if (size <= PGSIZE){
			copy_bytes = size;
		}
		else{
			copy_bytes = PGSIZE;
		}
	}		
		


    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/

}


/*Given a virtual address, this function copies the contents of the page to val*/
void GetVal(void *va, void *val, int size) {
	void *pa = (void *) translate(pagedir, va);
    	int mask = (1 << 12) - 1;
        int offset = ((pde_t) va) & mask;
        int copy_bytes;
	if (size <= PGSIZE - offset){
                 copy_bytes = size;
         }
         else {
                 copy_bytes = PGSIZE - offset;
         }
    while(size) {
        memcpy(val, pa, copy_bytes);

        size -= copy_bytes;
        va += copy_bytes;
        pa = translate(
                pagedir,
                va);

        val += copy_bytes;
        if (size <= PGSIZE - offset){
                 copy_bytes = size;
         }
         else {
                 copy_bytes = PGSIZE - offset;
         }

    }


    /* HINT: put the values pointed to by "va" inside the physical memory at given
    "val" address. Assume you can access "val" directly by derefencing them.
    If you are implementing TLB,  always check first the presence of translation
    in TLB before proceeding forward */


}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void MatMult(void *mat1, void *mat2, int size, void *answer) {
	int i = 0;
	for (i = 0; i < size; i++){
		int j, k = 0;
		for(j = 0; j < size; j++){
			int *answer2 = (int *) translate(pagedir, ((int *) answer) + i * size + j);
			*answer2 = 0;
			for (k = 0; k < size; k++){
				int *matrix1 = (int *) translate(pagedir, ((int *) mat1) + i * size + k);
				int *matrix2 = (int *) translate(pagedir, ((int *) mat2) + k * size + j);
				*answer2 += *matrix1 * *matrix2;
			}
		}
	} 
    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
    matrix accessed. Similar to the code in test.c, you will use GetVal() to
    load each element and perform multiplication. Take a look at test.c! In addition to 
    getting the values from two matrices, you will perform multiplication and 
    store the result to the "answer array"*/
}
