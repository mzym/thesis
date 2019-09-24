/*
 * Routines to pack and unpack tuples for their
 * transmission between PargreSQL backends.
 *
 * (c) Mikhail Zymbler
 */

#ifndef PAR_TUPACK_H
#define PAR_TUPACK_H

#include "fmgr.h"
#include "lib/stringinfo.h"
#include "executor/tuptable.h"

extern StringInfoData par_tupack(TupleTableSlot *slot);
extern void par_tunpack(StringInfoData buf, TupleTableSlot *slot);

#endif /* PAR_TUPACK_H */
