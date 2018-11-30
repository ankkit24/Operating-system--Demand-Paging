/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	
        struct mblock *left;
        nbytes = (unsigned int) roundmb(nbytes);

        struct mblock *vmlist = proctab[currpid].vmemlist; //mblock structure is defined in mem.h header file

        if (vmlist->mnext == NULL || nbytes == 0) {
		return SYSERR;
        }

        struct mblock *prev = vmlist;
        struct mblock *next;
	for(next = vmlist->mnext; next!=NULL; next=next->mnext){
                if (next->mlen > nbytes) {
                        left = next + nbytes;
                        left->mnext = next->mnext;
                        left->mlen = next->mlen - nbytes;
                        prev->mnext = left;
                        return ((WORD *)next);
                } else if (next->mlen == nbytes) {
                        prev->mnext = next->mnext;
                        return ((WORD *)next);
                }
                prev = next;
        }
	
	//kprintf("To be implemented!\n");
	return( SYSERR );
}


