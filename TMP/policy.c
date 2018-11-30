/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  //kprintf("To be implemented!\n"); -> done
  if(policy == SC ){  	// the policy is initialized in the initialize.c file
	kprintf("Using SC page replacement policy\n");
	}else{
		kprintf("Using AGING page replacement policy\n");
		}
	//setting the policy to the extern variable
	page_replace_policy=policy;
  	// page replacement policy to be set for SC=3 and AGING=4
  return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
