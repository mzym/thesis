/*-------------------------------------------------------------------------
 *
 * par_parallelizer.c
 *	  The query parallelizer subsystem of PargreSQL.
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "nodes/plannodes.h"
#include "nodes/relation.h"
#include "nodes/nodeFuncs.h"
//#include "catalog/pg_operator.h"
//#include "executor/executor.h"
//#include "executor/nodeAgg.h"
//#include "miscadmin.h"
//#include "nodes/makefuncs.h"
//#include "optimizer/clauses.h"
//#include "optimizer/cost.h"
//#include "optimizer/pathnode.h"
//#include "optimizer/paths.h"
#include "optimizer/planmain.h"
//#include "optimizer/planner.h"
//#include "optimizer/prep.h"
//#include "optimizer/subselect.h"
//#include "optimizer/tlist.h"
//#include "optimizer/var.h"
#include "par_parallelizer/par_parallelizer.h"
#include "par_inis/_pargresql_library.h"
//#ifdef OPTIMIZER_DEBUG
//#include "nodes/print.h"
//#endif
//#include "parser/parse_expr.h"
#include "parser/parse_node.h"
#include "parser/parse_oper.h"
#include "parser/parse_relation.h"
//#include "parser/parsetree.h"
//#include "utils/lsyscache.h"
//#include "utils/syscache.h"

/*****************************************************************************
 *
 *	   Query parallelizer entry point
 *
 *****************************************************************************/

void print_targetlist(List *targetlist);
void print_opexprlist(List *opexprlist);
void print_restrictlist(List *restrictlist);
bool isleft(Plan *parent, Plan *child);
int joinattr(Plan *parent, Plan *child);
void print_nodetag_recursive(Plan *plan);
Plan *insert_exchange_here_or_deeper(Plan *plan, int *port, int joinattr);
Plan *par_Parallelize_recursive(Plan *plan, int *port);
void add_qual_attr_mod_nodes_equals_me(Plan *plan, int attr, int nodes, int me);
Oid get_query_result_relid(Query *query);
AttrNumber get_atno_in_relid_by_atname(Oid relid, const char* atname);
char *get_relid_fragattr(Oid relid);
int get_relid_fragatno(Oid relid);

void print_targetlist(List *targetlist)
{
	ListCell *lc;
	foreach(lc, targetlist)
	{
		TargetEntry *tle = (TargetEntry*)lfirst(lc);
		printf("%d-%s,", tle->resno, tle->resname);
	}
}

void print_opexprlist(List *opexprlist)
{
	ListCell *lc;
	foreach(lc, opexprlist)
	{
		OpExpr *ox;
		char *opname;

		ox = (OpExpr*)lfirst(lc);
		Assert(IsA(ox, OpExpr));
		opname = get_opname(ox->opno);
		printf("%d %s %d,",
			//nodeTag(linitial(ox->args)),
			((Var*)linitial(ox->args))->varattno,
			opname,
			((Var*)lsecond(ox->args))->varattno
			//nodeTag(lsecond(ox->args))
		);
		pfree(opname);
	}
}

void print_restrictlist(List *restrictlist)
{
	ListCell *lc;
	foreach(lc, restrictlist)
	{
		Node *n = (Node*)lfirst(lc);
		printf("%d,", nodeTag(n));
	}
}

bool isleft(Plan *parent, Plan *child)
{
	return parent->lefttree == child;
}

int joinattr(Plan *parent, Plan *child)
{
	List *oxlist;
	OpExpr *ox;
	char *opname;

	if (IsA(parent, MergeJoin)) { oxlist = ((MergeJoin*)parent)->mergeclauses; }
	else if (IsA(parent, NestLoop)) { oxlist = ((Join*)parent)->joinqual; }
	else if (IsA(parent, HashJoin)) { oxlist = ((HashJoin*)parent)->hashclauses; }
	else { Assert(false); }

	Assert(!IsNull(oxlist));
	ox = (OpExpr*)linitial(oxlist);
	Assert(IsA(ox, OpExpr));
	opname = get_opname(ox->opno);
	Assert(opname[0] == '=');
	pfree(opname);

	if (isleft(parent, child))
	{
		return ((Var*)linitial(ox->args))->varattno;
	}
	else
	{
		return ((Var*)lsecond(ox->args))->varattno;
	}
}

