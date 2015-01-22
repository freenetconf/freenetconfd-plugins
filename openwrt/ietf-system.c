/*
 * Copyright (C) 2014 Cisco Systems, Inc.
 *
 * Author: Petar Koretic <petar.koretic@sartura.hr>
 * Author: Zvonimir Fras <zvonimir.fras@sartura.hr>
 * Author: Luka Perkov <luka.perkov@sartura.hr>
 *
 * freenetconfd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with freenetconfd. If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <uci.h>

#include "freenetconfd/freenetconfd.h"
#include "freenetconfd/datastore.h"
#include "freenetconfd/plugin.h"

int rpc_set_current_datetime(struct rpc_data *data);
int rpc_system_restart(struct rpc_data *data);
int rpc_system_shutdown(struct rpc_data *data);

__unused struct module *init();
__unused void destroy();

struct module m;
char *ns = "urn:ietf:params:xml:ns:yang:ietf-system";

datastore_t root = DATASTORE_ROOT_DEFAULT;

struct uci_context *c;
struct uci_ptr p;

// FIXME: this is only a prototype
static void generic_update(datastore_t *node)
{
	if (node) printf("Datastore node UPDATE\t%s: %s\n", node->name, node->value);
}

static int set_hostname(char *data)
{
	char cmd[] = "system.@system[0].hostname=", *full_cmd;
	struct uci_context *c;
	struct uci_ptr ptr;

	full_cmd = malloc(strlen(data) + strlen(cmd) + 1);

	strcpy(full_cmd, cmd);
	strcat(full_cmd, data);

	c = uci_alloc_context();

	memset(&ptr, 0, sizeof(ptr));

	if (uci_lookup_ptr(c, &ptr, full_cmd, true) != UCI_OK)
	{
		return -1;
	}

	uci_set(c, &ptr);
	uci_save(c, ptr.p);

	uci_free_context(c);

	return 0;
}

static int set_location(char *data)
{
	char cmd[] = "system.@system[0].location=", *full_cmd;
	struct uci_context *c;
	struct uci_ptr ptr;

	full_cmd = malloc(strlen(data) + strlen(cmd) + 1);

	strcpy(full_cmd, cmd);
	strcat(full_cmd, data);

	c = uci_alloc_context();

	memset(&ptr, 0, sizeof(ptr));

	if (uci_lookup_ptr(c, &ptr, full_cmd, true) != UCI_OK)
	{
		return -1;
	}

	uci_set(c, &ptr);
	uci_save(c, ptr.p);

	uci_free_context(c);

	return 0;
}

static char *get_hostname(datastore_t *datastore)
{
	char *path, *buf;

	path = strdup("system.@system[0].hostname");

	if (uci_lookup_ptr(c, &p, path, true) != UCI_OK)
	{
		free(path);
		DEBUG("uci error\n");
		return NULL;
	}
	free(path);

	buf = malloc(sizeof(char) * strlen(p.o->v.string));
	strcpy(buf, p.o->v.string);

	return buf;
}

static char *get_timezone_location(datastore_t *datastore)
{
	FILE *fp = fopen("/etc/TZ", "r");
	size_t n = 1024;
	char *buffer = malloc(n);

	if (!fp)
	{
		return strcpy(buffer, "No data yet");
	}

	getline(&buffer, &n, fp);
	fclose(fp);

	return buffer;
}

static int set_timezone_location(char *data)
{
	FILE *fp = fopen("/etc/TZ", "w");

	if (!fp)
	{
		DEBUG("ERROR: Cannot open \"%s\" for writing\n", "/etc/TZ");
		return -1;
	}

	fprintf(fp, "%s\n", data);
	fclose(fp);

	return 0;
}


//put number of list element in data
static int set_server(char *data)
{
	printf("%s\n", data);
	return 0;
}

static char *get_current_datetime(datastore_t *datastore)
{
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime ( &rawtime );
	return asctime(timeinfo);
}

static char *get_location(datastore_t *datastore)
{
	char *path, *buf;

	path = strdup("system.@system[0].location");

	if (uci_lookup_ptr(c, &p, path, true) != UCI_OK)
	{
		free(path);
		DEBUG("uci error\n");
		return NULL;
	}
	free(path);

	buf = malloc(sizeof(char) * strlen(p.o->v.string));
	strcpy(buf, p.o->v.string);

	return buf;
}

static int create_store()
{
	// ietf-system
	datastore_t *system = ds_add_child_create(&root, "system", NULL, ns, NULL, 0);
	system->update = generic_update;

	datastore_t *location = ds_add_child_create(system, "location", "Zagreb", NULL,
	                        NULL, 0); // string
	location->get = get_location;
	location->set = set_location;

	location->update = generic_update;
	datastore_t *hostname = ds_add_child_create(system, "hostname", "OpenWrt", NULL,
	                        NULL, 0); // string
	hostname->get = get_hostname;
	hostname->set = set_hostname;
	hostname->update = generic_update;
	
	ds_add_child_create(system, "contact", "yes, please", NULL, NULL, 0); // string
	datastore_t *clock = ds_add_child_create(system, "clock", NULL, NULL, NULL, 0);
	datastore_t *ntp = ds_add_child_create(system, "ntp", NULL, NULL, NULL, 0);

	// clock
	datastore_t *timezone_location = ds_add_child_create(clock, "timezone-location",
	                                 "Europe/Zagreb", NULL, NULL, 0); // string
	timezone_location->get = get_timezone_location;
	timezone_location->set = set_timezone_location;
	timezone_location->update = generic_update;
	ds_add_child_create(clock, "timezone-utc-offset", "60", NULL, NULL, 0); // int16
	//TODO
	//does offset needs to written
	

	// ntp
	ds_add_child_create(ntp, "enabled", "false", NULL, NULL, 0); // bool

	//TODO
	//how to bind function to list element
	//information ti obrain index
	// server list
	for (int i = 1; i < 3; i++)
	{
		//server
		datastore_t *server = ds_add_child_create(ntp, "server", NULL, NULL, NULL, 0);
		server->set = set_server;
		server->is_list = 1;
		char server_name[BUFSIZ];
		snprintf(server_name, BUFSIZ, "server%d", i);
		datastore_t *name = ds_add_child_create(server, "name", server_name, NULL, NULL,
		                                        0); // string
		name->is_key = 1;

		datastore_t *association_type = ds_add_child_create(server, "association-type",
		                                NULL, NULL, NULL, 0);
		ds_set_is_config(association_type, 0, 0);
		datastore_t *iburst = ds_add_child_create(server, "iburst", "false", NULL, NULL,
		                      0);
		ds_set_is_config(iburst, 0, 0);
		datastore_t *prefer = ds_add_child_create(server, "prefer", "false", NULL, NULL,
		                      0);
		ds_set_is_config(prefer, 0, 0);

		// udp
		datastore_t *udp = ds_add_child_create(server, "udp", NULL, NULL, NULL, 0);
		ds_add_child_create(udp, "address", "127.0.0.1", NULL, NULL, 0); // inet-addr
		ds_add_child_create(udp, "port", "8088", NULL, NULL, 0); // int16
	}

	datastore_t *dns_resolver = ds_add_child_create(system, "dns-resolver", NULL,
	                            NULL, NULL, 0);
	ds_add_child_create(dns_resolver, "search", "localhost1", NULL, NULL,
	                    0)->is_list = 1;
	ds_add_child_create(dns_resolver, "search", "localhost2", NULL, NULL,
	                    0)->is_list = 1;

	for (int i = 1; i < 3; i++)
	{
		datastore_t *server = ds_add_child_create(dns_resolver, "server", NULL, NULL,
		                      NULL, 0);
		char server_name[BUFSIZ];
		snprintf(server_name, BUFSIZ, "server%d", i);
		datastore_t *name = ds_add_child_create(server, "name", server_name, NULL, NULL,
		                                        0); // string
		name->is_key = 1;

		// udp
		datastore_t *udp_and_tcp = ds_add_child_create(server, "udp-and-tcp", NULL,
		                           NULL, NULL, 0);
		ds_add_child_create(udp_and_tcp, "address", "127.0.0.1", NULL, NULL,
		                    0); // inet-addr
		ds_add_child_create(udp_and_tcp, "port", "8088", NULL, NULL, 0); // int16
	}

	datastore_t *options = ds_add_child_create(dns_resolver, "options", NULL, NULL,
	                       NULL, 0);
	ds_add_child_create(options, "timeout", "5", NULL, NULL, 0);
	ds_add_child_create(options, "attempts", "2", NULL, NULL, 0);


	// ietf-system-state
	datastore_t *system_state = ds_add_child_create(&root, "system-state", NULL, ns,
	                            NULL, 0);
	ds_set_is_config(system_state, 0, 0);

	datastore_t *platform = ds_add_child_create(system_state, "platform", NULL, ns,
	                        NULL, 0);

	ds_add_child_create(platform, "os-name", "The awesome Linux", ns, NULL, 0);
	ds_add_child_create(platform, "os-release", "Top noch", ns, NULL, 0);
	ds_add_child_create(platform, "os-version", "latest", ns, NULL, 0);
	ds_add_child_create(platform, "machine", "x86_64", ns, NULL, 0);

	datastore_t *clock_state = ds_add_child_create(system_state, "clock", NULL, ns,
	                           NULL, 0);

	datastore_t *current_datetime = ds_add_child_create(clock_state,
	                                "current_datetime", "now", ns, NULL, 0);
	current_datetime->get = get_current_datetime;

	datastore_t *boot_datetime = ds_add_child_create(clock_state, "boot-datetime",
	                             "before", ns, NULL, 0);
	//boot_datetime->get = get_boot_datetime;

	return 0;
}

// RPC
int rpc_set_current_datetime(struct rpc_data *data)
{
	/* TODO: use settimeofday */

	char cmd[BUFSIZ];
	char *date = roxml_get_content(roxml_get_chld(data->in, "current-datetime", 0),
	                               NULL, 0, NULL);

	snprintf(cmd, BUFSIZ, "date -s %s", date);

	if (system(cmd) == -1)
		return RPC_ERROR;

	DEBUG("rpc\n");

	roxml_add_node(data->out, 0, ROXML_ELM_NODE, "data", NULL);

	return RPC_DATA;
}

