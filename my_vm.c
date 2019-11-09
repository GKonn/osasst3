#include "my_vm.h"
int flag=0;
char *physicalmem;
char* virtualmap=NULL;
char* physicalmap=NULL;
int tablecount;	//counts tables created
int pagecount;	//counts pages allocated
pde_t* pagedir;

/*
Function responsible for allocating and setting your physical memory 
*/
int get_bitmap(bitmap_t map, int i){
return map[i/8]&(1<<(i/8));
}
void set_bitmap(bitmap_t map, int i, int bit){
map[i/8]=bit<<(i%8);
}
void SetPhysicalMem() {
	physicalmem=(char*)malloc(MEMSIZE*sizeof(char));
	
    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps are  initialized and zero'd
    //virtual mem is 2^20 bits (2^17 bytes)
    //physical mem is 2^18 bits (2^15 bytes)
	virtualmap=(char*) malloc((MEM_SIZE/PGSIZE)/8);
	bzero(virtualmap,(MEM_SIZE/PGSIZE)/8);
	physicalmap=(char*) malloc((MEM_SIZE/PGSIZE)/8);
	bzero(physicalmap, (MEM_SIZE/PGSIZE)/8);
    //initializes directory
	pagedir=(pde_t*)malloc(sizeof(pde_t)*1024);
	bzero(pagedir,1024);
    //creates pagetable for directory
	pte_t *pagetable=(pte_t*)malloc(sizeof(pte_t)*1024);
    //sets first entry of dir to table
	pagedir[0]=&pagetable;
    //sets first entry of table to memory location
	pagetable[0]=&physicalmem;
	return;
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
void* Translate(pde_t *pgdir, void *va) {
	
    //HINT: Get the Page directory index (1st level) Then get the
    //2nd-level-page table index using the virtual address.  Using the page
    //directory index and page table index get the physical address


    //If translation not successfull
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
PageMap(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to Translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    //Use virtual address bitmap to find the next free page
   /*Enact in loop
    * 1. Traverse virtual bitmap
    * 2. Locate null/free position 
    * 3. link virtual to physical
    
    */
	int temp,i,j;
	int group=((num_pages+1)<<1)-1;
	for(i=0;i<MEMSIZE/PGSIZE;i++){//bit by bit of vmap
		temp=virtualmap[i/8];//set to current section of bitmap
		if(!(temp&group)){//see if pages are free
		temp=4<<temp;//set temp to vaddr
			for(j=num_pages;j<num_pages;j++){
				if(PageMap(pagedir,virtualmap,physicalmap)==-1){
				return NULL;	
				}
			}
		return (void*)temp;
		}
	} 
}
void *next_physical(){
	int temp,i;
	for(i=0;i<MEMSIZE/PGSIZE;i+=PGSIZE){
		temp=physicalmap[i];
                if(getbitmap(physicalmap,i)==0){//freeblock
		//return addr of physical
		return (void*) &physicalmem+i*PGSIZE;
		}
	}
	return 0;
}

/* Function responsible for allocating pages
and used by the benchmark
*/
void *m_alloc(unsigned int num_bytes) {
    // If the physical memory is not yet initialized, then allocate and initialize.
	if(flag==0){
	//set physical space
	SetPhysicalMem();
	flag=1;
	}
    //checks how many pages are needed (bytes/pgsize+1
	int pages;
	if(num_bytes%(PGSIZE/8)!=0){
	pages=num_bytes/(PGSIZE/8);
	}
	else{
	pages=(num_bytes/(PGSIZE/8))+1;
	}
	return getnextavail(pages);
	
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

    /* HINT: Using the virtual address and Translate(), find the physical page. Copy
       the contents of "val" to a physical page. NOTE: The "size" value can be larger
       than one page. Therefore, you may have to find multiple pages using Translate()
       function.*/
	int pages,i,f;
	if(size%PGSIZE==0){
	pages=size/PGSIZE;
	f=1;
	}//divides evenly
	else{
	pages=(size/PGSIZE)+1;i
	}
	void* phys;
	//copy over byte by byte for each page
	for(i=0;i<pages;i++){
	phys=Translate(pagedir,va+PGSIZE);//set to physical mem addr
		if(f==1){
		memcpy(phys+(i*PGSIZE), val+(i*PGSIZE), PGSIZE);
		}
		else{
			if(size>PGSIZE){
			memcpy(phys+(i*PGSIZE), val+(i*PGSIZE), PGSIZE);
			size-=PGSIZE;
			}
			else{
			memcpy(phys+(i*PGSIZE), val+(i*PGSIZE), size);
			}
		}
	}
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

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
    matrix accessed. Similar to the code in test.c, you will use GetVal() to
    load each element and perform multiplication. Take a look at test.c! In addition to 
    getting the values from two matrices, you will perform multiplication and 
    store the result to the "answer array"*/

       
}
