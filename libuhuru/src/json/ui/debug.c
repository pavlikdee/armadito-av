#include "libuhuru-config.h"
#include <libuhuru/core.h>

#include "debug.h"

#include <glib.h>
#include <stdlib.h>

static void json_object_str(struct json_object *obj, GString *out, int level);
static void json_array_str(struct json_object *obj, GString *out, int level);
static void json_value_str(struct json_object *obj, GString *out, int level);

static void space(GString *out, int level)
{
  while (level--)
    g_string_append_c(out, ' ');
}

static void json_object_str(struct json_object *obj, GString *out, int level)
{
  int first = 1;

  g_string_append(out, "{\n");

  json_object_object_foreach(obj, key, val) {
    if (!first) {
      g_string_append(out, ",\n");
    } else
      first = 0;

    space(out, level + 2);

    g_string_append_printf(out, "\"%s\": ", key);
    json_value_str(val, out, level + 2);
  }

  if (!first)
    g_string_append_c(out, '\n');

  space(out, level);

  g_string_append_c(out, '}');
}

static void json_array_str(struct json_object *obj, GString *out, int level)
{
  int i;
  struct json_object *val;
  int first = 1;

  g_string_append(out, "[\n");

  for(i = 0; i < json_object_array_length(obj); i++) {
    if (!first) {
      g_string_append(out, ",\n");
    } else
      first = 0;

    space(out, level + 2);

    val = json_object_array_get_idx(obj, i);
    json_value_str(val, out, level + 2);
  }

  if (!first)
    g_string_append_c(out, '\n');

  space(out, level);

  g_string_append_c(out, ']');
}

static void json_value_str(struct json_object *obj, GString *out, int level)
{
  int i;

  switch(json_object_get_type(obj)) {
  case json_type_null:
    g_string_append(out, "(null)");
    break;
  case json_type_boolean:
    g_string_append(out, json_object_get_boolean(obj) ? "true" : "false");
    break;
  case json_type_double:
    g_string_append_printf(out, "%f", json_object_get_double(obj));
    break;
  case json_type_int:
    g_string_append_printf(out, "%d", json_object_get_int(obj));
    break;
  case json_type_string:
    g_string_append_printf(out, "\"%s\"", json_object_get_string(obj));
    break;
  case json_type_object:
    json_object_str(obj, out, level);
    break;
  case json_type_array:
    json_array_str(obj, out, level);
  }
}

const char *jobj_str(struct json_object *obj)
{
  GString *out = g_string_sized_new(100);
  const char *ret;

  json_value_str(obj, out, 0);

  ret = out->str;

  g_string_free(out, FALSE);

  return ret;
}

void jobj_debug(struct json_object *obj, const char *obj_name)
{
  const char *s = jobj_str(obj);

  uhuru_log(UHURU_LOG_MODULE, UHURU_LOG_LEVEL_DEBUG, "JSON %s: %s", obj_name, s);

  free((void *)s);
}