void print_nodetag_recursive(Plan *plan)
{
	if (plan == NULL) {
		return;
	}

	if (IsA(plan, MergeJoin))
	{
		printf("mjoin<%d = %d",
			joinattr(plan, plan->lefttree),
			joinattr(plan, plan->righttree)
		);
//		print_opexprlist(((MergeJoin*)plan)->mergeclauses);
		printf(">");
	}
	else if (IsA(plan, NestLoop))
	{
		if (((Join*)plan)->joinqual != NULL) {
			printf("njoin<%d = %d",
				joinattr(plan, plan->lefttree),
				joinattr(plan, plan->righttree)
			);
		} else {
			printf("njoin<NULL, WTF?");
		}
//		print_opexprlist(((Join*)plan)->joinqual);
		printf(">");
	}
	else if (IsA(plan, HashJoin))
	{
		printf("hjoin<%d = %d",
			joinattr(plan, plan->lefttree),
			joinattr(plan, plan->righttree)
		);
//		print_opexprlist(((HashJoin*)plan)->hashclauses);
		printf(">");
	}
	else if (IsA(plan, SeqScan))
	{
		printf("seqscan<FIXME");
		printf(">");
	}
	else if (IsA(plan, Hash))
	{
		printf("hash<FIXME");
		printf(">");
	}
	else if (IsA(plan, Material))
	{
		printf("mat<FIXME");
		printf(">");
	}
	else if (IsA(plan, Sort))
	{
		printf("sort<FIXME");
		printf(">");
	}
	else if (IsA(plan, Merge))
	{
		int port;
		int fragattr;
		port = ((Gather*)plan->lefttree)->port;
		fragattr = plan->righttree->righttree->fragattr;
		printf("exchange<port%d, frag%d>", port, fragattr);
	}
	else if (IsA(plan, Result))
	{
		printf("result");
	}
	else if (IsA(plan, Agg))
	{
		int i;
		char *astratname = "unknown";
		Agg *agg = (Agg*)plan;
		if (agg->aggstrategy == AGG_PLAIN) {
			astratname = "plain";
		} else if (agg->aggstrategy == AGG_SORTED) {
			astratname = "sorted";
		} else if (agg->aggstrategy == AGG_HASHED) {
			astratname = "hash";
		}
		printf("agg<type=%s", astratname);
		for (i = 0; i < agg->numCols; i++)
		{
			printf(", %d", agg->grpColIdx[i]);
		}
		printf(">");
	}
	else
	{
		printf("%d", plan->type);
	}

	if (IsA(plan, Merge))
	{
		printf("(");
		print_nodetag_recursive(plan->righttree->lefttree);
		printf(")");
	}
	else
	{
		printf("(");
		print_nodetag_recursive(plan->lefttree);
		printf(",");
		print_nodetag_recursive(plan->righttree);
		printf(")");
	}
}

// Inserts an Exchange above the specified node, or below it
// if the node is Material, Sort, Hash, or other node that requires
// the Exchange to be put underneath instead.
Plan *insert_exchange_here_or_deeper(Plan *plan, int *port, int joinattr)
{
	// FIXME: needs to be a recursive function, in order to
	// cover the case of "Material(Sort(Hash(...)))".
	if (IsA(plan, Material) || IsA(plan, Sort) || IsA(plan, Hash))
	{
		Plan *oldleft = plan->lefttree;
		Plan *newleft = make_exchange(
			plan->lefttree,
			(*port)++,
			joinattr
		);
		//plan->lefttree = make_exchange(
		//	plan->lefttree,
		//	(*port)++,
		//	joinattr
		//);
		if ((unsigned long)newleft > 0xf0000000000000) {
			printf("FUCK, newleft == %lx\n", (unsigned long)newleft);
		}
		if ((unsigned long)oldleft > 0xf0000000000000) {
			printf("FUCK, oldleftleft == %lx\n", (unsigned long)oldleft);
		}
		if (plan->lefttree != oldleft) {
			printf("FUCK, newleft != oldleft\n");
		}
		printf("newleft == %lx\n", (unsigned long)newleft);
		plan->lefttree = newleft;
		return plan;
	}
	else
	{
		return make_exchange(
			plan,
			(*port)++,
			joinattr
		);
	}
}

