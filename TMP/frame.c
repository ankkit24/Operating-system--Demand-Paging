/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

#define get_physical_address_from_frame(i) 	 ((unsigned int) 4096*(1024+i))

fr_map_t frm_tab[NFRAMES]; //extern variable defined in the paging.h file
extern int global_pt[]; //common page table for all the processes
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	int i;
	for(i=0;i<NFRAMES;i++){	
        	frm_tab[i].list_bs = NULL;
        	frm_tab[i].fr_indicator = -1;
        	frm_tab[i].fr_pid = -1;  //current pid
		frm_tab[i].fr_status = 0; //mapping the frame with status=1
        	frm_tab[i].fr_refcnt = 0;
        	frm_tab[i].fr_type = 0;
        	frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_vpno =-1;
		frm_tab[i].fr_id = -1;
	}
        kprintf("Frame initialization complete\n");
  	//kprintf("To be implemented!\n"); -> done
  	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	// if the frame is unmapped, then return the frame
	int i;
	for(i=0;i<NFRAMES;i++){
		if(frm_tab[i].fr_status == 0){
			*avail = i; //returning the unmapped frame
			//kprintf("Returning Unmapped frame\n");
			return i;
		}
	}
	if (page_replace_policy == SC) {
		//kprintf("Calling SC replacement policy\n");
                return getFrame_sc();
        }
	if(page_replace_policy == AGING){
		//kprintf("Calling AGING replacement policy\n");
		return getFrame_aging();
	}
	return SYSERR; //if no frame is mapped	

}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{	
  	return OK;
}


void removeFrameTableEntry(pd_t *pd_entry, int i, int isFR_PAGE) {
        frm_tab[i].fr_dirty = 0;
        frm_tab[i].fr_pid = -1;
        frm_tab[i].fr_refcnt = 0;
        frm_tab[i].fr_status = 0;
        frm_tab[i].fr_type = 0;
        frm_tab[i].fr_vpno = 0;
}

int global_pt[4];

int createPageDir(int pid) {
        int i,avail;
        int frame = get_frm(&avail);
	if(frame == -1){
		kprintf("bad frame\n");
                return -1;
        }
        frm_tab[frame].fr_pid = pid;
        frm_tab[frame].fr_status = 1;
        frm_tab[frame].fr_type = FR_DIR;
        frm_tab[frame].fr_refcnt=4;
	frm_tab[frame].fr_pf_accessed=0;
 
        proctab[pid].pdbr = (FRAME0 + frame) * NBPG;
	//kprintf("page table pdbr: %lu",proctab[pid].pdbr);
        for (i = 0; i < 1024; i++) {
                pd_t *pd_entry = proctab[pid].pdbr + (i * sizeof(pd_t));
                pd_entry->pd_pcd = 0;
                pd_entry->pd_acc = 0;
                pd_entry->pd_mbz = 0;
                pd_entry->pd_fmb = 0;
                pd_entry->pd_global = 0;
                pd_entry->pd_avail = 0;
                pd_entry->pd_base = 0;
                pd_entry->pd_pres = 0;
                pd_entry->pd_write = 1;
                pd_entry->pd_user = 0;
                pd_entry->pd_pwt = 0;

                if (i < 4) {
                        pd_entry->pd_pres = 1;
                        pd_entry->pd_write = 1;
                        pd_entry->pd_base = global_pt[i];
                }
        }
	//kprintf("page dir created\n");
        return frame;
}


int initializeGlobalPageTable() {
        int i, j, k;
        for (i = 0; i < 4; i++) {
                k = createPageTable(NULLPROC);
                if (k == -1) {
			kprintf("page table not initiallized\n");
                        return SYSERR;
                }
		frm_tab[k].fr_global = 1;
                global_pt[i] = FRAME0 + k;

                for (j = 0; j < 1024; j++) {
                        pt_t *pt = global_pt[i] * NBPG + j * sizeof(pt_t);
                        pt->pt_pres = 1;
                        pt->pt_write = 1;
                        pt->pt_base = j + i * FRAME0;
                        frm_tab[k].fr_refcnt++;
                }
        }
	kprintf("global page table initialized\n");
        return OK;
}

