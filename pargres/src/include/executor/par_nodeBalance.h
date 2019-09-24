#ifndef PAR_NODEBALANCE_H
#define PAR_NODEBALANCE_H

#include "nodes/execnodes.h"

extern int	ExecCountSlotsBalance(Balance *node);
extern BalanceState *ExecInitBalance(Balance *node, EState *estate, int eflags);
extern TupleTableSlot *ExecBalance(BalanceState *node);
extern void ExecEndBalance(BalanceState *node);
extern void ExecReScanBalance(BalanceState *node, ExprContext *exprCtxt);

#endif   /* PAR_NODEBALANCE_H */
