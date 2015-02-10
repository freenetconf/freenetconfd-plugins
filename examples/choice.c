/*
 * Copyright (C) 2015 Sartura, Ltd.
 *
 * Author: Zvonimir Fras <zvonimir.fras@sartura.hr>
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
#include <errno.h>
#include <unistd.h>

__unused struct module *init();
__unused void destroy();

struct module m;

// namespace for yang plugin
char *ns = "urn:ietf:params:xml:ns:yang:choice";

// root node default setup
datastore_t root = DATASTORE_ROOT_DEFAULT;


/**
 * This examples demonstrates how you would implement choice statements from
 * YANG in freenetconfd.
 *
 * It uses create_child() callback to create children.
 * When writing create_child() callback, usually you would check the name of
 * your desired node, edit non-default properties and add callbacks.
 * This callback is needed whenever there's a chance any node with non-default
 * properties gets deleted, because freenetconfd doesn't have enough information
 * to recreate the node without it.
 * This is always the case with choices.
 */


static datastore_t *create_player_child(datastore_t *self, char *name, char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, ns, target_name, target_position);

	/**
	 * In order to clear all elements from other cases, as directed
	 * by YANG RFC, use ds_purge_choice_group()
	 *
	 * In a real-world situation, you might have more choice statements
	 * in same container, or at least other containers/leafs which don't
	 * belong to the choice statement.
	 *
	 * If that happens, you will want to call ds_purge_choice_group()
	 * inside of an if() before setting choice_group.
	 */
	ds_purge_choice_group(self, 1);

	if (!strcmp(name, "instrument"))
	{
		/**
		 * In case you have more choice statements in the same container,
		 * assign different choice_group to elements of each choice statement.
		 * This time we only have choice "choose", so we use just 1
		 */
		child->choice_group = 1;
	}
	else if (!strcmp(name, "electronic"))
	{
		child->choice_group = 1;
	}

	return child;
}

static int create_store()
{
	datastore_t *player= ds_add_child_create(&root, "player", NULL, ns, NULL, 0);
	player->create_child = create_player_child;

	/**
	 * Since all "instrument"'s and "electronic"'s children have only default values,
	 * and no getters or setters, we don't need to explicitly define how they will be
	 * created, freenetconfd will take care of that for us.
	 *
	 * In real-world situation, you will probably want to define getters and setters
	 * for all of the nodes and attach appropriate create_child() callbacks to their
	 * parents
	 */

	return 0;
}

__unused struct module *init()
{
	create_store();

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
