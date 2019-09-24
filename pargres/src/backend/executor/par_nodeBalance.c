/*-------------------------------------------------------------------------
 *
 * nodeBalance.c
 *	  Routines to handle load balancing in PargreSQL
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "executor/executor.h"
#include "executor/par_nodeBalance.h"
#include "executor/nodeLimit.h"

#define max(x, y) ((x) > (y) ? (x) : (y))

TupleTableSlot *				/* return: a tuple or NULL */
ExecBalance(BalanceState *node)
{
	TupleTableSlot *slot;
	LimitState *lstate = (LimitState*)node;

	if (node->done) {
		return NULL;
	}

	if (/*FIXME task requested*/true) {
		if (lstate->noCount) {
			// 1. Cannot assign the tuples that we've already loaded
			// 2. Cannot assign the tuples, that are not there at the other machine
			lstate->count = max(lstate->position, /*FIXME skip*/0);
			lstate->noCount = false;
			// FIXME send(node->count);
		} else {
			// FIXME send(-1);
		}
	}

	slot = ExecLimit((LimitState*)node); // super()

	if (TupIsNull(slot)) {
		node->done = true;
		// FIXME scatter the "-1"
	}

	return slot;
}

BalanceState *
ExecInitBalance(Balance *node, EState *estate, int eflags)
{
	BalanceState *balancestate;
	Plan	   *outerPlan;

	/* check for unsupported flags */
	Assert(!(eflags & EXEC_FLAG_MARK));

	/*
	 * create state structure
	 */
	balancestate = makeNode(BalanceState);
	((LimitState*)balancestate)->ps.plan = (Plan *) node;
	((LimitState*)balancestate)->ps.state = estate;

	((LimitState*)balancestate)->lstate = LIMIT_INITIAL;
	balancestate->done = false;

	/*
	 * Miscellaneous initialization
	 *
	 * Balance nodes never call ExecQual or ExecProject, but they need an
	 * exprcontext anyway to evaluate the limit/offset parameters in.
	 */
	ExecAssignExprContext(estate, &((LimitState*)balancestate)->ps);

	/*
	 * initialize child expressions
	 */
	((LimitState*)balancestate)->limitOffset = ExecInitExpr((Expr *) (((LimitState*)node)->limitOffset), (PlanState *) balancestate);
	((LimitState*)balancestate)->limitCount = ExecInitExpr((Expr *) (((LimitState*)node)->limitCount), (PlanState *) balancestate);

#define LIMIT_NSLOTS 1

	/*
	 * Tuple table initialization (XXX not actually used...)
	 */
	ExecInitResultTupleSlot(estate, &((LimitState*)balancestate)->ps);

	/*
	 * then initialize outer plan
	 */
	outerPlan = outerPlan(node);
	outerPlanState(balancestate) = ExecInitNode(outerPlan, estate, eflags);

	/*
	 * limit nodes do no projections, so initialize projection info for this
	 * node appropriately
	 */
	ExecAssignResultTypeFromTL(&((LimitState*)balancestate)->ps);
	((LimitState*)balancestate)->ps.ps_ProjInfo = NULL;

	return balancestate;
}

int
ExecCountSlotsBalance(Balance *node)
{
	return ExecCountSlotsNode(outerPlan(node)) +
		ExecCountSlotsNode(innerPlan(node)) +
		LIMIT_NSLOTS;
}

/* ----------------------------------------------------------------
 *		ExecEndBalance
 *
 *		This shuts down the subplan and frees resources allocated
 *		to this node.
 * ----------------------------------------------------------------
 */
void
ExecEndBalance(BalanceState *node)
{
	ExecFreeExprContext(&((LimitState*)node)->ps);
	ExecEndNode(outerPlanState(node));
}


void
ExecReScanBalance(BalanceState *node, ExprContext *exprCtxt)
{
	ExecReScanLimit((LimitState*)node, exprCtxt);
}
