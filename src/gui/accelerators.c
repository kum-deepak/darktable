#include <gtk/gtk.h>

#include "gui/accelerators.h"
#include "common/darktable.h"
#include "control/control.h"

static void dt_accel_path_global(char *s, size_t n, const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s",
           NC_("accel", "global"), path);
}

static void dt_accel_path_view(char *s, size_t n, char *module,
                               const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           NC_("accel", "views"), module, path);
}

static void dt_accel_path_iop(char *s, size_t n, char *module,
                              const char *path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           NC_("accel", "image operations"), module, path);
}

static void dt_accel_path_lib(char *s, size_t n, char *module,
                              const char* path)
{
  snprintf(s, n, "<Darktable>/%s/%s/%s",
           NC_("accel", "plugins"), module, path);
}

void dt_accel_register_global(const gchar *path, guint accel_key,
                              GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_global(accel_path, 256, path);
  gtk_accel_map_add_entry(accel_path, accel_key, mods);

  strcpy(accel->path, accel_path);
  accel->views = DT_LIGHTTABLE_VIEW | DT_DARKTABLE_VIEW | DT_CAPTURE_VIEW;
  accel->local = FALSE;
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);

}

void dt_accel_register_view(dt_view_t *self, const gchar *path, guint accel_key,
                            GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_view(accel_path, 256, self->module_name, path);
  gtk_accel_map_add_entry(accel_path, accel_key, mods);

  strcpy(accel->path, accel_path);
  if(!strcmp(self->module_name, "capture"))
    accel->views = DT_CAPTURE_VIEW;
  else if(!strcmp(self->module_name, "darkroom"))
    accel->views = DT_DARKTABLE_VIEW;
  else if(!strcmp(self->module_name, "filmstrip"))
    accel->views = DT_DARKTABLE_VIEW | DT_CAPTURE_VIEW;
  else if(!strcmp(self->module_name, "lighttable"))
    accel->views = DT_LIGHTTABLE_VIEW;
  else
    accel->views = 0;
  accel->local = FALSE;
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);
}

void dt_accel_register_iop(dt_iop_module_so_t *so, gboolean local,
                           const gchar *path, guint accel_key,
                           GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_iop(accel_path, 256, so->op, path);
  gtk_accel_map_add_entry(accel_path, accel_key, mods);

  strcpy(accel->path, accel_path);
  accel->local = local;
  accel->views = DT_DARKTABLE_VIEW;
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);
}

void dt_accel_register_lib(dt_lib_module_t *self, gboolean local,
                           const gchar *path, guint accel_key,
                           GdkModifierType mods)
{
  gchar accel_path[256];
  dt_accel_t *accel = (dt_accel_t*)malloc(sizeof(dt_accel_t));

  dt_accel_path_lib(accel_path, 256, self->plugin_name, path);
  accel->local = local;
  accel->views = self->views();
  darktable.control->accelerator_list =
      g_slist_prepend(darktable.control->accelerator_list, accel);
}

void dt_accel_connect_global(const gchar *path, GClosure *closure)
{
  gchar accel_path[256];
  dt_accel_path_global(accel_path, 256, path);
  gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                  closure);
}

void dt_accel_connect_view(dt_view_t *self, const gchar *path,
                           GClosure *closure)
{
  gchar accel_path[256];
  dt_accel_path_view(accel_path, 256, self->module_name, path);
  gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                  closure);
  self->accel_closures = g_slist_prepend(self->accel_closures, closure);
}

void dt_accel_connect_iop(dt_iop_module_t *module, const gchar *path,
                          GClosure *closure)
{
  gchar accel_path[256];
  dt_accel_path_iop(accel_path, 256, module->op, path);
  gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                  closure);

  module->accel_closures = g_slist_prepend(module->accel_closures, closure);
}

void dt_accel_connect_lib(dt_lib_module_t *module, const gchar *path,
                          GClosure *closure)
{
  gchar accel_path[256];
  dt_accel_path_lib(accel_path, 256, module->plugin_name, path);
  gtk_accel_group_connect_by_path(darktable.control->accelerators, accel_path,
                                  closure);

  module->accel_closures = g_slist_prepend(module->accel_closures, closure);
}

void dt_accel_disconnect_list(GSList *list)
{
  GClosure *closure;
  while(list)
  {
    closure = (GClosure*)(list->data);
    gtk_accel_group_disconnect(darktable.control->accelerators, closure);
    list = g_slist_delete_link(list, list);
  }
}
