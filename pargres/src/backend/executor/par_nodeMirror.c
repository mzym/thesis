/*-------------------------------------------------------------------------
 *
 * nodeMirror.c
 *	  Routines to handle load balancing in PargreSQL
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "executor/executor.h"
#include "executor/par_nodeMirror.h"
#include "executor/nodeLimit.h"

TupleTableSlot *				/* return: a tuple or NULL */
ExecMirror(MirrorState *node)
{
	if (((LimitState*)node)->lstate == LIMIT_INITIAL) { // first run
		((LimitState*)node)->offset = /*FIXME request task*/ 0 - ((LimitState*)node)->offset;
	}
	return ExecLimit((LimitState*)node); // super()
}

MirrorState *
ExecInitMirror(Mirror *node, EState *estate, int eflags)
{
	MirrorState *mirrorstate;
	Plan	   *outerPlan;

	/* check for unsupported flags */
	Assert(!(eflags & EXEC_FLAG_MARK));

	/*
	 * create state structure
	 */
	mirrorstate = makeNode(MirrorState);
	((LimitState*)mirrorstate)->ps.plan = (Plan *) node;
	((LimitState*)mirrorstate)->ps.state = estate;

	((LimitState*)mirrorstate)->lstate = LIMIT_INITIAL;

	/*
	 * Miscellaneous initialization
	 *
	 * Mirror nodes never call ExecQual or ExecProject, but they need an
	 * exprcontext anyway to evaluate the limit/offset parameters in.
	 */
	ExecAssignExprContext(estate, &((LimitState*)mirrorstate)->ps);

	/*
	 * initialize child expressions
	 */
	((LimitState*)mirrorstate)->limitOffset = ExecInitExpr((Expr *) (((LimitState*)node)->limitOffset), (PlanState *) mirrorstate);
	((LimitState*)mirrorstate)->limitCount = ExecInitExpr((Expr *) (((LimitState*)node)->limitCount), (PlanState *) mirrorstate);

#define LIMIT_NSLOTS 1

	/*
	 * Tuple table initialization (XXX not actually used...)
	 */
	ExecInitResultTupleSlot(estate, &((LimitState*)mirrorstate)->ps);

	/*
	 * then initialize outer plan
	 */
	outerPlan = outerPlan(node);
	outerPlanState(mirrorstate) = ExecInitNode(outerPlan, estate, eflags);

	/*
	 * limit nodes do no projections, so initialize projection info for this
	 * node appropriately
	 */
	ExecAssignResultTypeFromTL(&((LimitState*)mirrorstate)->ps);
	((LimitState*)mirrorstate)->ps.ps_ProjInfo = NULL;

	return mirrorstate;
}

int
ExecCountSlotsMirror(Mirror *node)
{
	return ExecCountSlotsNode(outerPlan(node)) +
		ExecCountSlotsNode(innerPlan(node)) +
		LIMIT_NSLOTS;
}

/* ----------------------------------------------------------------
 *		ExecEndMirror
 *
 *		This shuts down the subplan and frees resources allocated
 *		to this node.
 * ----------------------------------------------------------------
 */
void
ExecEndMirror(MirrorState *node)
{
	ExecFreeExprContext(&((LimitState*)node)->ps);
	ExecEndNode(outerPlanState(node));
}


void
ExecReScanMirror(MirrorState *node, ExprContext *exprCtxt)
{
	ExecReScanLimit((LimitState*)node, exprCtxt);
}
