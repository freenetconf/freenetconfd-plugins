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
#include <time.h>
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

static int set_uci(char *cmd, char *data)
{
	char *full_cmd;
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
	free(full_cmd);
	free(cmd);

	return 0;
}

static int set_hostname(datastore_t *self, char *data)
{
	char cmd[] = "system.@system[0].hostname=";
	return set_uci(cmd, data);
}

static int set_location(datastore_t *self, char *data)
{
	char cmd[] = "system.@system[0].location=";
	return set_uci(cmd, data);
}

static int set_contact(datastore_t *self, char *data)
{
	char cmd[] = "system.@system[0].contact=";
	return set_uci(cmd, data);
}

static int set_ntp_server_name(datastore_t *self, char *data)
{
	datastore_t *tmp = self->parent->prev;
	int index = 0;

	while (tmp->prev)
	{
		tmp = tmp->prev;
		if (strcmp(tmp->name, "name"))index++;
	}

	char *path;
	path = (char *) malloc(sizeof(char) * BUFSIZ);
	snprintf(path, BUFSIZ, "system.@ntpserver[%d].name", index);

	return set_uci(path, data);
}

static char *get_uci(char *path)
{
	char *buf;
	printf("%s\n", path);
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

static char *get_hostname(datastore_t *self)
{
	char *path = strdup("system.@system[0].hostname");
	return get_uci(path);
}

static char *get_contact(datastore_t *self)
{
	char *path = strdup("system.@system[0].contact");
	return get_uci(path);
}

static char *get_timezone_location(datastore_t *self)
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

static int set_timezone_location(datastore_t *self, char *data)
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
static int set_server(datastore_t *self, char *data)
{
	printf("%s\n", self->value);
	printf("%s\n", data);
	return 0;
}

static char *get_server(datastore_t *self)
{
	printf("Getting your servers.\n");
	printf("%s\n", self->value);
	printf("%s\n", self->name);
	return 0;
}

static char *get_current_datetime(datastore_t *self)
{
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime ( &rawtime );
	return asctime(timeinfo);
}

static char *get_location(datastore_t *self)
{
	char *path = strdup("system.@system[0].location");
	return get_uci(path);
}

static char *get_ntp_enable(datastore_t *self)
{
	char *path = strdup("system.ntp.enable");
	return get_uci(path);
}

static char *get_ntp_server_name(datastore_t *self)
{
	datastore_t *tmp = self->parent->prev;
	int index = 0;

	while (tmp->prev)
	{
		tmp = tmp->prev;
		if (strcmp(tmp->name, "name"))index++;
	}

	char *path;
	path = (char *) malloc(sizeof(char) * BUFSIZ);
	snprintf(path, BUFSIZ, "system.@ntpserver[%d].name", index);

	return get_uci(path);
}


static datastore_t *create_server_udp(datastore_t *self, char *name,
                                      char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, NULL, NULL, 0);
	if (strcmp(name, "address") == 0)
	{

	}
	else if (strcmp(name, "port") == 0)
	{

	}
	return child;
}


static datastore_t *create_server_child(datastore_t *self, char *name,
                                        char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, NULL, NULL,
	                     0);

	if (strcmp(name, "name") == 0)
	{
		child->get = get_ntp_server_name;
		child->set = set_ntp_server_name;
		child->is_key = 1;
	}
	else if (strcmp(name, "association_type") == 0)
	{
		ds_set_is_config(child, 0, 0);

		//get
	}
	else if (strcmp(name, "iburst") == 0)
	{
		ds_set_is_config(child, 0, 0);

	}
	else if (strcmp(name, "prefer") == 0)
	{
		ds_set_is_config(child, 0, 0);
	}
	else if (strcmp(name, "udp") == 0)
	{
		child->create_child = create_server_udp;
	}
	else if (strcmp(name, "enabled") == 0)
	{

	}

	return child;
}

static datastore_t *create_clock_child(datastore_t *self, char *name,
                                       char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, NULL, NULL,
	                     0);

	if (strcmp(name, "timezone-location"))
	{
		child->get = get_timezone_location;
		child->set = set_timezone_location;
	}
	else if (strcmp(name, "timezone-utc-offset"))
	{

	}

	return child;
}

