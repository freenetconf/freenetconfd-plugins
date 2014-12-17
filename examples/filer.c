/*
 * Copyright (C) 2014 Cisco Systems, Inc.
 *
 * Author: Zvonimir Fras <zvonimir.fras@sartura.hr>
 * Author: Luka Perkov <luka.perkov@sartura.hr>
 *
 * This is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * You should have received a copy of the GNU General Public License along with
 * this project. If not, see <http://www.gnu.org/licenses/>.
 */

#include <freenetconfd/plugin.h>
#include <freenetconfd/datastore.h>
#include <freenetconfd/freenetconfd.h>

#include <stdio.h>
#include <unistd.h>

__unused struct module *init();
__unused void destroy();

struct module m;
char *ns = "urn:ietf:params:xml:ns:yang:filer";

static char *filename = "/tmp/freenetconfd-filer";

datastore_t root = {"root",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,1,0,0};

static int set_data(char *data)
{
	FILE *fp = fopen(filename, "w");

	if (!fp)
	{
		DEBUG("ERROR: Cannot open \"%s\" for writing\n", filename);
		return -1;
	}

	fprintf(fp, "%s\n", data);
	fclose(fp);
	return 0;
}

/*
 * using datastore is not required for this simple example
 * datastore can be used in case you want to use the same get function
 * for several different nodes, for example in lists
 */
static char *get_data(datastore_t *datastore)
{
	FILE *fp = fopen(filename, "r+");

	size_t n = 1024;
	char *buffer = malloc(n); // freenetconfd will handle freeing the return value

	if (!fp)
		return strcpy(buffer, "No data set yet");

	getline(&buffer, &n, fp);

	return buffer;
}

static int del_file(struct datastore *self, void *data)
{
	unlink(filename);
	return 0;
}

static datastore_t *create_data_node(datastore_t *self, char *name, char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *data = ds_add_child_create(self, name, value, ns, target_name, target_position);
	data->get = get_data;
	data->set = set_data;
	data->del = del_file;

	return data;
}

static int create_store()
{
	// filer
	datastore_t *filer = ds_add_child_create(&root, "filer", NULL, ns, NULL, 0);
	filer->create_child = create_data_node;
	filer->create_child(filer, "data", "original", NULL, NULL, 0);

	return 0;
}

__unused struct module *init()
{
	create_store();

	m.rpcs = NULL;
	m.rpc_count = 0;
	m.ns = ns;
	m.datastore = &root;

	return &m;
}

__unused void destroy()
{
	ds_free(root.child, 1);
	root.child = NULL;
}
