/* (c) Mikhail Zymbler */

#ifdef PAR_PLANMAIN_H
#error "par_planmain.h is already included elsewhere"
#else // PAR_PLANMAIN_H is undefined
#define PAR_PLANMAIN_H

// These methods are for internal use in make_exchange.
extern Split *make_split(Plan *lefttree, Plan *righttree);
extern Merge *make_merge(Plan *lefttree, Plan *righttree);
extern Scatter *make_scatter(Plan *sibling, int port, int fragattr);
extern Gather *make_gather(Plan *sibling, int port);

// These methods are for internal use in make_stream.
extern Balance *make_balance(Plan *subtree, int port);
extern Mirror *make_mirror(Plan *sibling, int partition, int port);

// Use these in Parallelizer.
extern Plan *make_exchange(Plan *plan, int port, int fragattr);
extern Plan *make_stream(SeqScan *seqscan, int port);

#endif