static datastore_t *create_dns_resolver_server_udp_tcp (datastore_t *self,
        char *name,
        char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, NULL, NULL,
	                     0);
	return child;	
}

static datastore_t *create_dns_resolver_server_child(datastore_t *self,
        char *name,
        char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, NULL, NULL,
	                     0);

	if (strcmp(name, "name"))
	{
		child->is_key = 1;
	}
	else if (strcmp(name, "udp-and-tcp"))
	{
		child->create_child = create_dns_resolver_server_udp_tcp;
	}
	
	return child;
}

static datastore_t *create_dns_resolver_child(datastore_t *self, char *name,
        char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, NULL, NULL,
	                     0);
	if (strcmp(name, "search"))
	{
		child->is_list = 1;
	}
	else if (strcmp(name, "server"))
	{
		child->create_child = create_dns_resolver_server_child;
	}

	return child;
}

static datastore_t *create_system_child(datastore_t *self, char *name,
                                        char *value, char *ns, char *target_name, int target_position)
{
	datastore_t *child = ds_add_child_create(self, name, value, NULL, NULL,
	                     0);

	if (strcmp(name, "server"))
	{
		child->is_list = 1;
		child->create_child = create_server_child;
	}
	else if (strcmp(name, "hostname"))
	{
		child->get = get_hostname;
		child->set = set_hostname;
	}
	else if (strcmp(name, "location"))
	{
		child->get = get_location;
		child->set = set_location;
	}
	else if (strcmp(name, "clock"))
	{
		child->create_child = create_clock_child;
	}
	else if (strcmp(name, "contact"))
	{
		child->get = get_contact;
		child->set = set_contact;
	}
	else if (strcmp(name, "dns_resolver"))
	{
		child->create_child = create_dns_resolver_child;
	}

	return child;
}

static int create_store()
{
	// ietf-system
	datastore_t *system = ds_add_child_create(&root, "system", NULL, ns, NULL, 0);

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

	datastore_t *contact = ds_add_child_create(system, "contact", "yes, please",
	                       NULL, NULL, 0); // string
	contact->get = get_contact;
	contact->set = set_contact;

	datastore_t *clock = ds_add_child_create(system, "clock", NULL, NULL, NULL, 0);

	// clock
	datastore_t *timezone_location = ds_add_child_create(clock, "timezone-location",
	                                 "Europe/Zagreb", NULL, NULL, 0); // string
	timezone_location->get = get_timezone_location;
	timezone_location->set = set_timezone_location;
	timezone_location->update = generic_update;
	ds_add_child_create(clock, "timezone-utc-offset", "60", NULL, NULL, 0); // int16
	//TODO
	//does offset needs to written
	datastore_t *ntp = ds_add_child_create(system, "ntp", NULL, NULL, NULL, 0);
	ntp->create_child = create_server_child;


	// ntp
	datastore_t *ntp_enable = ds_add_child_create(ntp, "enabled", "false", NULL,
	                          NULL, 0); // bool
	ntp_enable->get = get_ntp_enable;
	//ntp_enable->set = set_ntp_enable;
	//TODO
	//how to bind function to list element
	//information to obtain index
	// server list
	for (int i = 1; i < 3; i++)
	{
		//server
		datastore_t *server = ds_add_child_create(ntp, "server", NULL, NULL, NULL, 0);
		//server->set = set_server;
		//server->get = get_server;
		server->is_list = 1;
		char server_name[BUFSIZ];
		snprintf(server_name, BUFSIZ, "server%d", i);
		datastore_t *name = ds_add_child_create(server, "name", server_name, NULL, NULL,
		                                        0); // string
		name->is_key = 1;
		name->get = get_ntp_server_name;
		name->set = set_ntp_server_name;


		datastore_t *association_type = ds_add_child_create(server, "association-type",
		                                NULL, NULL, NULL, 0);
		ds_set_is_config(association_type, 0, 0);
		datastore_t *iburst = ds_add_child_create(server, "iburst", "false", NULL, NULL,
		                      0);
		ds_set_is_config(iburst, 0, 0);
		datastore_t *prefer = ds_add_child_create(server, "prefer", "false", NULL, NULL,
		                      0);
		//prefer->get = get_server;
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
