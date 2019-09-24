/*
 * _pargresql_memory_manager.h
 *
 * Copyright (c) 2012 Mikhail Zymbler, Constantin Pan, Alexey Koltakov
 */

#ifndef _PARGRESQL_MEMORY_MANAGER_H_
#define _PARGRESQL_MEMORY_MANAGER_H_

#include <fcntl.h>		/* for nonblocking */
#include <semaphore.h>	/* Posix semaphores */
#include <string.h>
#include <sys/mman.h>	/* Posix shared memory */
#include <sys/stat.h>	/* for S_xxx file mode constants */
#include <unistd.h>

#define SHMEMNAME			"/mem0"
#define BLOCKS_IN_SHMEM		2000
#define MAX_MESSAGE_SIZE	16777
#define UNPROCESSED			0
#define PROCESSED			1

typedef int uuid_t;
typedef int node_t;

typedef enum {
	TO_SEND = 0,
	TO_RECV,
	TO_PROBE,
	TO_CLOSE
} messagetype_t;

typedef char message_t[MAX_MESSAGE_SIZE];

typedef struct {
	sem_t state;
	uuid_t port;
	node_t node;
	messagetype_t msgType;
	message_t msg;
	int msgSize;
} shmblock_t;

/*
 * Данная функция создает и открывает новый объект разделяемой памяти.
 * Аргумент name содержит имя объекта разделяемой памяти.
 * Аргумент node содержит номер текущего узла.
 * Аргумент nodescount содержит общее количество узлов.
 */
extern void CreateSHMObject(const char *name, int node, int nodescount);

/*
 * Данная функция открывает уже существующий объект разделяемой памяти.
 * Параметр name содержит имя объекта разделяемой памяти.
 * В параметре node возвращается номер текущего узла.
 * В параметре nodescount возвращается общее количество узлов.
 */
extern void OpenSHMObject(const char *name, int *node, int *nodescount);

/*
 * Данная функция удаляет уже существующий объект разделяемой памяти.
 * Аргумент name содержит имя объекта разделяемой памяти.
 */
extern void RemoveSHMObject(const char *name);

/*
 * Данная функция возвращает укзатель на блок разделяемой памяти по его номеру.
 */
extern shmblock_t *GetBlock(int blockNumber);

/*
 * Данная функция возвращает номер свободного блока разделяемой памяти.
 */
extern int GetEmptyBlockNumber();

/*
 * Данная функция помечает блок разделяемой памяти c заданным номером как свободный.
 */
extern void SetEmptyBlockNumber(int blockNumber);

/*
 * Данная функция возвращает количество необработанных блоков разделяемой памяти.
 */
extern int UnprocBlocksCount();

/*
 * Данная функция возвращает номер необработанного блока разделяемой памяти.
 */
extern int GetUnprocBlockNumber();

/*
 * Данная функция помечает блок разделяемой памяти c заданным номером как необработанный.
 */
extern void SetUnprocBlockNumber(int blockNumber);

/*
 * Данная функция возвращает количество используемых блоков разделяемой памяти.
 */
extern int CurrentBlocksCount();

/*
 * Данная функция возвращает номер используемого блока разделяемой памяти.
 */
extern int GetCurrentBlockNumber();

/*
 * Данная функция помечает блок разделяемой памяти c заданным номером как используемый.
 */
extern void SetCurrentBlockNumber(int blockNumber);

#endif /* _PARGRESQL_MEMORY_MANAGER_H_ */
