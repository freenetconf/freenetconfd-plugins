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

struct module m;
char *ns = "xml:ns:yang:house-lockdown";

datastore_t root = {"root",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,1,0,0};


int create_store()
{
	return 0;
}

// get
int get(struct rpc_data *data)
{
	node_t *ro_root = data->in;
	char *ro_root_name = roxml_get_name(ro_root, NULL, 0);

	// client requested get all
	if (ro_root_name && !strcmp("get", ro_root_name)) {
		ds_get_all(root.child, data->out, data->get_config, 1);

		return RPC_DATA;
	}

	// client requested filtered get
	datastore_t *our_root = ds_find_child(&root, ro_root_name, NULL);
	ds_get_filtered(ro_root, our_root, data->out, data->get_config);

	return RPC_DATA;
}

// edit-config
int edit_config(struct rpc_data *data)
{
	return ds_edit_config(data->in, root.child);
}


int rpc_house_lockdown(struct rpc_data *data)
{
	printf("┌-------------------------------------------------------------------------┐\n"
		   "|                                                                         |\n"
		   "|   Putting house in the lockdown, no one will be able to go in or out.   |\n"
		   "|                                                                         |\n"
		   "└-------------------------------------------------------------------------┘\n");

	return RPC_DATA;
}


struct rpc_method rpc[] = {
	{"house-lockdown", rpc_house_lockdown},
};

struct module *init()
{
	create_store();

	m.get = get;
	m.edit_config = edit_config;
	m.rpcs = rpc;
	m.rpc_count = (sizeof(rpc) / sizeof(*(rpc))); // to be filled directly by code generator
	m.ns = ns;

	return &m;
}

void destroy()
{
	ds_free(root.child, 1);
	root.child = NULL;
}