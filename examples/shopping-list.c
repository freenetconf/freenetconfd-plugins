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

__unused struct module *init();
__unused void destroy();

struct module m;
char *ns = "urn:ietf:params:xml:ns:yang:shopping-list";

datastore_t root = DATASTORE_ROOT_DEFAULT;

static int create_store()
{
	// shopping-list
	datastore_t *shopping_list = ds_add_child_create(&root, "shopping-list", NULL, ns, NULL, 0);

	ds_add_child_create(shopping_list, "item", "milk", NULL, NULL, 0)->is_list = 1;
	ds_add_child_create(shopping_list, "item", "cookies", NULL, NULL, 0)->is_list = 1;
	ds_add_child_create(shopping_list, "item", "bread", NULL, NULL, 0)->is_list = 1;
	ds_add_child_create(shopping_list, "item", "butter", NULL, NULL, 0)->is_list = 1;
	ds_add_child_create(shopping_list, "item", "yogurt", NULL, NULL, 0)->is_list = 1;

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
