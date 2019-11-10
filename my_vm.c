#include "my_vm.h"

/* Globals */
int flag = 0;
pde_t *pagedir;
char* physicalmap;
char* virtualmap;
char* physicalmem;
void SetPhysicalMem() {	
	physicalmem = malloc(MEMSIZE);
	physicalmap = malloc(PAGENUM);
	virtualmap = malloc(PAGENUM);
	//Zero out the bitmaps
	bzero(physicalmap, BITMAPSIZE);
	bzero(virtualmap, BITMAPSIZE);
	// Creates the page directory and zeroes it out
	pagedir = malloc(sizeof(pde_t) * 1024);
	bzero(pagedir, 1024 * sizeof(pde_t));	
	
}
int get_bitmap(bitmap_t map, int i){
return map[i/8]&(1<<(i/8));
}
void set_bitmap(bitmap_t map, int i, int bit){
map[i/8]|=bit<<(i%8);
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
int PageMap(pde_t *pgdir, void *va, void *pa) {//note: needs to allocate page tables as necessary
	if(translate(pgdir,va)!=NULL){
	return -1;
	}
	int offset  = 12;
	int secondoffset = 10;
        long mask = (1 << secondoffset) - 1; 
     	pde_t virtualAddr = (pte_t) va,
        firstbits = virtualAddr >> (secondoffset + offset) & mask,
        secondbits = (virtualAddr >> offset) & mask;
	if(!pgdir[firstbits]){
		pte_t *new = (pte_t *) calloc( PGSIZE / sizeof(pte_t), sizeof(pte_t));
	return -1;
	}
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



void *next_physical(){
	int temp, i = 0;
	for(i = 0; i < PAGENUM; i += PGSIZE){
		temp = physicalmap[i];
		if(get_bitmap(physicalmap, i) == 0){
			return (void*) &physicalmem + i * PGSIZE;
		}
	}
}
int checkrest(int i,int pages){
int counter;
	for(counter=i+1;counter<i+pages;counter++){
		if(get_bitmap(physicalmap,counter)!=0){
		return -1;
		}
	}
	return 0;
}

void *get_next_avail(int num_pages) {
	int temp, i, j = 0;
	//int group = ((1<< num_pages+1)-1);
	for (i = 0; i < PAGENUM; i++){
		if(get_bitmap(physicalmap,i)==0&&checkrest(i,num_pages)==0){//current bit is set
			for(j=num_pages;j<num_pages;j++){
				if(PageMap(pagedir ,(void*)(i+j<<12),next_physical)==-1){
				return (void*)(i<<12);	
				}
			}
		return (void*)temp; 
	 	}
	}

}

void *m_alloc(unsigned int num_bytes) {
	/* Checks to see if physical memory has already been set */
	if (flag == 0){
		SetPhysicalMem();
		flag = 1;
	}

	/* Make sure that you are trying to malloc a legal number of bytes */
	if (num_bytes < 1){
		printf("%s\n", "Number of bytes must be 1 or greater");
		return;
	}
	int pages;
	/* Checks how many pages it will take for this malloc call */
	if(num_bytes%(PGSIZE)==0){
	pages=num_bytes/(PGSIZE);
	}
	else{
	pages=(num_bytes/PGSIZE)+1;
	}
		
	return get_next_avail(pages);
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
	
    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/

}


/*Given a virtual address, this function copies the contents of the page to val*/
void GetVal(void *va, void *val, int size) {

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
