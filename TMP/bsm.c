/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
        int i;
        for (i = 0; i < 8; ++i)
        {
                bsm_tab[i].bs_num = 0;
                bsm_tab[i].nframes = NULL;
                bsm_tab[i].bs_status = 0; //not mapped
                bsm_tab[i].bs_pid = -1;
                bsm_tab[i].bs_vpno = 0;
                bsm_tab[i].bs_npages = 0;
                bsm_tab[i].bs_sem =  -1;
		bsm_tab[i].bs_private = 0;
		bsm_tab[i].bs_refcount = 0;
        }

        //curr_bs_num = 0;
        kprintf("backing store map initialized \n");
	//kprintf("bsm_tab[1].bs_status: %d\n",bsm_tab[1].bs_status);
        return OK;

}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	int i;
	for(i=0; i<8; i++){
		if(bsm_tab[i].bs_status == 0){ //returning the backing store that is unmapped
			*avail = i;
			//kprintf("*avail in get_bsm: %d\n", *avail);
			return i;
		}
		else
			return -1;
	}	
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{

	kprintf("vpno: %d, source: %d, npages: %d\n",vpno,source,npages);
	if(source<0 || source>=8) //backing stores are between 0 and 7
		return SYSERR;
	if(vpno < 4096)		 // Virtual memmory starts from 4096
		return SYSERR;
	if(npages<0 || npages > 256)
		return SYSERR;  //number of pages cannot exceed 256
	/* above 3 if conditions can be combined, separeated to mention the conditions along */

	// initializing proctab entries
	bs_map_t *map = proctab[pid].bs_map[source];
	map->bs_npages = npages;
	map->bs_pid = pid;
	map->bs_status = 1;
	map->bs_vpno = vpno;
	map->bs_refcount = 0;
	
        bsm_tab[source].bs_pid    = pid;
        bsm_tab[source].bs_status = 1;          /* setting the mapping */
        bsm_tab[source].bs_vpno   = vpno;
        bsm_tab[source].bs_npages = npages;
		
	return OK;	
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
}


