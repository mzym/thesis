#ifdef PAR_CREATEPLAN_C
#error "par_createplan.c is already included elsewhere"
#else // PAR_CREATEPLAN_C is undefined
#define PAR_CREATEPLAN_C

Split *make_split(Plan *lefttree, Plan *righttree)
{
	Split	*node = makeNode(Split);
	Plan	*plan = &node->plan;

	copy_plan_costsize(plan, lefttree);
	// FIXME: don't we need to alter the cost estimations?

	plan->targetlist = lefttree->targetlist;
	plan->qual = NIL;
	plan->lefttree = lefttree;
	plan->righttree = righttree;

	return node;
}

Merge *make_merge(Plan *lefttree, Plan *righttree)
{
	Merge	*node = makeNode(Merge);

	if ((unsigned long)node > 0xf00000000000000) {
		printf("FUCK, node == %lx\n", (unsigned long)node);
	}

	Plan	*plan = &node->plan;

	copy_plan_costsize(plan, righttree);
	// FIXME: don't we need to alter the cost estimations?

	plan->targetlist = righttree->targetlist;
	plan->qual = NIL;
	plan->lefttree = lefttree;
	plan->righttree = righttree;

	return node;
}

/*
 * sibling has to be the other child of the Split of the
 * same Exchange "metanode". It's used because Scatter
 * is a nullary operation.
 */
Scatter *make_scatter(Plan *sibling, int port, int fragattr)
{
	Scatter	*node = makeNode(Scatter);
	Plan	*plan = &node->plan;

	copy_plan_costsize(plan, sibling);
	// FIXME: don't we need to alter the cost estimations?

	plan->targetlist = sibling->targetlist;
	plan->qual = NIL;
	plan->lefttree = NULL;
	plan->righttree = NULL;
	node->port = port;
	plan->fragattr = fragattr;

	return node;
}

/*
 * sibling has to be the Split node of the same Exchange "metanode".
 * It's used because Gather is a nullary operation.
 */
Gather *make_gather(Plan *sibling, int port)
{
	Gather	*node = makeNode(Gather);
	Plan	*plan = &node->plan;

	copy_plan_costsize(plan, sibling);
	// FIXME: don't we need to alter the cost estimations?

	plan->targetlist = sibling->targetlist;
	plan->qual = NIL;
	plan->lefttree = NULL;
	plan->righttree = NULL;
	node->port = port;

	return node;
}

Plan *make_exchange(Plan *plan, int port, int fragattr)
{
	Plan *split, *merge, *scatter, *gather;
	scatter = (Plan*)make_scatter(plan, port, fragattr);
	split = (Plan*)make_split(plan, scatter);
	gather = (Plan*)make_gather(split, port);
	merge = (Plan*)make_merge(gather, split);
	if ((unsigned long)merge > 0xf00000000000000) {
		printf("FUCK, merge == %lx\n", (unsigned long)merge);
	} else {
		printf("OK, merge == %lx\n", (unsigned long)merge);
	}
	return merge;
}

Balance *make_balance(Plan *subtree, int port)
{
	return NULL;
}

Mirror *make_mirror(Plan *sibling, int partition, int port)
{
	Mirror	   *node = makeNode(Mirror);
	Plan	   *plan = (Plan*)node;

	copy_plan_costsize(plan, sibling);

	plan->targetlist = sibling->targetlist; // FIXME use child instead of sibling
	plan->qual = NIL;
	plan->lefttree = NULL; // FIXME create seqscan
	plan->righttree = NULL;

	((Limit*)node)->limitOffset = NULL; // FIXME zero offset
	((Limit*)node)->limitCount = NULL; // FIXME unlimited count

	return node;
}

Plan *make_stream(SeqScan *seqscan, int port)
{
	Plan *plan;
	List *subplans = NIL;
	int i;
	Plan *balance;

	balance = make_balance((Plan*)seqscan, port);
	lappend(subplans, balance);

	for (/*FIXME each mirror of the relation that seqscan scans*/i = 0; i < 1; i++) {
		lappend(subplans, /*FIXME use make_mirror*/ NULL);
	}

	plan = (Plan*)make_append(subplans, false, ((Plan*)seqscan)->targetlist);
	plan->qual = ((Plan*)seqscan)->qual; // put filters above the Append node
	((Plan*)seqscan)->qual = NIL; // clear the filters of the Scan node

	return plan;
}

#endif

