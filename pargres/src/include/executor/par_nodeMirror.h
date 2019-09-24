#ifndef PAR_NODEBALANCE_H
#define PAR_NODEBALANCE_H

#include "nodes/execnodes.h"

extern int	ExecCountSlotsMirror(Mirror *node);
extern MirrorState *ExecInitMirror(Mirror *node, EState *estate, int eflags);
extern TupleTableSlot *ExecMirror(MirrorState *node);
extern void ExecEndMirror(MirrorState *node);
extern void ExecReScanMirror(MirrorState *node, ExprContext *exprCtxt);

#endif   /* PAR_NODEBALANCE_H */
