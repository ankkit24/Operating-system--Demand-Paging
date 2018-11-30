#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
   if(bs_id<0 || bs_id>=8)
	return SYSERR;

   // unmapping the backing store
   bsm_tab[bs_id].bs_npages = 0;
   bsm_tab[bs_id].bs_pid = -1;
   bsm_tab[bs_id].bs_refcount = 0;
   bsm_tab[bs_id].bs_sem = -1;
   bsm_tab[bs_id].bs_status = 0;
   bsm_tab[bs_id].bs_vpno = -1;        
 
   unsigned int pdbr = (proctab[currpid].pdbr)/4096;

   write_cr3(proctab[currpid].pdbr);
	
   //kprintf("To be implemented!\n");
   return OK;

}