Plan *par_Parallelize_recursive(Plan *plan, int *port)
{
	if (plan == NULL)
	{
		return NULL;
	}

	// recursion
	plan->lefttree = par_Parallelize_recursive(plan->lefttree, port);
	plan->righttree = par_Parallelize_recursive(plan->righttree, port);
	// FIXME: consider Append nodes, which do not have "left" or "right" subtrees.

	if (
		IsA(plan, MergeJoin)
		|| (IsA(plan, NestLoop) && (((Join*)plan)->joinqual != NULL))
		|| IsA(plan, HashJoin)
	)
	{
		plan->lefttree = insert_exchange_here_or_deeper(
			plan->lefttree,
			port,
			joinattr(plan, plan->lefttree)
		);
		plan->righttree = insert_exchange_here_or_deeper(
			plan->righttree,
			port,
			joinattr(plan, plan->righttree)
		);
	}
	else if (IsA(plan, Agg))
	{
		if (((Agg*)plan)->aggstrategy == AGG_PLAIN)
		{
			// No grouping, bad :(
			// Insert that constant ZERO exchange :P
			// FIXME: Implement some kind of two-stage aggregation,
			// cause the current variant is incorrect!
			plan->lefttree = insert_exchange_here_or_deeper(
				plan->lefttree,
				port,
				0
				// Send all tuples to one node, hence a global group will occur on 0-node.
				// But the other nodes will still generate some "zero-sum" tuple.
			);
		}
		else
		{
			// Grouping by some attribute - so we just
			// exchange by this attribute first.
			plan->lefttree = insert_exchange_here_or_deeper(
				plan->lefttree,
				port,
				((Agg*)plan)->grpColIdx[0]
				// Use the first group-by attribute
				// as the exchange attribute, in order
				// to get the correct results of aggregation.
			);
		}
	}
	// FIXME: streams not implemented yet
	/*
	if (IsA(plan->lefttree, SeqScan))
	{
		plan->lefttree = make_stream((SeqScan*)plan->lefttree, (*port)++);
	}
	if (IsA(plan->righttree, SeqScan))
	{
		plan->righttree = make_stream((SeqScan*)plan->righttree, (*port)++);
	}
	*/
	return plan;
}

/*
 * Adds the following expression to the plan qual list:
 * tuple[attr] % nodes == me
 */
void add_qual_attr_mod_nodes_equals_me(Plan *plan, int attr, int nodes, int me)
{
	Expr *mod_nodes, *is_mine, *attr_expr;
	Const *nodes_const, *me_const;
	TargetEntry *te;

	// Make constant expressions
	nodes_const = make_const(NULL, makeInteger(nodes), -1);
	me_const = make_const(NULL, makeInteger(me), -1);

	// Get the expression for tuple[attr]
	te = list_nth(plan->targetlist, attr - 1);
	Assert(IsA(te, TargetEntry));
	attr_expr = te->expr;

	// Get the expression for % operator
	mod_nodes = make_op(NULL, list_make1(makeString("%")), (Node*)attr_expr, (Node*)nodes_const, -1);
	Assert(IsA(mod_nodes, OpExpr));

	// Get the expression for == operator
	is_mine = make_op(NULL, list_make1(makeString("=")), (Node*)mod_nodes, (Node*)me_const, -1);
	Assert(IsA(is_mine, OpExpr));

	// Append the "==" expression to plan's qual list
	if (IsA(plan, Result)) {
		// For some reason, Result operator ignores plain 'qual'
		// and only respects its own resconstantqual :(
		((Result*)plan)->resconstantqual = (Node*)lappend(plan->qual, is_mine);
	} else {
		plan->qual = lappend(plan->qual, is_mine);
	}
}

