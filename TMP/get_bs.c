#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

bs_map_t bsm_tab[];
init_bsm();

int get_bs(bsd_t bs_id, unsigned int npages) {
     /* requests a new mapping of npages with ID map_id */
    //npages should not be greater than 256 or negative
    if(npages>256 || npages<0){
    	return SYSERR;
    }

    //bacing store numbers should be between 0 and 7 (only 8 backing stores available)
    if(bs_id<0 || bs_id>7){
    	return SYSERR;
    }

    //kprintf("bs_id: %d\n",bs_id);
    //kprintf("bsm_tab[bs_id].bs_status: %d\n", bsm_tab[bs_id].bs_status);

    if (bsm_tab[bs_id].bs_status == 0) {
    	if (bsm_map(currpid, 4096, bs_id, npages) == SYSERR) {
                return SYSERR;
        }
        	return npages;
        }else {
                return bsm_tab[bs_id].bs_npages;
        }
}


