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
char *ns = "urn:ietf:params:xml:ns:yang:black-book";

datastore_t root = DATASTORE_ROOT_DEFAULT;

static int create_store()
{
	// black-book
	datastore_t *black_book = ds_add_child_create(&root, "black-book", NULL, ns, NULL, 0);

	datastore_t *person = ds_add_child_create(black_book, "person", NULL, NULL, NULL, 0);
	person->is_list = 1;
	ds_add_child_create(person, "name", "Monika", NULL, NULL, 0)->is_key = 1;
	ds_add_child_create(person, "phone", "09876543", NULL, NULL, 0);

	person = ds_add_child_create(black_book, "person", NULL, NULL, NULL, 0);
	person->is_list = 1;
	ds_add_child_create(person, "name", "Scarlet", NULL, NULL, 0)->is_key = 1;
	ds_add_child_create(person, "phone", "09876544", NULL, NULL, 0);
	
	person = ds_add_child_create(black_book, "person", NULL, NULL, NULL, 0);
	person->is_list = 1;
	ds_add_child_create(person, "name", "Tina", NULL, NULL, 0)->is_key = 1;
	ds_add_child_create(person, "phone", "09876545", NULL, NULL, 0);

	person = ds_add_child_create(black_book, "person", NULL, NULL, NULL, 0);
	person->is_list = 1;
	ds_add_child_create(person, "name", "Jessica", NULL, NULL, 0)->is_key = 1;
	ds_add_child_create(person, "phone", "09876546", NULL, NULL, 0);

	person = ds_add_child_create(black_book, "person", NULL, NULL, NULL, 0);
	person->is_list = 1;
	ds_add_child_create(person, "name", "Jennifer", NULL, NULL, 0)->is_key = 1;
	ds_add_child_create(person, "phone", "09876547", NULL, NULL, 0);

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
