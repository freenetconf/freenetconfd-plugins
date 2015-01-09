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

int rpc_house_lockdown(struct rpc_data *data);

__unused struct module *init();
__unused void destroy();

struct module m;
char *ns = "urn:ietf:params:xml:ns:yang:house-lockdown";

datastore_t root = DATASTORE_ROOT_DEFAULT;

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

__unused struct module *init()
{
	m.rpcs = rpc;
	m.rpc_count = (sizeof(rpc) / sizeof(*(rpc))); // to be filled directly by code generator
	m.ns = ns;
	m.datastore = &root;

	return &m;
}

__unused void destroy()
{
	ds_free(root.child, 1);
	root.child = NULL;
}
