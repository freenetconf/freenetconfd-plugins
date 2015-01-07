/*
 * Copyright (C) 2014 Cisco Systems, Inc.
 *
 * Author: Mario Halambek <mario.halambek@sartura.hr>
 *
 * This is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 * You should have received a copy of the GNU General Public License along
 * with this project. If not, see <http://www.gnu.org/licenses/>.
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
char *ns = "urn:ietf:params:xml:ns:yang:vehicle";


static char *filename_name = "/tmp/freenetconfd-vehicle_name";
static char *filename_rims = "/tmp/freenetconfd-vehicle_rims";
static char *filename_color = "/tmp/freenetconfd-vehicle_color";
static char *filename_abs = "/tmp/freenetconfd-vehicle_abs";
static char *filename_ac = "/tmp/freenetconfd-vehicle_ac";

// root node default setup
datastore_t root = DATASTORE_ROOT_DEFAULT;


static int set_node_rims(char *data)
{
	FILE *fp = fopen(filename_rims, "w");

	if (!fp)
	{
		DEBUG("ERROR: Cannot open \"%s\" for writing\n", filename_rims);
		return -1;
	}

	fprintf(fp, "%s\n", data);
	fclose(fp);

	return 0;
}

static int set_node_color(char *data)
{
	FILE *fp = fopen(filename_color, "w");

	if (!fp)
	{
		DEBUG("ERROR: Cannot open \"%s\" for writing\n", filename_color);
		return -1;
	}

	fprintf(fp, "%s\n", data);
	fclose(fp);

	return 0;
}

static int set_node_abs(char *data)
{
	FILE *fp = fopen(filename_abs, "w");

	if (!fp)
	{
		DEBUG("ERROR: Cannot open \"%s\" for writing\n", filename_abs);
		return -1;
	}

	fprintf(fp, "%s\n", data);
	fclose(fp);

	return 0;
}

static int set_node_ac(char *data)
{
	FILE *fp = fopen(filename_ac, "w");

	if (!fp)
	{
		DEBUG("ERROR: Cannot open \"%s\" for writing\n", filename_ac);
		return -1;
	}

	fprintf(fp, "%s\n", data);
	fclose(fp);

	return 0;
}

static int set_node_name(char *data)
{
	FILE *fp = fopen(filename_name, "w");

	if(!fp)
	{
		DEBUG("ERROR: Cannot open \"%s\" for writing\n", filename_name);
		return -1;
	}

	fprintf(fp, "%s\n", data);
	fclose(fp);

	return 0;
}

static char *get_node_name(datastore_t *datastore)
{
	FILE *fp = fopen(filename_name, "r");
	size_t n = 1024;
	char *buffer = malloc(n);

	if (!fp)
	{
		return strcpy(buffer, "No data yet.");
	}

	getline(&buffer, &n, fp);
	fclose(fp);

	return buffer;
}

static char *get_node_rims(datastore_t *datastore)
{
	FILE *fp = fopen(filename_rims, "r");
	int i=0;
	size_t n = 1024;

	char *buffer = malloc(n);

	if (!fp)
	{
		return strcpy(buffer, "No data yet.");
	}

	getline(&buffer, &n, fp);
	fclose(fp);

	return buffer;
}

static char *get_node_color(datastore_t *datastore)
{
	FILE *fp = fopen(filename_color, "r");
	int i;
	size_t n = 1024;

	char *buffer = malloc(n);

	if (!fp)
	{
		return strcpy(buffer, "No data yet.");
	}

	getline(&buffer, &n, fp);
	fclose(fp);

	return buffer;
}

static char *get_node_abs(datastore_t *datastore)
{
	FILE *fp = fopen(filename_abs, "r");
	int i=0;
	size_t n = 1024;

	char *buffer = malloc(n);

	if (!fp)
	{
		return strcpy(buffer, "No data yet.");
	}

	getline(&buffer, &n, fp);
	fclose(fp);

	return buffer;
}

static char *get_node_ac(datastore_t *datastore)
{
	FILE *fp = fopen(filename_ac, "r");
	int i=0;
	size_t n = 1024;

	char *buffer = malloc(n);

	if (!fp)
	{
		return strcpy(buffer, "No data yet.");
	}

	getline(&buffer, &n, fp);
	fclose(fp);

	return buffer;
}

static int del_node_name(struct datastore *self, void *data)
{
	unlink(filename_name);
	return 0;
}

static int del_node_rims(struct datastore *self, void *data)
{
	unlink(filename_rims);
	return 0;
}

static int del_node_color(struct datastore *self, void *data)
{
	unlink(filename_color);
	return 0;
}

static int del_node_abs(struct datastore *self, void *data)
{
	unlink(filename_abs);
	return 0;
}

static int del_node_ac(struct datastore *self, void *data)
{
	unlink(filename_ac);
	return 0;
}

static datastore_t *create_node(datastore_t *self, char *name, char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, ns, target_name, target_position);

	if (!strcmp(name, "name"))
	{
		child->set = set_node_name;
		child->get = get_node_name;
		child->del = del_node_name;
	}
	else if (!strcmp(name, "rims"))
	{
		child->set = set_node_rims;
		child->get = get_node_rims;
		child->del = del_node_rims;
	}
	else if (!strcmp(name, "color"))
	{
		child->set = set_node_color;
		child->get = get_node_color;
		child->del = del_node_color;
	}
	else if (!strcmp(name, "abs"))
	{
		child->set = set_node_abs;
		child->get = get_node_abs;
		child->del = del_node_abs;
	}
	else if (!strcmp(name, "ac"))
	{
		child->set = set_node_ac;
		child->get = get_node_ac;
		child->del = del_node_ac;
	}

	return child;
}

static int create_store()
{
	// vehicle
	datastore_t *vehicle = ds_add_child_create(&root, "vehicle", NULL, ns, NULL, 0);
	vehicle->create_child = create_node;

	vehicle->create_child(vehicle, "name", "Mazda", NULL, NULL, 0);
	vehicle->create_child(vehicle, "rims", "O.Z.", NULL, NULL, 0);
	vehicle->create_child(vehicle, "color", "Red", NULL, NULL, 0);
	vehicle->create_child(vehicle, "abs", "true", NULL, NULL, 0);
	vehicle->create_child(vehicle, "ac", "Wind", NULL, NULL, 0);

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
