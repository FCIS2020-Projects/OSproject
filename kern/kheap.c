#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

struct Free_FRACTIONS
{
	int counter;
	uint32* start;
};
struct Free_FRACTIONS arr[1024];
struct Free_FRACTIONS notfree[1024];
bool b = 1;
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//NOTE: Allocation is based on BEST FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	//TODO: [PROJECT 2019 - BONUS1] Implement the FIRST FIT strategy for Kernel allocation
	// Beside the BEST FIT
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy
	//change this "return" according to your answer



	size = ROUNDUP(size,PAGE_SIZE);
	unsigned int s=size/PAGE_SIZE;
//	if(b == 1)
//	{
		get_free_spaces();
//		b = 0;
//	}

	struct Free_FRACTIONS bestfit;
	bestfit.start = NULL;
	int min = 100000;
	//cprintf("k");
	for(int i=0 ;i<1024 ;i++){
		if(arr[i].counter == s)
		{
			bestfit = arr[i];
			break;
		}
		else if(arr[i].counter>s && arr[i].counter-s<min)
		{
			//cprintf("i = %d\n", i);
			//cprintf("s = %d\n", s);
			//cprintf("arr[i].start = %x\n", arr[i].start);
			bestfit = arr[i];
			min = arr[i].counter-s;
		}
	}
	if (bestfit.start != NULL)
	{
		for(int i =0;i<s;i++)
		{
			struct Frame_Info* ptr_frame_info;
			int ret = allocate_frame(&ptr_frame_info) ;
			map_frame(ptr_page_directory, ptr_frame_info, (void*)bestfit.start + i*PAGE_SIZE, PERM_PRESENT|PERM_WRITEABLE);
		}
	}


	////////////////////////////////
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
	////////////////////////////////////////////
	return bestfit.start;
}

void get_free_spaces()
{
	for(int i = 0;i<1024;i++){
		arr[i].counter = 0;
		arr[i].start = NULL;
	}
	int arrindex = 0;
	for(int i = KERNEL_HEAP_START ;i < KERNEL_HEAP_MAX ;i+=PAGE_SIZE){
		struct Frame_Info* fr = NULL;
		uint32* page_table = NULL;
		fr = get_frame_info(ptr_page_directory,(void*)i,&page_table);
		if(fr==NULL)
		{
			if(arr[arrindex].counter == 0)
				arr[arrindex].start =(uint32*) i;
			arr[arrindex].counter += 1;
		}
		else
		{

			if(arr[arrindex].counter > 0)
				arrindex++;
		}

	}
}
void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");



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
	for(int i=0;i<counter;i++)
	{
		unmap_frame(ptr_page_directory , virtual_address+i*PAGE_SIZE);

	}

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_virtual_address()


		int fr_num=to_frame_number(to_frame_info(physical_address));
		uint32 *ptrPageTable1=NULL;
		int py=0;
		for(uint32 start=KERNEL_HEAP_START; start<KERNEL_HEAP_MAX; start+=PAGE_SIZE)
		{
			//cprintf("start=%x\n",start);
			ptrPageTable1=NULL;
			get_page_table(ptr_page_directory,(void*)start,&ptrPageTable1);
			if( ptrPageTable1!=NULL&&(ptrPageTable1[PTX(start)]&PERM_PRESENT) )
			{
				py=ptrPageTable1[PTX(start)]>>12;
				if(py==fr_num)
				{
					start&=0xFFFFF000;
					return start;
				}
			}

		}

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	uint32 *ptrPageTable=NULL;
		uint32 py=0;
		struct Frame_Info* ptr_fr=NULL;

		get_page_table(ptr_page_directory,(void*)virtual_address,&ptrPageTable);
		if(ptrPageTable!=NULL)
		{
			return ptrPageTable[PTX(virtual_address)]&0xFFFFF000;
		}
		return 0;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code
	if(new_size==0)
	{
		kfree(virtual_address);
		return NULL;
	}
	int counter=0;
	for(int i = 0 ; i < 1024 ; i++)
	{
		if(notfree[i].start == ROUNDDOWN(virtual_address,PAGE_SIZE))
		{
			counter = notfree[i].counter;
			break;
		}
	}
	int current_size=counter*PAGE_SIZE;
	cprintf("\ncurrent_size=%d new_size=%d\n",current_size,new_size);
	if(counter*PAGE_SIZE>=new_size)
		return virtual_address;

	void* newva =kmalloc(new_size);
	if(newva!=NULL)
	{
		kfree(virtual_address);
		return newva;
	}
	return NULL;
	panic("krealloc() is not implemented yet...!!");

}
