#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "my_vm.h"
#include "scv.h"

void mapper(scv_t *phys_pages, pde_t *pgdir, void *va) {
    	char* vav;
	int i = 0;
    	for(i = 0, vav = (char *) va; i < scv_size(phys_pages); i++, va += PGSIZE) {
        	int page_num = *((int *) scv_at(phys_pages, i));
 		PageMap(pgdir, vav, scv_at(mdata.physicalmemory, page_num));
    	}
}

void setmem( scv_t *physicalmem, int pages) {
	int ret = scv_reserve(physicalmem, scv_size(physicalmem) + pages);
	assert(!ret);
	physicalmem->size = scv_capacity(physicalmem);
}
/*
void destroy_inner_pagetable() {
    int i = 0;
    for(; i < PGSIZE / sizeof(pde_t); i++)
        if(mdata.pagedir[i]) {
            free((void *) mdata.pagedir[i]);
        }
}
*/

void get_next_avail(scv_t *bitmap, scv_t *ret, int pages, int os) {
    	int pagesum = 0;
	int i = 0;
  	int j = 0;
    	for(i = os; i < scv_size(bitmap); i++) {
        	char *location = (char *) scv_at(bitmap, i);
        	for(j = 0; j < 8; j++) {
            		if(!((*location >> j) & 1)) {
                		int val = i * 8 + j;
				pagesum++;
                		scv_push_back(ret, &val);
            		}
            		if(pagesum == pages) {
                		return;
            		}
        	}
    	}
}

void resize(scv_t *bitmap, int pages) {
    	int num_chars = (int) ceil(pages / 8.0);
    	char a = 0;
    	while(num_chars--) {
        	scv_push_back(bitmap, &a);
    	}
}

void setpages(scv_t *bitmap, scv_t *pages, int vp, int vpn, bool free) {
    		if(vpn) {
            		int os = vp % 8;
			int i = vp / 8;
            		int vps;
   			if (vpn <= 8 - os){
				vps = vpn;
			}
			else {
				vps = 8 - os;
			}
        		while(i < scv_size(mdata.virtualmap) && vpn) {
            			pde_t mask = (1 << vps) - 1;
            			mask <<= os;
            			if(free) {
                			*((char *) scv_at(bitmap, i)) &= (char) ~(mask);
            			}
				else {
                			*((char *) scv_at(bitmap, i)) |= (char) mask;
            			}
            			i++;
            			vpn -= vps;
            			os = 0;
				if (vpn <= 8 - os){
                			vps = vpn;
                		}
                		else {
                			vps = 8 - os;
                		}
    			}
		} 
		else {
        		int i = 0;
			int j = 0;
        		for(i = 0; i < scv_size(bitmap); i++) {
            			char *temp = (char *) scv_at(bitmap, i);
            			while(j < scv_size(pages)) {
                			int *val = (int *) scv_at(pages, j);
                			if(*val / 8 != i) {
                    				break;
                			}
                			if(free) {
                    				*temp &= ~(1 << (*val % 8)); 
                			} 
					else {
                    				*temp |= 1 << (*val % 8);
                			}
					j++;
            			}
            			if(j >= scv_size(pages)) {
                			break;
            			}
        		}
    	}
}

int Consecutive(scv_t *bitmap, int pages) {
    	int consecutive = 0;
	int i = 0;
    	for(i = 0; i < scv_size(bitmap); i++) {
        	int j;
        	char val = *((char *) scv_at(bitmap, i));
        	for(j = 0; j < 8; j++) {
            		if((val >> j) & 1) {
                		consecutive = 0;
            		}	 
			else {
                		consecutive++;
            		}
            		if(consecutive == pages) {
                		return i * 8 + j - consecutive + 1;
            		}
        	}
    	}	
    return -1;
}

scv_t *findpages(int pages) {
    	scv_t *freed = scv_new(sizeof(int), pages);
    	get_next_avail(mdata.physicalmap, freed, pages, 0);
    
	if(scv_size(freed) < pages) {
		int os = scv_size(mdata.physicalmap);
        	pages -= scv_size(freed);
        	resize(mdata.physicalmap, pages);
        	get_next_avail(mdata.physicalmap, freed, pages, os);
        	while(pages % 8){
            		pages++;
        		setmem(mdata.physicalmemory, pages);
    		}
	}
    	setpages(mdata.physicalmap, freed,0,0,false);
    	return freed;
}

void *findvirtual(scv_t *phys_pages) {
	int vpn = scv_size(phys_pages);
        int vpa = vpn;
	int vp = Consecutive(mdata.virtualmap, scv_size(phys_pages));

    // if no virt_pages are available, make new ones
    if(vp == -1) {
        vp = scv_size(mdata.virtualmap) * 8;

        int j;
        char last = *((char *) scv_back(mdata.virtualmap));
        
        for(j = 7; j > 0; j--) {
            if(!((last >> j) & 1)) {
                vp--;
                vpa--;
            } else {
                break;
            }
        }
        resize(mdata.virtualmap, vpa);
    }
    	void *va = (void *) (vp << 12);
    	setpages(mdata.virtualmap, NULL, vp, vpn, false);
    	mapper(phys_pages,mdata.pagedir, va);
    	return va;
}
