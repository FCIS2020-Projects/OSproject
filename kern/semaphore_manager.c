#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/environment_definitions.h>

#include <kern/semaphore_manager.h>
#include <kern/memory_manager.h>
#include <kern/sched.h>
#include <kern/kheap.h>

//==================================================================================//
//============================== HELPER FUNCTIONS ==================================//
//==================================================================================//
//Helper functions to deal with the semaphore queue
//void enqueue(struct Env_Queue* queue, struct Env* env);
//struct Env* dequeue(struct Env_Queue* queue);

///// Helper Functions
//void enqueue(struct Env_Queue* queue, struct Env* env)
//{
//	LIST_INSERT_HEAD(queue, env);
//}
//
//struct Env* dequeue(struct Env_Queue* queue)
//{
//	struct Env* envItem = LIST_LAST(queue);
//	LIST_REMOVE(queue, envItem);
//	return envItem;
//}

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===============================
// [1] Create "semaphores" array:
//===============================
//Dynamically allocate the "semaphores" array
//initialize the "semaphores" array by 0's and empty = 1
void create_semaphores_array(uint32 numOfSemaphores)
{
	semaphores = (struct Semaphore*) kmalloc(numOfSemaphores*sizeof(struct Semaphore));
	if (semaphores == NULL)
	{
		panic("Kernel runs out of memory\nCan't create the array of semaphores.");
	}
	for (int i = 0; i < MAX_SEMAPHORES; ++i)
	{
		memset(&(semaphores[i]), 0, sizeof(struct Semaphore));
		semaphores[i].empty = 1;
		LIST_INIT(&(semaphores[i].env_queue));
	}
}


//========================
// [2] Allocate Semaphore:
//========================
//Allocates a new (empty) semaphore object from the "semaphores" array
//Return:
//	a) if succeed:
//		1. allocatedSemaphore (pointer to struct Semaphore) passed by reference
//		2. SempahoreObjectID (its index in the array) as a return parameter
//	b) E_NO_SEMAPHORE if the the array of semaphores is full (i.e. reaches "MAX_SEMAPHORES")
int allocate_semaphore_object(struct Semaphore **allocatedObject)
{
	int32 semaphoreObjectID = -1 ;
	for (int i = 0; i < MAX_SEMAPHORES; ++i)
	{
		if (semaphores[i].empty)
		{
			semaphoreObjectID = i;
			break;
		}
	}

	if (semaphoreObjectID == -1)
	{
		//try to double the size of the "semaphores" array
		if (USE_KHEAP == 1)
		{
			semaphores = (struct Semaphore*) krealloc(semaphores, 2*MAX_SEMAPHORES);
			if (semaphores == NULL)
			{
				*allocatedObject = NULL;
				return E_NO_SEMAPHORE;
			}
			else
			{
				semaphoreObjectID = MAX_SEMAPHORES;
				MAX_SEMAPHORES *= 2;
			}
		}
		else
		{
			*allocatedObject = NULL;
			return E_NO_SEMAPHORE;
		}
	}

	*allocatedObject = &(semaphores[semaphoreObjectID]);
	semaphores[semaphoreObjectID].empty = 0;

	return semaphoreObjectID;
}

//======================
// [3] Get Semaphore ID:
//======================
//Search for the given semaphore object in the "semaphores" array
//Return:
//	a) if found: SemaphoreObjectID (index of the semaphore object in the array)
//	b) else: E_SEMAPHORE_NOT_EXISTS
int get_semaphore_object_ID(int32 ownerID, char* name)
{
	int i=0;
	for(; i < MAX_SEMAPHORES; ++i)
	{
		if (semaphores[i].empty)
			continue;

		if(semaphores[i].ownerID == ownerID && strcmp(name, semaphores[i].name)==0)
		{
			return i;
		}
	}
	return E_SEMAPHORE_NOT_EXISTS;
}

//====================
// [4] Free Semaphore:
//====================
//delete the semaphore with the given ID from the "semaphores" array
//Return:
//	a) 0 if succeed
//	b) E_SEMAPHORE_NOT_EXISTS if the semaphore is not exists
int free_semaphore_object(uint32 semaphoreObjectID)
{
	if (semaphoreObjectID >= MAX_SEMAPHORES)
		return E_SEMAPHORE_NOT_EXISTS;

	memset(&(semaphores[semaphoreObjectID]), 0, sizeof(struct Semaphore));
	semaphores[semaphoreObjectID].empty = 1;
	LIST_INIT(&(semaphores[semaphoreObjectID].env_queue));

	return 0;
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//======================
// [1] Create Semaphore:
//======================
int createSemaphore(int32 ownerEnvID, char* semaphoreName, uint32 initialValue)
{
	//TODO: [PROJECT 2019 - MS1 - [4] Semaphore] CreateSemaphore

	int semaphoreID = get_semaphore_object_ID(ownerEnvID , semaphoreName);

	if(semaphoreID == E_SEMAPHORE_NOT_EXISTS)
	{
		struct Semaphore *newSemaphore;

		semaphoreID = allocate_semaphore_object(&newSemaphore);
		if(semaphoreID != E_NO_SEMAPHORE)
		{
//			const char* temp_semaphorName = semaphoreName ;
//			char* temp_semaphorName_array = semaphores[semaphoreID].name ;

			semaphores[semaphoreID].ownerID = ownerEnvID ;
//			strcpy(temp_semaphorName_array ,temp_semaphorName);
			strcpy(semaphores[semaphoreID].name ,semaphoreName);

			semaphores[semaphoreID].value = initialValue;

			return semaphoreID ;
		}
		else  return E_NO_SEMAPHORE;
	}
	else  return E_SEMAPHORE_EXISTS ;

}

//============
// [2] Wait():
//============
void waitSemaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT 2019 - MS1 - [4] Semaphore] WAIT

	struct Env* myenv = curenv; //The calling environment

	int semaphoreID = get_semaphore_object_ID(ownerEnvID , semaphoreName);

	semaphores[semaphoreID].value--;

	if(semaphores[semaphoreID].value < 0)
	{
		sched_remove_ready(myenv);
		enqueue(&(semaphores[semaphoreID].env_queue),myenv);
		myenv->env_status = ENV_BLOCKED ;

		curenv = NULL;
	}
	fos_scheduler();


}

//==============
// [3] Signal():
//==============
void signalSemaphore(int ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT 2019 - MS1 - [4] Semaphore] SIGNAL

	int semaphoreID = get_semaphore_object_ID(ownerEnvID , semaphoreName);

	semaphores[semaphoreID].value++;
	if(semaphores[semaphoreID].value <= 0)
	{
		struct Env *myenv = NULL ;
		myenv = dequeue(&(semaphores[semaphoreID].env_queue));
		sched_insert_ready(myenv);
		myenv->env_status = ENV_READY ;
	}
}
