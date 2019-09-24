/*-------------------------------------------------------------------------
 * nodeScatter.c
 *	  One of the four suboperations of the Exchange
 *
 * Copyright (c) 2011, Mikhail Zymbler, Constantin S. Pan
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include <stdio.h>

#include "access/par_tupack.h"
#include "executor/executor.h"
#include "executor/par_nodeScatter.h"
#include "par_inis/_pargresql_library.h" // FIXME: INIS naming

// FIXME: use the UUID actually (instead of int)
int fragfunc(int size, TupleTableSlot *slot, int fragattr)
{
	if (fragattr == 0)
	{
		// FIXME: this is a hack to allow constant zero exchange
		// (the root exchange), without the struct having
		// "fragfunc" property. NEED TO FIX THIS ASAP.
		return 0;
	}
	uint32 val = DatumGetUInt32(slot->tts_values[fragattr - 1]);
	return val % size;
}

void printhex(char *name, char *buf, int len)
{
	int i;
	printf("%s {\n", name);
	for (i = 0; i < len; i++)
	{
		printf("%02X ", (unsigned int)buf[i]);
	}
	printf("}\n", name);
}


/* ----------------------------------------------------------------
 *		ExecScatter
 * ----------------------------------------------------------------
 */
TupleTableSlot *				/* return: a tuple or NULL */
ExecScatter(ScatterState *node)
{
	//ScanDirection direction;
	//TupleTableSlot *slot

	if (node->isSending)
	{
		int flag;
		_pargresql_Test(&node->request, &flag); // FIXME: INIS naming
		if (flag)
		{
			node->buf = NULL;
			node->isSending = 0;
			node->status = PAR_OK;
			return NULL;
		}
		else
		{
			node->status = PAR_WAIT;
			return NULL;
		}
	}
	else
	{
		int dst, rank, size;
		rank = _pargresql_GetNode(); // FIXME: INIS naming
		size = _pargresql_GetNodesCount(); // FIXME: INIS naming
		uuid_t port = ((Scatter*)node->ps.plan)->port; // FIXME: use the UUID actually (instead of int)
		if (TupIsNull(node->upstreamTuple))
		{ // send EOF tuple
			for (dst = 0; dst < size; dst++)
			{
				char *zero = "\0\0";
//				printhex("send zero", zero, 2);
				_pargresql_ISend(dst, port, 2, zero, &node->request); // FIXME: is NULL a valid message/request
			}
			node->isSending = 0;
			node->status = PAR_OK;
			return NULL;
		}
		else
		{
			int fragattr;
			fragattr = node->ps.plan->fragattr;
			dst = fragfunc(size, node->upstreamTuple, fragattr);
			StringInfoData sid = par_tupack(node->upstreamTuple);
			node->buf = sid.data;
//			printhex("msg", sid.data, sid.len);
			_pargresql_ISend(dst, port, sid.len, sid.data, &node->request);
			node->isSending = 1;
			node->status = PAR_WAIT;
			return NULL;
		}
	}
	return NULL;
}

/* ----------------------------------------------------------------
 *		ExecInitScatter
 *
 *		This initializes the scatter node state structures and
 *		the node's subplan.
 * ----------------------------------------------------------------
 */
ScatterState *
ExecInitScatter (Scatter *node, EState *estate, int eflags)
{
	ScatterState *scatterstate;
	Plan	   *childPlan;

	/* check for unsupported flags */
	Assert(!(eflags)); // FIXME: no flags supported

	/*
	 * create state structure
	 */
	scatterstate = makeNode(ScatterState);
	scatterstate->ps.plan = (Plan *) node;
	scatterstate->ps.state = estate;

	scatterstate->status = PAR_OK;
	scatterstate->isSending = 0;
	scatterstate->upstreamTuple = NULL;

	/*
	 * Tuple table initialization
	 */
	ExecInitResultTupleSlot(estate, &scatterstate->ps);

	/*
	 * scatter nodes do no projections, so initialize projection info for this
	 * node appropriately
	 */
	ExecAssignResultTypeFromTL(&scatterstate->ps);
	scatterstate->ps.ps_ProjInfo = NULL;

	return scatterstate;
}

int
ExecCountSlotsScatter(Scatter *node)
{
	return ExecCountSlotsNode(outerPlan(node)) +
		ExecCountSlotsNode(innerPlan(node)) +
		1 /* our slot */;
}

/* ----------------------------------------------------------------
 *		ExecEndScatter
 *
 *		This shuts down the subplan and frees resources allocated
 *		to this node.
 * ----------------------------------------------------------------
 */
void
ExecEndScatter(ScatterState *node)
{
}


void
ExecReScanScatter(ScatterState *node, ExprContext *exprCtxt)
{
	node->buf = NULL;
	node->status = PAR_OK;
	node->isSending = 0;
}
