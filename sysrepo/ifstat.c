/*
 * Copyright (C) 2015 Cisco Systems, Inc.
 *
 * Author: Luka Perkov <luka.perkov@sartura.hr>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <freenetconfd/plugin.h>
#include <freenetconfd/datastore.h>
#include <freenetconfd/freenetconfd.h>


#include <roxml.h>

#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

__unused struct module *init();
__unused void destroy();

struct module m;
char *ns = "urn:ietf:params:xml:ns:yang:ifstat";
datastore_t datastore_root = DATASTORE_ROOT_DEFAULT;

static node_t *sysrepo_root = NULL;
static char *sysrepo_data = NULL;
static char xpath[BUFSIZ] = { 0 };


#define ___debug(fmt, ...) do { \
        fprintf(stderr, "%s: %s: %s (%d): " fmt "\n", "sysrepo-ifstat.so", __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__); \
		fflush(stderr); \
    } while (0)


#define CUSTOM_XML_APPLY_OP_XPATH "<xml><command>apply_xpathOpDataStore</command><param1>urn:ietf:params:xml:ns:yang:ietf-interfaces:interfaces-state</param1></xml>"
#define CUSTOM_XML_HELO "<xml><protocol>srd</protocol></xml>"

int sock = -1;
#define SYSREPO_BUF 10000
char message[SYSREPO_BUF] = { 0 };

static int sysrepo_connect()
{
	struct sockaddr_in server;
	int flags;
	int len;

	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1) return -1;

	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(3500);

	if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		___debug("failed to connect");
		return -1;
	}

	flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1)
		return -1;

	flags &= ~O_NONBLOCK;
	fcntl(sock, F_SETFL, flags);

	___debug("connected");

	sprintf(message, "%07d %s", (int) strlen(CUSTOM_XML_HELO), CUSTOM_XML_HELO);
	___debug("sending: %s", message);
	if (send(sock , message , strlen(message) , 0) < 0) {
		return -1;
	}

	memset(message, 0, SYSREPO_BUF);
	for (int i = 0; i < 8; i++) {
		recv(sock, message + i, 1, 0);
	}

	len = atoi(message);
	___debug("received: %s", message);

	memset(message, 0, SYSREPO_BUF);
	for (int i = 0; i < len; i++) {
		recv(sock, message + i, 1, 0);
	}
	___debug("received (%d): %s", len, message);

	return 0;
}

static int sysrepo_write(const char *fmt, ...)
{
	node_t *root, *child;
	char *xml = NULL, buffer[BUFSIZ];
	va_list ap;

	root = roxml_load_buf(CUSTOM_XML_APPLY_OP_XPATH);
	if (!root) return -1;

	va_start(ap, fmt);
	vsnprintf(buffer, BUFSIZ, fmt, ap);
	va_end(ap);

	child = roxml_get_chld(root, "xml", 0);
	roxml_add_node(child, 0, ROXML_ELM_NODE, "param2", buffer);

	roxml_commit_changes(root, NULL, &xml, 0);

	sprintf(message, "%07d %s", (int) strlen(xml), xml);
	___debug("sending: %s", message);
	if (send(sock, message, strlen(message) , 0) < 0) {
		return -1;
	}

	roxml_close(root);
	free(xml);

	return 0;
}

static int sysrepo_read()
{
	int len = 0;

	memset(message, 0, SYSREPO_BUF);
	for (int i = 0; i < 8; i++) {
		recv(sock, message + i, 1, 0);
	}

	len = atoi(message);
	___debug("received: %s", message);

	roxml_close(sysrepo_root);
	free(sysrepo_data);
	char *sysrepo_data = calloc(len + 1, sizeof(char));
	if (!sysrepo_data) {
		return -1;
	}

	for (int i = 0; i < len; i++) {
		recv(sock, sysrepo_data + i, 1, 0);
	}
	___debug("received (%d): %s", len, sysrepo_data);

	sysrepo_root = roxml_load_buf(sysrepo_data + 8);

	if (!sysrepo_root)
		return -1;

	return 0;
}

static void sysrepo_tree(const char *fmt, ...)
{
	va_list ap;
	int rc;

	rc = sysrepo_connect();
	if (rc) return;

	va_start(ap, fmt);
	sysrepo_write(fmt, ap);
	va_end(ap);

	sysrepo_read();

	close(sock);
	sock = -1;
}

static char *get_node(char *tag_value, char *child_name, bool statistics)
{
	char *xml = NULL;
	node_t **nodes;
	int nodes_num;

	if (statistics)
		snprintf(xpath, BUFSIZ, "/data/interfaces-state/interface[./name = '%s']/statistics/%s/text()", tag_value, child_name);
	else
		snprintf(xpath, BUFSIZ, "/data/interfaces-state/interface[./name = '%s']/%s/text()", tag_value, child_name);

	nodes = roxml_xpath(sysrepo_root, xpath, &nodes_num);
	if (!nodes_num) {
		free(xml);
		return NULL;
	}

	roxml_commit_changes(nodes[0], NULL, &xml, 0);

	return xml;
}

static char *tree_update(datastore_t *datastore)
{
	ds_free(datastore->child, 1);
	datastore->child = NULL;
	node_t **nodes;
	int nodes_num;
	char *value;

	sysrepo_tree("/*");

	nodes = roxml_xpath(sysrepo_root, "/data/interfaces-state/interface/name/text()", &nodes_num);

	for (int i = 0; i < nodes_num; i++)
	{
		roxml_commit_changes(nodes[i], NULL, &value, 0);
		___debug("using: %s", value);

		datastore_t *interface = ds_add_child_create(datastore, "interface", NULL, NULL, NULL, 0);
		interface->is_list = 1;

		datastore_t *name = ds_add_child_create(interface, "name", value, NULL, NULL, 0);
		name->is_key = 1;

		ds_add_child_create(interface, "type", NULL, NULL, NULL, 0);

		ds_add_child_create(interface, "admin-status", get_node(value, "admin-status", false), NULL, NULL, 0);
		ds_add_child_create(interface, "oper-status", get_node(value, "oper-status", false), NULL, NULL, 0);
		ds_add_child_create(interface, "last-status", get_node(value, "last-status", false), NULL, NULL, 0);
		ds_add_child_create(interface, "if-index", get_node(value, "if-index", false), NULL, NULL, 0);
		ds_add_child_create(interface, "phys-address", get_node(value, "phys-address", false), NULL, NULL, 0);
		ds_add_child_create(interface, "higher-layer-if", get_node(value, "higher-layer-if", false), NULL, NULL, 0);
		ds_add_child_create(interface, "lower-layer-if", get_node(value, "lower-layer-if", false), NULL, NULL, 0);
		ds_add_child_create(interface, "speed", get_node(value, "speed", false), NULL, NULL, 0);

		datastore_t *stats = ds_add_child_create(interface, "statistics", NULL, NULL, NULL, 0);

		ds_add_child_create(stats, "discontinuity-time", get_node(value, "discontinuity-time", true), NULL, NULL, 0);
		ds_add_child_create(stats, "in-octets", get_node(value, "in-octets", true), NULL, NULL, 0);
		ds_add_child_create(stats, "in-unicast-pkts", get_node(value, "in-unicast-pkts", true), NULL, NULL, 0);
		ds_add_child_create(stats, "in-broadcast-pkts", get_node(value, "in-broadcast-pkts", true), NULL, NULL, 0);
		ds_add_child_create(stats, "in-multicast-pkts", get_node(value, "in-multicast-pkts", true), NULL, NULL, 0);
		ds_add_child_create(stats, "in-discards", get_node(value, "in-discards", true), NULL, NULL, 0);
		ds_add_child_create(stats, "in-errors", get_node(value, "in-errors", true), NULL, NULL, 0);
		ds_add_child_create(stats, "in-unknown-protos", get_node(value, "in-unknown-protos", true), NULL, NULL, 0);
		ds_add_child_create(stats, "out-octets", get_node(value, "out-octets", true), NULL, NULL, 0);
		ds_add_child_create(stats, "out-unicast-pkts", get_node(value, "out-unicast-pkts", true), NULL, NULL, 0);
		ds_add_child_create(stats, "out-broadcast-pkts", get_node(value, "out-broadcast-pkts", true), NULL, NULL, 0);
		ds_add_child_create(stats, "out-multicast-pkts", get_node(value, "out-multicast-pkts", true), NULL, NULL, 0);
		ds_add_child_create(stats, "out-discards", get_node(value, "out-discards", true), NULL, NULL, 0);
		ds_add_child_create(stats, "out-errors", get_node(value, "out-errors", true), NULL, NULL, 0);

		free(value);
	}

	return NULL;
}

__unused struct module *init()
{
	ds_add_child_create(&datastore_root, "interfaces-state", NULL, ns, NULL, 0)->get = tree_update;

	m.rpc_count = 0;
	m.ns = ns;
	m.datastore = &datastore_root;

	return &m;
}

__unused void destroy()
{
	ds_free(datastore_root.child, 1);
	datastore_root.child = NULL;

	roxml_close(sysrepo_root);
	roxml_release(RELEASE_ALL);
	free(sysrepo_data);
}
