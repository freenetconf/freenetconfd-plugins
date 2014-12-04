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
char *ns = "xml:ns:yang:sandwich";

datastore_t root = {"root",NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,1,0,0};

int set_name(char *name)
{
	printf("Name set to: %s\n", name);
	return 0;
}

int create_store()
{
	// sandwich
	datastore_t *sandwich = ds_add_child_create(&root, "sandwich", NULL, ns, NULL, 0);
	datastore_t *name = ds_add_child_create(sandwich, "name", "original", NULL, NULL, 0);
	name->set = set_name;
	ds_add_child_create(sandwich, "butter", "true", NULL, NULL, 0);
	ds_add_child_create(sandwich, "meat", "salami", NULL, NULL, 0);
	ds_add_child_create(sandwich, "cheese", "gouda", NULL, NULL, 0);
	ds_add_child_create(sandwich, "salad", "green", NULL, NULL, 0);
	return 0;
}

struct module *init()
{
	create_store();

	m.rpcs = NULL;
	m.rpc_count = 0;
	m.ns = ns;
	m.datastore = &root;

	return &m;
}

void destroy()
{
	ds_free(root.child, 1);
	root.child = NULL;
}
