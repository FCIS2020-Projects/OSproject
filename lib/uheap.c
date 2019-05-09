
#include <inc/lib.h>

// malloc()
//	This function use BEST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
struct Segment
{
	int counter;
	uint32* start;
};
struct Segment free_segments[1024];
struct Segment notfree[1024];

void get_free_spaces()
{
	for(int i = 0;i<1024;i++){
		free_segments[i].counter = 0;
		free_segments[i].start = NULL;
	}
	int arrindex = 0;
	int notfreeindex = 0;
	for(int i = USER_HEAP_START ;i < USER_HEAP_MAX ;i+=PAGE_SIZE){
		struct Frame_Info* fr = NULL;
		uint32* page_table = NULL;
		if(i >= (int)notfree[notfreeindex].start + notfree[notfreeindex].counter*PAGE_SIZE)
			notfreeindex++;
		if(i < (int)notfree[notfreeindex].start)
		{
			if(free_segments[arrindex].counter == 0)
				free_segments[arrindex].start =(uint32*) i;
			free_segments[arrindex].counter += 1;
		}
		else
		{

			if(free_segments[arrindex].counter > 0)
				arrindex++;
		}

	}
}

uint32* BESTFIT_Strategy( int s)
{
	get_free_spaces();

	struct Segment bestfit;
		bestfit.start = NULL;
		int min = (USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE;

		for(int i=0 ;i<1024 ;i++){
			if(free_segments[i].counter == s)
			{
				bestfit = free_segments[i];
				break;
			}
			else if(free_segments[i].counter > s && free_segments[i].counter - s < min)
			{
				bestfit = free_segments[i];
				min = free_segments[i].counter - s;
			}
		}

		if(bestfit.start != NULL)
		{
			for(int i=0 ;i<1024 ;i++){
				if(notfree[i].counter == 0)
				{
					notfree[i].start = bestfit.start;
					notfree[i].counter = s;
					break;
				}
			}
		}
		return bestfit.start;
}
void* malloc(uint32 size)
{
	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code

	// Steps:
	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)

	size = ROUNDUP(size,PAGE_SIZE);
	unsigned int s=size/PAGE_SIZE;

	uint32* virtual_address = BESTFIT_Strategy(s);

	//	2) if no suitable space found, return NULL
	if(virtual_address == NULL)
		return NULL;
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	sys_allocateMem((uint32)virtual_address, s);
	// 	4) Return pointer containing the virtual address of allocated space,
	//
	return virtual_address;
	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer
}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT 2019 - MS2 - [6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");

	// Steps:


	//	 Else,


	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	size = ROUNDUP(size,PAGE_SIZE);
	unsigned int s=size/PAGE_SIZE;

	uint32* VirtualAddress = BESTFIT_Strategy(s);

	//	2) if no suitable space found, return NULL
	if (VirtualAddress == NULL)
		return NULL;

	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable.
	//                                 Else, it returns -ve

	//char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
	int objID = sys_createSharedObject(sharedVarName,size,isWritable,VirtualAddress);

	//objID != E_NO_SHARE && objID != E_SHARED_MEM_EXISTS
	if(objID != E_NO_SHARE && objID != E_SHARED_MEM_EXISTS)
	{
		return (void*)VirtualAddress;
	}
	else
		return NULL;

	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT 2019 - MS2 - [6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	// Steps:

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer
	int size ;

	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	int size_shObj = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	//	2) If not exists, return NULL
	if(size_shObj != E_SHARED_MEM_NOT_EXISTS )
	{

		//	3) Implement BEST FIT strategy to search the heap for suitable space
		//		to share the variable (should be on 4 KB BOUNDARY)
		size = ROUNDUP(size_shObj,PAGE_SIZE);
		unsigned int s=size/PAGE_SIZE;

		uint32* VirtualAddress = BESTFIT_Strategy(s);

		//	4) if no suitable space found, return NULL
		if (VirtualAddress == NULL)
			return NULL;

		//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
		//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable.
		//                             Else, it returns -ve

		int objID = sys_getSharedObject(ownerEnvID,sharedVarName,VirtualAddress);
		//objID != E_SHARED_MEM_NOT_EXISTS
		if(objID != E_SHARED_MEM_NOT_EXISTS)
		{
			return (void*)VirtualAddress;
		}
		else
			return NULL;
	}
	else
		return NULL;

	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL

}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code

	//you should get the size of the given allocation using its address
	int counter;
	for(int i = 0 ; i < 1024 ; i++)
	{
		if(notfree[i].start == virtual_address)
		{
			counter = notfree[i].counter;
			notfree[i].counter = 0;
			break;
		}
	}
	//you need to call sys_freeMem()
	sys_freeMem((uint32)virtual_address, counter);

	//refer to the project presentation and documentation for details
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - BONUS4] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

}
