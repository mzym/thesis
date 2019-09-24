/*
 * _pargresql_memory_manager.c
 *
 * Copyright (c) 2012 Mikhail Zymbler, Constantin Pan, Alexey Koltakov
 */

#include <assert.h>
#include <stdio.h>
#include "par_inis/_pargresql_memory_manager.h"

typedef struct stack {
	int top;
	int data[BLOCKS_IN_SHMEM];
} stack_t;

typedef struct queue {
	int head;
	int tail;
	int data[BLOCKS_IN_SHMEM];
} queue_t;

typedef struct {
	sem_t semaphore;
	int node;
	int nodescount;
	
	sem_t nempty;
	sem_t emptySem;
	stack_t emptyblocks;
	
	sem_t nunproc;
	sem_t unprocSem;
	queue_t unprocblocks;

	shmblock_t blocks[BLOCKS_IN_SHMEM];
} shmem_t;

#define init_stack(s)											\
	for((s).top = BLOCKS_IN_SHMEM-1; (s).top >= 0; (s).top--)	\
		((s).data[(s).top]) = -1

#define push(s, e)												\
	(s).top++;													\
	((s).data[(s).top]) = (e)

#define pop(s, e)												\
	(e) = ((s).data[(s).top]);									\
	((s).data[(s).top]) = -1;									\
	(s).top--

#define init_queue(q)											\
	for((q).head = 0; (q).head < BLOCKS_IN_SHMEM; (q).head++)	\
		((q).data[(q).head]) = -1;								\
	(q).head = 0;												\
	(q).tail = 0

#define enqueue(q, e)											\
	((q).data[(q).tail]) = (e);									\
	((q).tail) = ((q).tail + 1) % BLOCKS_IN_SHMEM

#define dequeue(q, e)											\
	(e) = (q).data[(q).head];									\
	(q).data[(q).head] = -1;									\
	(q).head = ((q).head + 1) % BLOCKS_IN_SHMEM

#define queue_count(q)											\
	( (q).data[(q).tail] == -1 ) ?								\
		(( (q).head <= (q).tail ) ?								\
				( (q).tail - (q).head )							\
				: ( (q).tail - (q).head + BLOCKS_IN_SHMEM ))	\
		: BLOCKS_IN_SHMEM

static shmem_t *memptr; 		/* указатель на разделяемую память */
static queue_t curblocks;


/*
 * Данная функция создает и открывает новый объект разделяемой памяти.
 * Аргумент name содержит имя объекта разделяемой памяти.
 * Аргумент node содержит номер текущего узла.
 * Аргумент nodescount содержит общее количество узлов.
 */
