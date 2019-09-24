/*
 * _pargresql_library.c
 *
 * Copyright (c) 2012 Mikhail Zymbler, Constantin Pan, Alexey Koltakov
 */

#include <assert.h>
#include <stdio.h>
#include "par_inis/_pargresql_memory_manager.h"
#include "par_inis/_pargresql_library.h"

static int node;	/* номер текущего узла */
static int nodescount;	/* общее количество узлов */


/*
 * Данная функция открывает доступ к разделяемой памяти.
 * Никакие другие функции библиотеки обменов сообщениями не могут быть вызваны
 * до вызова данной функции.
 */
extern void _pargresql_InitLib()
{
	OpenSHMObject(SHMEMNAME, &node, &nodescount);
}

/*
 * Данная функция помещает в свободный блок разделяемой памяти
 * сообщение buf размера size. Возврат из функции гарантирует,
 * что сообщение помещено в разделяемую память, но не гарантирует,
 * что сообщение отправлено процессу-получателю.
 * Параметр dst содержит адрес процесса-получателя(номер узла).
 * Параметр port позволяет отличать, какому оператору exchange 
 * в каком из текущих сеансов отправляется сообщение.
 * Параметр request позволяет определить текущее состояние сообщения.
 */
extern void _pargresql_ISend(int dst, uuid_t port, int size, void *buf, _pargresql_request_t *request)
{
	shmblock_t *block;
	int blockNumber;
	
	assert(size <= MAX_MESSAGE_SIZE);
	blockNumber = GetEmptyBlockNumber();
	block = GetBlock(blockNumber);

	block->node = dst;
	block->port = port;
	block->msgType = TO_SEND;
	block->msgSize = size;
	memcpy(block->msg, buf, size);

	SetUnprocBlockNumber(blockNumber);
	request->buf = NULL;
	request->blockNumber = blockNumber;
}

/*
 * Данная функция определяет размер пришедшего сообщения.
 * Если сообщение пришло, в параметре size возвращается его размер,
 * а в параметре flag - 1, в противном случае в параметре flag возвращается 0.
 * Параметр dst содержит адрес процесса-получателя(номер узла).
 * Параметр port позволяет отличить, какому оператору exchange
 * в каком из текущих сеансов отправляется сообщение.
 */
extern void _pargresql_IProbe(int dst, uuid_t port, int *flag, int *size)
{
	shmblock_t *block;
	int blockNumber, res;

	blockNumber = GetEmptyBlockNumber();
	block = GetBlock(blockNumber);

	block->node = dst;
	block->port = port;
	block->msgType = TO_PROBE;
	block->msgSize = -1;

	SetUnprocBlockNumber(blockNumber);
	res = sem_wait(&block->state);
	assert(res == 0);

	if (block->msgSize > -1) {
		*flag = 1;
		*size = block->msgSize;
	} else {
		*flag = 0;
	}

	block->msgSize = 0;
	SetEmptyBlockNumber(blockNumber);
}

/*
 * Данная функция помещает в свободный блок разделяемой памяти
 * запрос на получение сообщения в буфер buf размера size. Возврат из функции
 * гарантирует, что запрос был помещен в разделяемую память,
 * но не гарантирует, что сообщение получено и размещено в буфере buf.
 * Параметр src содержит адрес процесса-отправителя(номер узла).
 * Параметр port позволяет отличать, какому оператору exchange 
 * в каком из текущих сеансов отправляется сообщение.
 * Параметр request позволяет определить текущее состояние сообщения.
 */
extern void _pargresql_IRecv(int src, uuid_t port, int size, void *buf, _pargresql_request_t *request)
{
	shmblock_t *block;
	int blockNumber;

	assert(size <= MAX_MESSAGE_SIZE);	
	blockNumber = GetEmptyBlockNumber();
	block = GetBlock(blockNumber);
	
	block->node = src;
	block->port = port;
	block->msgType = TO_RECV;
	block->msgSize = size;

	SetUnprocBlockNumber(blockNumber);
	request->buf = buf;
	request->blockNumber = blockNumber;
}

/*
 * Данная функция выполняет проверку состояния сообщения, ассоциированного
 * в идентификатором request. Если соответствующее сообщение обработано,
 * в параметре flag возвращается 1, иначе - 0.
 */
extern void _pargresql_Test(_pargresql_request_t *request, int *flag)
{
	shmblock_t *block;
	int state, res;

	assert(request->blockNumber >= 0 && request->blockNumber < BLOCKS_IN_SHMEM);
	block = GetBlock(request->blockNumber);
	res = sem_getvalue(&block->state, &state);
	assert(res == 0);
	
	if (state == PROCESSED) {
		if (block->msgType == TO_RECV) {
			assert(request->buf != NULL);
			memcpy(request->buf, block->msg, block->msgSize);
		} else
			assert(block->msgType == TO_SEND);

		memset(block->msg, 0, MAX_MESSAGE_SIZE);
		block->msgSize = 0;
		res = sem_wait(&block->state);
		assert(res == 0);
		SetEmptyBlockNumber(request->blockNumber);
		request->buf = NULL;
		request->blockNumber = -1;
		*flag = 1;
	} else {
		*flag = 0;
	}
}

/*
 * Данная функция возвращает номер текущего узла.
 */
extern int _pargresql_GetNode(void)
{
	return node;
}

/*
 * Данная функция возвращает общее количество узлов.
 */
extern int _pargresql_GetNodesCount(void)
{
	return nodescount;
}

/*
 * Данная функция завершает работу с разделяемой памятью.
 * Все последующие обращения к любым функциям библиотеки запрещены.
 * К моменту вызова данной функции некоторым процессом все действия,
 * требующие его участия в обмене сообщениями, должны быть завершены.
 */
extern void _pargresql_FinalizeLib(void)
{
	shmblock_t *block;
	int blockNumber;

	blockNumber = GetEmptyBlockNumber();
	block = GetBlock(blockNumber);
	block->msgType = TO_CLOSE;
	SetUnprocBlockNumber(blockNumber);
}
