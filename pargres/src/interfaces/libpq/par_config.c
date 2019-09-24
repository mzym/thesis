/*-----------------------------------------------------------------------------
 *
 * par_config.c
 * 	This file contains implementation for functions used by par_libpq-fe to
 * 	load its configuration.
 *
 * 2010, Constantin S. Pan
 *
 *-----------------------------------------------------------------------------
 */

#include "par_config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void par_config_resize(par_config *conf, const int new_cap)
{
	int i;
	for (i = new_cap; i < conf->capacity; i++)
	{
		free(conf->conninfo[i]);
	}
	conf->conninfo = realloc(conf->conninfo, sizeof(char*) * new_cap);
	for (i = conf->capacity; i < new_cap; i++)
	{
		conf->conninfo[i] = malloc(sizeof(char) * INIT_CONNINFO_LEN);
	}
	conf->capacity = new_cap;
}

int par_config_readline(par_config *conf, FILE *f)
{
	if (conf->nodes_count == conf->capacity)
	{
		par_config_resize(conf, conf->capacity * 2);
	}
	char *current = conf->conninfo[conf->nodes_count];
	fgets(current, INIT_CONNINFO_LEN, f);

	char *c;
	if (c = strchr(current, '\n')) // remove trailing '\n'
	{
		*c = '\0';
	}
	if (c = strchr(current, '#')) // remove comments
	{
		*c = '\0';
	}

	if (current[0] != '\0')
	{
		// if not empty
		conf->nodes_count++;
	}
}

par_config *par_config_load(const char *filename)
{
	par_config *conf = malloc(sizeof(par_config));
	conf->nodes_count = 0;
	conf->capacity = 0;
	conf->conninfo = NULL;
	par_config_resize(conf, INIT_CAPACITY);

	FILE *f = fopen(filename, "r");
	if (f == NULL)
	{
		puts("Cannot open PargreSQL config file");
	}
	else
	{
		while (!feof(f))
		{
			par_config_readline(conf, f);
		}
		fclose(f);
	}
	return conf;
}

void par_config_unload(par_config *conf)
{
	par_config_resize(conf, 0); // FIXME: WTF this causes SEGFAULT!?
	free(conf);
}

#ifdef TEST
int main()
{
	par_config *c = par_config_load("par_config.h");
	int i;
	for (i = 0; i < c->nodes_count; i++)
	{
		puts(c->conninfo[i]);
	}
	return 0;
}
#endif

