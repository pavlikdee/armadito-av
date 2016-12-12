#include <libarmadito/armadito.h>
#include <libjrpc/jrpc.h>

#include "test.h"
#include "libtest.h"

#include <fcntl.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static struct a6o_base_info *base_info_new(const char *name, time_t base_update_ts, const char *version, size_t signature_count, const char *full_path)
{
	struct a6o_base_info *b = malloc(sizeof(struct a6o_base_info));

	b->name = strdup(name);
	b->base_update_ts = base_update_ts;
	b->version = version;
	b->signature_count = signature_count;
	b->full_path = strdup(full_path);

	return b;
}

#define N_BASES 3

static struct a6o_module_info *module_info_new(void)
{
	struct a6o_module_info *mod = malloc(sizeof(struct a6o_module_info));

	mod->name = strdup("moduleH1");
	mod->mod_status = A6O_UPDATE_NON_AVAILABLE;
	mod->mod_update_ts = 69;
	mod->base_infos = calloc(N_BASES + 1, sizeof(void *));
	mod->base_infos[0] = base_info_new("daily", 1, "0.1", 0xdeadbeef, "/var/lib/clamav/daily.cld");
	mod->base_infos[1] = base_info_new("main", 2, "0.2", 0xabbaface, "/var/lib/clamav/main.cvd");
	mod->base_infos[2] = base_info_new("bytecode", 3, "0.3", 0xfeedface, "/var/lib/clamav/bytecode.cld");
	mod->base_infos[3] = NULL;

	return mod;
}

static int test_notification(struct jrpc_connection *conn)
{
	json_t *params;
	struct a6o_module_info *mod = module_info_new();

	/* JRPC_STRUCT2JSON(a6o_module_info, mod, &params); */

	return jrpc_notify(conn, "status", params);
}

static void simple_cb(json_t *result, void *user_data)
{
	fprintf(stderr, "The callback has been called, result is %lld\n", json_integer_value(json_object_get(result, "result")));
}

static int test_call(struct jrpc_connection *conn, int count)
{
	int op = 0, i, ret = 0;

	for(i = 0; i < count; i++) {
		json_t *operands = json_object();

		json_object_set(operands, "op1", json_integer(op));
		json_object_set(operands, "op2", json_integer(op + 1));
		op++;

		/* struct operands o; */
		/* o.op1 = 1; */
		/* o.op2 = 2; */

		ret = jrpc_call(conn, "add", operands, simple_cb, NULL);

		if (ret)
			break;
	}

	return ret;
}

static void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s INPUT_PIPE OUTPUT_PIPE\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct jrpc_connection *conn;
	int *p_input_fd = malloc(sizeof(int));
	int *p_output_fd = malloc(sizeof(int));
	int ret;

	*p_output_fd = open(argv[2], O_WRONLY);
	if (*p_output_fd < 0) {
		perror("cannot open output pipe");
		exit(EXIT_FAILURE);
	}

	sleep(1);

	*p_input_fd = open(argv[1], O_RDONLY);
	if (*p_input_fd < 0) {
		perror("cannot open input pipe");
		exit(EXIT_FAILURE);
	}

	conn = jrpc_connection_new(NULL, NULL);

	jrpc_connection_set_read_cb(conn, unix_fd_read_cb, p_input_fd);
	jrpc_connection_set_write_cb(conn, unix_fd_write_cb, p_output_fd);

	test_call(conn, 10);

	while((ret = jrpc_process(conn)) > 0) {
		if (ret == 1)
			return 1;
	}

	return ret;
}