int rpc_system_restart(struct rpc_data *data)
{
	pid_t reboot_pid;

	if ( 0 == (reboot_pid = fork()) )
	{
		reboot(LINUX_REBOOT_CMD_CAD_OFF);
		exit(1); /* never reached if reboot cmd succeeds */
	}

	if (reboot_pid < 0)
		return RPC_ERROR;

	int reboot_status;
	waitpid(reboot_pid, &reboot_status, 0);

	if ( !WIFEXITED(reboot_status) || WEXITSTATUS(reboot_status) != 0)
		return RPC_ERROR;

	return RPC_DATA;
}

int rpc_system_shutdown(struct rpc_data *data)
{
	pid_t shutdown_pid;

	if ( 0 == (shutdown_pid = fork()) )
	{
		sync();
		reboot(LINUX_REBOOT_CMD_HALT);
		exit(1); /* never reached if reboot cmd succeeds */
	}

	if (shutdown_pid < 0)
		return RPC_ERROR;

	int shutdown_status;
	waitpid(shutdown_pid, &shutdown_status, 0);

	if ( !WIFEXITED(shutdown_status) || WEXITSTATUS(shutdown_status) != 0)
		return RPC_ERROR;

	return RPC_DATA;
}

struct rpc_method rpc[] =
{
	{"set-current-datetime", rpc_set_current_datetime},
	{"system-restart", rpc_system_restart},
	{"system-shutdown", rpc_system_shutdown},
};

__unused struct module *init()
{
	c = uci_alloc_context();
	if (!c)
		return NULL;

	create_store();

	m.rpcs = rpc;
	m.rpc_count = (sizeof(rpc) / sizeof(*
	                                    (rpc))); // to be filled directly by code generator
	m.ns = ns;
	m.datastore = &root;

	return &m;
}

__unused void destroy()
{
	uci_free_context(c);

	ds_free(root.child, 1);
	root.child = NULL;
}
