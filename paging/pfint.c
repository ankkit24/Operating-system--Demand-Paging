/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
int global_age = 0;

SYSCALL pfint()
{

  	int store,pageth;
//	int global_age = 0;
  	// Getting the faulted address 'a' -> Point 1 requirement
  	unsigned int a = read_cr2();
	 //reading the cr2 register for the page fault address
  	//checking if the address is valid by looking up for the address in the backing store
  	//this happens when the page table shows that the bit is invalid and the address copied in the cr2 register
	//kprintf("store: %d, pageth: %d\n", store, pageth);
  	//checking if a is a legal address -> Point 4 requirement
 	if((bstore_lookup(currpid, a, &store, &pageth) == SYSERR) || store == -1 || pageth == -1){
		//kprintf("Error message: 'a' has not been mapped in pd");
		kill(currpid);
		return SYSERR;	
  	}

  	// 'pd' pointing to the current page directry -> Point 3 requirement
  	unsigned int p = a>>22; 		   // p -> upper 10 bits of 'a'
  	unsigned int q = (a/NBPG) & 0x000fffff;   // q -> lower bits of 'a'

	// page directory
	// get the page dir entry
  	pd_t *pde = proctab[currpid].pdbr + sizeof(pd_t) * (p);

	// check if the pde is present
        if (pde->pd_pres == 0) { 
                int new_frame = new_pt_entry(currpid);
                if (new_frame == 0) {
			kprintf("No free frame, apply replacement policy .\n");
			kill(currpid);	
                        return SYSERR;
                }

                pde->pd_pres = 1;
                pde->pd_write = 1;
                pde->pd_base = new_frame + FRAME0;
		
        	frm_tab[new_frame].fr_pid = currpid;
        	frm_tab[new_frame].fr_status = 1;
        	frm_tab[new_frame].fr_type = FR_PAGE;
 		frm_tab[new_frame].fr_pf_accessed =1;
		frm_tab[new_frame].fr_age = global_age++;		
 		//kprintf("Page dir entry created in pfint\n");
	}
	
	//Page Table
	unsigned int pto = (a >> 12) & 0x000003ff; //page table offset
	pt_t *pte = (pde->pd_base) * NBPG + sizeof(pt_t) * pto;
	if(pte->pt_pres != 1){

        	int new_frame = new_pt_entry(currpid);
                if (new_frame == -1) {
                        kprintf("No free frame\n");
                                return SYSERR;
                }		
        	pte->pt_pres = 1;
     		pte->pt_write = 1;
        	pte->pt_base = new_frame + FRAME0;
        	frm_tab[(pde->pd_base) - FRAME0].fr_refcnt++;
		frm_tab[new_frame].fr_pf_accessed = 1;
		frm_tab[new_frame].fr_pid = currpid;
		frm_tab[new_frame].fr_status = 1;
		frm_tab[new_frame].fr_type = FR_PAGE;
		frm_tab[new_frame].fr_age = global_age++;
		//kprintf("Page table entry created in pfint\n");
		//kprintf("store: %d, pageth: %d\n",store, pageth);

		read_bs((char *)((new_frame + FRAME0) * NBPG), store, pageth);
			
		//kprintf("read_bs completed in pfint\n");
		
	}
	unsigned long pdbr = proctab[currpid].pdbr;
	//kprintf("currpid: %d, pfint: %lu\n",currpid, proctab[currpid].pdbr);
        write2CR3(currpid);

  	//kprintf("To be implemented!\n");
  	return OK;
}

int new_pt_entry(int pid) {

        int i,avail;
        int new_frame = get_frm(&avail);
        if (new_frame == -1) {
                return SYSERR;
        }
	
	//updating the frame tab entries for the created page table of the process
        frm_tab[new_frame].fr_refcnt = 0;
        frm_tab[new_frame].fr_type = FR_PAGE;
        frm_tab[new_frame].fr_dirty = 0; // page is not dirty/is clean
        frm_tab[new_frame].fr_status = 1; // 1 for mapped
        frm_tab[new_frame].fr_pid = pid;
        frm_tab[new_frame].fr_vpno = -1;
        frm_tab[new_frame].fr_pf_accessed = 0;
        frm_tab[new_frame].fr_global = 0;
	frm_tab[new_frame].fr_age = 0;

        for (i = 0; i < 1024; i++) {
                pt_t *pt = (FRAME0 + new_frame) * NBPG + i * sizeof(pt_t);
                pt->pt_dirty = 0; //page is not dirty/is clean
                pt->pt_mbz = 0;
                pt->pt_global = 0;
                pt->pt_avail = 0;
                pt->pt_base = 0;
                pt->pt_pres = 0;
                pt->pt_write = 1;
                pt->pt_user = 0;
                pt->pt_pwt = 0;
                pt->pt_pcd = 0;
                pt->pt_acc = 0;
        }
        return new_frame;
}
