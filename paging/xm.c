/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{

  if(source< 0 || source> 7 ){
		return SYSERR;
	}else{
 		// 'source' is the backing store number, we set all the parameters of the bsm_tab for this store to map
 		bsm_tab[source].bs_pid 	  = currpid;
		bsm_tab[source].bs_status = 1; 		/* setting the mapping */
		bsm_tab[source].bs_vpno   = virtpage;
		bsm_tab[source].bs_npages = npages;
		bsm_tab[source].bs_sem    = 0;

		bs_map_t *map = proctab[currpid].bs_map[source];
		map->bs_status   = 1; 		//value 1 for mapped status
		map->bs_vpno     = virtpage;
		map->bs_npages   = npages;
		map->bs_pid      = currpid;
		map->bs_refcount = 0;
		//kprintf("xmmap success\n");

		return OK;
	}
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{	
	//virtual page has to be beyond 4096, as the virtual memory is beyond 4096 in the Xinu memory location
	if(virtpage < 4096){ 
		kprintf("Virtual memory belongs beyond 4096\n");
		return SYSERR;
	}
	
	int store, pageth;
	store = proctab[currpid].store;
	bsm_tab[store].bs_status = 0;
    	bsm_tab[store].bs_pid = -1;
    	bsm_tab[store].bs_vpno = 0;
    	bsm_tab[store].bs_npages = 0;

        bs_map_t *map = proctab[currpid].bs_map[store];
        map->bs_npages = 0;
        map->bs_pid = -1;
        map->bs_status = 0;
        map->bs_vpno = 0;

	write_cr3(proctab[currpid].pdbr);
	
        return OK;	
  	  //kprintf("To be implemented!");
  	  //return SYSERR;
}

/* Backing store lookup function */
SYSCALL bstore_lookup(int pid, int vpage, int *store, int *pageth){
		
	//Checking the backing store for the current process
	int i;
	unsigned int vpno = (vpage / NBPG);

	for(i=0;i<8;i++){
		bs_map_t *bs = proctab[pid].bs_map[i];
		// condition for mapped backing store and present in the mapped backing store
		if( (bs->bs_status == 1) && (vpno >= bs->bs_vpno) && (vpno < bs->bs_vpno+bs->bs_npages) ){
			//kprintf("Mapping found\n");
			*store = i;
			*pageth = vpno - bs->bs_vpno;
			return OK;
		}else{
			*store = -1;
			*pageth  = -1;
			//kprintf("Mapping not found\n");
			return SYSERR;
		}
	}
}
