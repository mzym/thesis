-- Calculates the Kernigal-Lin gains for the vertices.
-- 'in_edges_tb' - the edge table name (a int, b int, w float)
-- 'in_partition_tb' - partitioning table name (a int, p int)
-- 'in_result_tb' - result table name [will be created/overwritten as (a int, p int, gain float)]
create or replace function calc_gains(in_edges_tb text, in_partition_tb text, in_result_tb text) returns void as $$
declare
	qry text;
begin

	execute 'drop table if exists '||quote_ident(in_result_tb);
	execute 'create table '||quote_ident(in_result_tb)||' (a int, p int, gain float)';
	qry =
		'insert into '||quote_ident(in_result_tb)||' select
			part.a,
			part.p,
			sum(subgains.gain) as gain
		from
			'||quote_ident(in_partition_tb)||' as part
			left join (
				select
					e.a,
					e.b,
					case when ap.p = bp.p then -e.w else e.w end as gain
				from
					'||quote_ident(in_edges_tb)||' as e
					left join '||quote_ident(in_partition_tb)||' as ap
						on e.a = ap.a
					left join '||quote_ident(in_partition_tb)||' as bp
						on e.b = bp.a
			) as subgains
				on part.a = subgains.a or part.a = subgains.b
		group by part.a, part.p';
	-- raise notice 'qry = %', qry;
	execute qry;
end
$$ language plpgsql;

-- Refines the partitioning, swapping the vertices with maximum gain, while the gain is positive.
-- 'in_edges_tb' - the edge table name.
-- 'in_gains_tb' - the gains table name (a int, p int, gain float), created by calc_gains
create or replace function refine_gains(in_edges_tb text, in_gains_tb text) returns void as $$
declare
	v record;
	curpart int;
begin
	curpart = 0;
	loop
		execute 'select * from '||quote_ident(in_gains_tb)||' where p = '||curpart||' and gain = (select max(gain) from '||quote_ident(in_gains_tb)||' where p = '||curpart||')'
			into v;
		raise notice 'curpart = %', curpart;
		if v is not null and v.gain > 0 then
			raise notice 'v = %, %, %', v.a, v.p, v.gain;
			execute 'update '||quote_ident(in_gains_tb)||' as g
					set gain = gain + w * (case when p = '||v.p||' then 2 else -2 end)
				from (select case when a = '||v.a||' then b else a end, w from '||quote_ident(in_edges_tb)||' where b = '||v.a||' or a = '||v.a||') as neighbors
				where neighbors.a = g.a';
			execute 'update '||quote_ident(in_gains_tb)||'
					set gain = -gain, p = 1 - p where a = '||v.a;
		else
			raise notice 'not found';
			exit;
		end if;
		curpart = 1 - curpart;
	end loop;
end
$$ language plpgsql;

-- Propagates the partition marks to the vertices of finer level.
-- 'in_collapse_tb' - the collapsed edges table name.
-- 'in_partition_tb' - the coarse partitioning table name (a int, p int), usually the result of calc_gains + refine_gains.
-- 'in_result_tb' - the name for the result table (a int, p int)
create or replace function propagate_partitions(in_collapse_tb text, in_partition_tb text, in_result_tb text) returns void as $$
begin
	execute 'drop table if exists '||quote_ident(in_result_tb);
	execute 'create table '||quote_ident(in_result_tb)||' (a int, p int)';
	execute 'insert into '||quote_ident(in_result_tb)||'
			select a, p from '||quote_ident(in_partition_tb)||'
		union
			select coll.b, part.p from '||quote_ident(in_collapse_tb)||' as coll, '||quote_ident(in_partition_tb)||' as part where coll.a = part.a';
end
$$ language plpgsql;

-- Performs a whole single uncoarsening step
-- 'in_edges_tb' - the previous level (finer) edge table name
-- 'in_collapse_tb' - the collapsed edges table name
-- 'in_coarse_part_tb' - the coarse partitioning table name
-- 'in_fine_part_tb' - the result
create or replace function uncoarsen(in_edges_tb text, in_collapse_tb text, in_coarse_part_tb text, in_fine_part_tb text) returns void as $$
begin
	perform propagate_partitions(in_collapse_tb, in_coarse_part_tb, 'temp_part');
	perform calc_gains(in_edges_tb, 'temp_part', in_fine_part_tb);
	perform refine_gains(in_edges_tb, in_fine_part_tb);
end
$$ language plpgsql;

-- vim: syntax=sql