/*
 * Returns the Oid of the result relation for an INSERT, UPDATE or DELETE query.
 */
Oid get_query_result_relid(Query *query)
{
	RangeTblEntry *rte;
	rte = (RangeTblEntry*)list_nth(query->rtable, query->resultRelation - 1);
	Assert(IsEqual(rte->rtekind, RTE_RELATION));
	return rte->relid;
}

/*
 * Returns the attribute's number (starting from 1) in relation by the
 * attribute's name.
 */
AttrNumber get_atno_in_relid_by_atname(Oid relid, const char* atname)
{
	Relation relation;
	int result;

	relation = RelationIdGetRelation(relid);
	result = attnameAttNum(relation, atname, /*sysColOk*/TRUE);
	RelationClose(relation);

	return result;
}

/*
 * This returns 'fragattr' reloption of the relation with given Oid.
 * You should free the allocated string yourself with pfree().
 */
char *get_relid_fragattr(Oid relid)
{
	Relation relation;
	char *fragattr, *result;
	StdRdOptions *opts;

	relation = RelationIdGetRelation(relid);

	opts = ((StdRdOptions*)relation->rd_options);
	fragattr = ((char*)opts) + (long)opts->fragattr; // just opts->fragattr would only be an offset
	result = palloc(strlen(fragattr) + 1);
	strcpy(result, fragattr);

	RelationClose(relation);

	return result;
}

/*
 * This returns fragattr number (starting from 1)
 * in the relation with given Oid.
 */
int get_relid_fragatno(Oid relid)
{
	char *fragattr;
	int result;

	fragattr = get_relid_fragattr(relid);
	result = get_atno_in_relid_by_atname(relid, fragattr);
	pfree(fragattr);

	return result;
}

Plan *par_Parallelize(Plan *plan, Query *query)
{
	int port = 0;
	int fragatno; // partitioning attribute number
	int nodes, me;

	printf("Be quiet, parallelizer is working...\n");
	print_nodetag_recursive(plan);	
	printf("\n");

	switch (query->commandType) {
		case CMD_SELECT:
			// Aggregate all the result tuples on node-0
			printf("This is a SELECT.\n");
			plan = par_Parallelize_recursive(plan, &port);
			printf("Exchange nodes inserted into the plan.\n");
			plan = insert_exchange_here_or_deeper(plan, &port, 0);
			printf("A special exchange (ex.func == 0) inserted into the root.\n");
			break;
		case CMD_INSERT:
			// No exchanges needed (FIXME: really?), just filter the
			// tuples of the root node, so they wouldn't get inserted
			// into every partition.
			fragatno = get_relid_fragatno(get_query_result_relid(query));
			printf("This is an INSERT into a table where fragattr is set to %d.\n", fragatno);
			nodes = _pargresql_GetNodesCount();
			me = _pargresql_GetNode();
			add_qual_attr_mod_nodes_equals_me(plan, fragatno, nodes, me);
			printf("Added qual 'tuple[%d] %% %d == %d'.\n", fragatno, nodes, me);
			break;
		case CMD_UPDATE:
			// FIXME: 
			fragatno = get_relid_fragatno(get_query_result_relid(query));
			printf("This is an UPDATE of a table where fragattr is set to %d.\n", fragatno);
			plan = par_Parallelize_recursive(plan, &port);
			printf("Exchange nodes inserted into the plan.\n");
			plan = insert_exchange_here_or_deeper(plan, &port, fragatno);
			printf("A special exchange (ex.func == tuple[%d] %% nodes) inserted into the root.\n", fragatno);
			break;
		case CMD_DELETE:
			printf("This is a DELETE. Doing nothing...\n");
			break;
		default:
			printf("This is not a SELECT, INSERT, UPDATE or DELETE command. Doing nothing...\n");
	}

	printf("Parallelizer finished his dark ritual.\n");
	print_nodetag_recursive(plan);	
	printf("\n");

	return plan;
}