extern void CreateSHMObject(const char *shmName, int node, int nodescount)
{
	int fd, res, i;

	shm_unlink(shmName);
	fd = shm_open(shmName, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	assert(fd > -1);
	res = ftruncate(fd, sizeof(shmem_t));
	assert(res == 0);
	memptr = mmap(NULL, sizeof(shmem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	res = sem_init(&memptr->semaphore, 1, 0);
	assert(res == 0);
	res = sem_init(&memptr->nunproc, 1, 0);
	assert(res == 0);
	res = sem_init(&memptr->unprocSem, 1, 1);
	assert(res == 0);
	res = sem_init(&memptr->nempty, 1, BLOCKS_IN_SHMEM);
	assert(res == 0);
	res = sem_init(&memptr->emptySem, 1, 1);
	assert(res == 0);

	memptr->node = node;
	memptr->nodescount = nodescount;
	init_stack(memptr->emptyblocks);
	init_queue(memptr->unprocblocks);

	for (i = 0; i < BLOCKS_IN_SHMEM; i++) {
		res = sem_init(&memptr->blocks[i].state, 1 , UNPROCESSED);
		assert(res == 0);
		push(memptr->emptyblocks, i);
	}
	
	res = sem_post(&memptr->semaphore);
	assert(res == 0);
	init_queue(curblocks);
}

/*
 * Данная функция открывает уже существующий объект разделяемой памяти.
 * Параметр name содержит имя объекта разделяемой памяти.
 * В параметре node возвращается номер текущего узла.
 * В параметре nodescount возвращается общее количество узлов.
 */
extern void OpenSHMObject(const char *shmName, int *node, int *nodescount)
{
	int fd, res;

	fd = shm_open(shmName, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	assert(fd > -1);
	memptr = mmap(NULL, sizeof(shmem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	res = sem_wait(&memptr->semaphore);
	assert(res == 0);
	*node = memptr->node;
	*nodescount = memptr->nodescount;
	res = sem_post(&memptr->semaphore);
	assert(res == 0);
	init_queue(curblocks);
}

/*
 * Данная функция удаляет уже существующий объект разделяемой памяти.
 * Аргумент name содержит имя объекта разделяемой памяти.
 */
extern void RemoveSHMObject(const char *name)
{
	int res;

	res = shm_unlink(name);
	assert(res == 0);
}

/*
 * Данная функция возвращает укзатель на блок разделяемой памяти по его номеру.
 */
extern shmblock_t *GetBlock(int blockNumber)
{
	shmblock_t *block;

	assert(blockNumber >= 0 && blockNumber < BLOCKS_IN_SHMEM);
	block = &memptr->blocks[blockNumber];

	return block;
}

/*
 * Данная функция возвращает номер свободного блока разделяемой памяти.
 */
extern int GetEmptyBlockNumber()
{
	int blockNumber, res;
	
	res = sem_wait(&memptr->nempty);
	assert(res == 0);
	res = sem_wait(&memptr->emptySem);
	assert(res == 0);
	
	pop(memptr->emptyblocks, blockNumber);
	assert(blockNumber >= 0 && blockNumber < BLOCKS_IN_SHMEM);
	
	res = sem_post(&memptr->emptySem);
	assert(res == 0);
	
	return blockNumber;
}

/*
 * Данная функция помечает блок разделяемой памяти c заданным номером как свободный.
 */
extern void SetEmptyBlockNumber(int blockNumber)
{
	int i, res;
	
	assert(blockNumber >= 0 && blockNumber < BLOCKS_IN_SHMEM);
	res = sem_wait(&memptr->emptySem);
	assert(res == 0);
	
	push(memptr->emptyblocks, blockNumber);
	
	res = sem_post(&memptr->emptySem);
	assert(res == 0);
	res = sem_post(&memptr->nempty);
	assert(res == 0);
}

/*
 * Данная функция возвращает количество необработанных блоков разделяемой памяти.
 */
extern int UnprocBlocksCount()
{
	int count, res;

	res = sem_getvalue(&memptr->nunproc, &count);
	assert(res == 0);

	return count;
}

/*
 * Данная функция возвращает номер необработанного блока разделяемой памяти.
 */
extern int GetUnprocBlockNumber()
{
	int blockNumber, res;
	
	res = sem_wait(&memptr->nunproc);
	assert(res == 0);
	res = sem_wait(&memptr->unprocSem);
	assert(res == 0);
	
	dequeue(memptr->unprocblocks, blockNumber);
	assert(blockNumber >= 0 && blockNumber < BLOCKS_IN_SHMEM);
	
	res = sem_post(&memptr->unprocSem);
	assert(res == 0);

	return blockNumber;
}

/*
 * Данная функция помечает блок разделяемой памяти c заданным номером как необработанный.
 */
extern void SetUnprocBlockNumber(int blockNumber)
{
	int res;
	
	assert(blockNumber >= 0 && blockNumber < BLOCKS_IN_SHMEM);
	res = sem_wait(&memptr->unprocSem);
	assert(res == 0);
	
	enqueue(memptr->unprocblocks, blockNumber);
	
	res = sem_post(&memptr->unprocSem);
	assert(res == 0);
	res = sem_post(&memptr->nunproc);
	assert(res == 0);
}

/*
 * Данная функция возвращает количество используемых блоков разделяемой памяти.
 */
extern int CurrentBlocksCount()
{
	return queue_count(curblocks);
}

/*
 * Данная функция возвращает номер используемого блока разделяемой памяти.
 */
extern int GetCurrentBlockNumber()
{
	int blockNumber;

	dequeue(curblocks, blockNumber);
	assert(blockNumber >= 0 && blockNumber < BLOCKS_IN_SHMEM);

	return blockNumber;
}

/*
 * Данная функция помечает блок разделяемой памяти c заданным номером как используемый.
 */
extern void SetCurrentBlockNumber(int blockNumber)
{
	assert(blockNumber >= 0 && blockNumber < BLOCKS_IN_SHMEM);
	enqueue(curblocks, blockNumber);
}
