/*
 * Copyright (C) 2015 Sartura, Ltd.
 *
 * Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
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
char *ns = "urn:ietf:params:xml:ns:yang:example";

datastore_t root = DATASTORE_ROOT_DEFAULT;

static int create_store()
{
	datastore_t *hello_store = ds_add_child_create(&root, "hello", NULL, ns, NULL, 0);

	datastore_t *hello = ds_add_child_create(hello_store, "world", NULL, NULL, NULL, 0);
	hello_store->is_list = 1;
	ds_add_child_create(hello, "foo", "1", NULL, NULL, 0);
	ds_add_child_create(hello, "bar", "2", NULL, NULL, 0);
	ds_add_child_create(hello, "toto", "3", NULL, NULL, 0);

	datastore_t *hello_2 = ds_add_child_create(hello_store, "world_2", NULL, NULL, NULL, 0);
	hello_store->is_list = 1;
	ds_add_child_create(hello_2, "foo", "4", NULL, NULL, 0);
	ds_add_child_create(hello_2, "bar", "5", NULL, NULL, 0);
	ds_add_child_create(hello_2, "toto", "6", NULL, NULL, 0);

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