int createPageTable(int pid) {

        int i,avail;
        int frame = get_frm(&avail);
	kprintf("pt frame value: %d\n",frame);
        if (frame == -1) {
		kprintf("page table not created\n");
                return SYSERR;
        }

        frm_tab[frame].fr_refcnt = 0;
        frm_tab[frame].fr_type = FR_TBL;
        frm_tab[frame].fr_dirty = 0;
        frm_tab[frame].fr_status = 1;
        frm_tab[frame].fr_pid = pid;
        frm_tab[frame].fr_vpno = -1;
	frm_tab[frame].fr_pf_accessed = 0;
	frm_tab[frame].fr_global = 0;

        for (i = 0; i < 1024; i++) {
              	pt_t *pt = (FRAME0 + frame) * NBPG + i * sizeof(pt_t);
                pt->pt_dirty = 0;
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
	//kprintf("page table created\n");
        return frame;
}

int write2CR3(int pid) {
        unsigned int pdbr = (proctab[pid].pdbr) / NBPG;
        write_cr3(proctab[pid].pdbr);
        return OK;
}


// Get the frame using Second Chance Page replacement
int getFrame_sc(){
	int i=0;
	//kprintf("pfa: %d, type: %d\n", frm_tab[i].fr_pf_accessed, frm_tab[i].fr_type);
        //frm_tab[i].fr_refcnt = 0;
	while(i < NFRAMES){
		if(frm_tab[i].fr_global == 1){
			i++;
			continue;
		}
		if(frm_tab[i].fr_type == FR_PAGE){
			if(frm_tab[i].fr_pf_accessed==1){
				frm_tab[i].fr_pf_accessed = 0;
				i++;
			}else{
				free_frame(i);
				return i;
			}
		}
		i++;
	}

	i = 0;
	while(i < NFRAMES){
		if(frm_tab[i].fr_type == FR_PAGE){
                        if(frm_tab[i].fr_pf_accessed==0){
				return i;	
			}
		}else
			i++;
	}
	//kprintf("i: %d\n",i);	
	return SYSERR;	
}

int getFrame_aging(){

	int i=0, timestamp=10000000, ret_frame;
	while(i < NFRAMES){
                if(frm_tab[i].fr_global == 1){
                        i++;
                        continue;
                }
                if(frm_tab[i].fr_type == FR_PAGE){
			if(frm_tab[i].fr_age < timestamp){
				timestamp = frm_tab[i].fr_age;
				ret_frame = i;
			}
			i++;	
		}else
			i++;
	}
	//kprintf("i value: %d, ret_frame: %d\n", i, ret_frame);
	free_frame(ret_frame);
	//kprintf("after free frame\n");
	return ret_frame;			
}

int free_frame(int id){

	//kprintf("id value from getFrame_sc: %d\n",frm_tab[id].fr_type);

        if(frm_tab[id].fr_type == FR_PAGE){
                int store, pageth;

 	unsigned long v_addr, pdbr;
  	unsigned int pt_offset, pd_offset;
	pt_t *pt_entry;
 	pd_t *pd_entry;
 	int bs_store, page_num, frame_pid;
    	v_addr = frm_tab[id].fr_vpno;
    	frame_pid = frm_tab[id].fr_pid;

    	pd_offset = v_addr>>22;
   	pt_offset = (v_addr >> 12) & 0x000003ff;
    	pdbr = proctab[frame_pid].pdbr;
   	pd_entry = pdbr + (pd_offset * sizeof(pd_t));
    	pt_entry = (pd_entry->pd_base * NBPG) + (pt_offset * sizeof(pt_t));
    	bs_store = proctab[frm_tab[id].fr_pid].store;
    	page_num = frm_tab[id].fr_vpno - proctab[frame_pid].vhpno;
        	
	if (pt_entry->pt_pres == 0) {
                 return SYSERR;
        }
        write_bs((id + 1024) * 4096, bs_store, page_num);
        pt_entry->pt_acc = 0;
        pt_entry->pt_avail = 0;
        pt_entry->pt_base = 0;
        pt_entry->pt_dirty = 0;
        pt_entry->pt_global = 0;
        pt_entry->pt_mbz = 0;
        pt_entry->pt_pcd = 0;
        pt_entry->pt_pres = 0;
        pt_entry->pt_pwt = 0;
        pt_entry->pt_write = 1;
        pt_entry->pt_user = 0;

        removeFrameTableEntry(pd_entry, id, 1);
        return OK;
}

        if(frm_tab[id].fr_type == FR_TBL){
                int j;
                for (j = 0; j < 1024; j++) {
                        pt_t *pt_entry = (id + 1024) * 4096 + j * sizeof(pt_t);
                        if (pt_entry->pt_pres == 1) {
                                free_frm(pt_entry->pt_base - 1024);
                        }
                }
                for (j = 0; j < 1024; j++) {
                        pd_t *pd_entry = proctab[frm_tab[id].fr_pid].pdbr + j * sizeof(pd_t);

                        if (pd_entry->pd_base - 1024 == id) {
                                pd_entry->pd_pres = 0;
                                pd_entry->pd_write = 1;
                                pd_entry->pd_user = 0;
                                pd_entry->pd_pwt = 0;
                                pd_entry->pd_pcd = 0;
                                pd_entry->pd_acc = 0;
                                pd_entry->pd_mbz = 0;
                                pd_entry->pd_fmb = 0;
                                pd_entry->pd_global = 0;
                                pd_entry->pd_avail = 0;
                                pd_entry->pd_base = 0;
                        }
                }

                removeFrameTableEntry(NULL, id, 0);
                return OK;
        }

        if(frm_tab[id].fr_type == FR_DIR ){
                int j;
                for (j = 4; j < 1024; j++) {
                        pd_t *pd_entry = proctab[frm_tab[id].fr_pid].pdbr + j * sizeof(pd_t);
                        if (pd_entry->pd_pres == 1) {
                                free_frm(pd_entry->pd_base - 1024);
                        }
                }
                removeFrameTableEntry(NULL, id, 0);
                return OK;
        }

        return OK;
}


