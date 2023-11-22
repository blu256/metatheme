/**
 * This file is part of MetaTheme.
 * Copyright (c) 2005 Martin Dvorak <jezek2@advel.cz>
 *
 * MetaTheme is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MetaTheme is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MetaTheme; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include "metathemeInt.h"

#include "msstyle-loader.c"
#include "msstyle-ini.c"

GtkDialog *dialog;
GtkListStore *themes_list;
GtkTreeView *tree;
GtkWidget *ok_button;
char *current_theme = NULL, *current_variant = NULL;


static gboolean themes_list_button_press_event(GtkWidget *widget, GdkEventButton *event);
static void save_cfg();


static void create_dialog()
{
   GtkWidget *vbox, *label, *scrolled, *button;
   GtkCellRenderer *renderer;
   
   dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("MSSTYLE Loader Configuration", NULL, 0,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                        NULL));

   gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

   /* main vbox: */
   vbox = dialog->vbox;
   
   label = gtk_label_new("Available themes:");
   gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
   gtk_misc_set_padding(GTK_MISC(label), 5, 5);
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
   
   /* create list view of imported themes: */
   themes_list = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

   tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(themes_list)));
   //g_object_unref(G_OBJECT(themes_list));
   g_object_ref(G_OBJECT(tree));

   renderer = gtk_cell_renderer_text_new();
   gtk_tree_view_insert_column_with_attributes(tree, 0, "Theme", renderer, "text", 0, NULL);
   renderer = gtk_cell_renderer_text_new();
   gtk_tree_view_insert_column_with_attributes(tree, 1, "Variant", renderer, "text", 1, NULL);
   
   scrolled = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_container_add(GTK_CONTAINER(scrolled), GTK_WIDGET(tree));
   gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

   g_signal_connect(G_OBJECT(tree), "button-press-event", G_CALLBACK(themes_list_button_press_event), NULL);

   /* create ok button: */
   ok_button = gtk_dialog_add_button(dialog, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
   g_signal_connect(G_OBJECT(ok_button), "clicked", G_CALLBACK(save_cfg), NULL);

   gtk_widget_show_all(dialog->vbox);
}


static void load_themes()
{
   DIR *dir = NULL;
   char *tmp, *tmp2;
   struct dirent *de;
   int i, len;
   GtkTreeIter iter;
   GtkTreePath *path;
   ms_theme_t *ms;

   tmp = g_strdup_printf("%s/.metatheme/msstyles/", getenv("HOME"));
   dir = opendir(tmp);
   if (!dir) {
      mkdir(tmp, 0755);
      goto error;
   }

   while ((de = readdir(dir))) {
      len = strlen(de->d_name);
      if (len < 9 || strcmp(de->d_name + (len - 9), ".msstyles")) continue;

      tmp2 = g_strdup_printf("%s%s", tmp, de->d_name);
      ms = ms_open_theme(tmp2);
      if (ms) {
         INI_FILE *ini;
         char *t = ms->ms_theme_info->defaultini;
         char *name, *opt;
         GSList *names = NULL, *it;
         GAllocator *allocator;
         gboolean first = TRUE;

         ini = ms_load_ini(ms, "THEMES_INI");
         if (!ini) {
            ms_close_theme(ms);
            free(tmp2);
            continue;
         }
         
         name = msstyle_ini_get_option(ini, "Documentation", "DisplayName");
         if (!name) name = ms->ms_theme_info->name;

         allocator = g_allocator_new("", 5);
         g_slist_push_allocator(allocator);

         for (i=0; i<ini->sorted->len; i++) {
            char *secname = ini->sorted->pdata[i];

            if (strncasecmp(secname, "ColorScheme.", 12)) continue;
            opt = msstyle_ini_get_option(ini, secname, "DisplayName");
            names = g_slist_append(names, g_strdup(opt));
         }

         it = names;
         while (t && strlen(t) > 0) {
            if (strncmp(t, "LARGEFONTS", 10) && strncmp(t, "EXTRALARGE", 10))  {
               gtk_list_store_append(themes_list, &iter);
               gtk_list_store_set(themes_list, &iter, 0, (first)? name : NULL, 1, it->data, -1);
               gtk_list_store_set(themes_list, &iter, 2, de->d_name, 3, t, -1);

               if (current_theme && current_variant && !strcmp(de->d_name, current_theme) && !strcmp(t, current_variant)) {
                  path = gtk_tree_model_get_path(GTK_TREE_MODEL(themes_list), &iter);
                  gtk_tree_view_set_cursor(tree, path, NULL, FALSE);
                  gtk_tree_view_scroll_to_cell(tree, path, NULL, TRUE, 0.5, 0);
                  gtk_tree_path_free(path);
               }
               
               free(it->data);
               it = it->next;
               first = FALSE;
            }

            t += strlen(t)+1;
         }

         while (it) {
            free(it->data);
            it = it->next;
         }

         g_slist_pop_allocator();

         ms_close_theme(ms);
      }

      free(tmp2);
   }

error:
   if (dir) closedir(dir);
   free(tmp);
}


static gboolean themes_list_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
   if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
      gtk_button_clicked(GTK_BUTTON(ok_button));
      return TRUE;
   }
   
   return FALSE;
}


static void load_cfg()
{
   MT_CONFIG *cfg;
   char *fname;

   fname = g_strdup_printf("%s/.metatheme/config", getenv("HOME"));
   cfg = metatheme_load_config(fname);
   if (!cfg) {
      free(fname);
      return;
   }

   current_theme = (char *)metatheme_get_config_option(cfg, "fname", "msstyle");
   current_variant = (char *)metatheme_get_config_option(cfg, "ini", "msstyle");

   if (current_theme && current_variant) {
      current_theme = g_strdup(current_theme);
      current_variant = g_strdup(current_variant);
   }
   else {
      current_theme = NULL;
      current_variant = NULL;
   }

   metatheme_free_config(cfg);
   free(fname);

}


static void save_cfg()
{
   char *fname;
   MT_CONFIG *cfg;
   GtkTreeIter iter;
   GtkTreePath *path;
   GtkTreeViewColumn *focus_column;
   gchar *theme, *variant;

   gtk_tree_view_get_cursor(tree, &path, NULL);
   if (path == NULL) return;
   if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(themes_list), &iter, path)) {
      gtk_tree_path_free(path);
      return;
   }
   
   gtk_tree_model_get(GTK_TREE_MODEL(themes_list), &iter, 2, &theme, 3, &variant, -1);
   gtk_tree_path_free(path);

   fname = g_strdup_printf("%s/.metatheme/config", getenv("HOME"));
   cfg = metatheme_load_config(fname);
   if (!cfg) {
      free(fname);
      return;
   }

   metatheme_set_config_option(cfg, "fname", "msstyle", theme);
   metatheme_set_config_option(cfg, "ini", "msstyle", variant);

   metatheme_save_config(cfg, fname);
   metatheme_free_config(cfg);
   free(fname);

   free(theme);
   free(variant);
}


int main(int argc, char** argv)
{
   GtkTreeIter iter;

   gtk_init(&argc, &argv);
   create_dialog();
   load_cfg();
   load_themes();

   if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(themes_list), &iter)) {
      GtkWidget *dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                              "No themes found. You must copy at least one *.msstyles file to '~/.metatheme/msstyles/' directory.");
      gtk_dialog_run(GTK_DIALOG(dlg));
      gtk_widget_destroy(dlg);
      gtk_widget_destroy(GTK_WIDGET(dialog));
   }
   else {
      gtk_dialog_run(dialog);
      gtk_widget_destroy(GTK_WIDGET(dialog));
   }

   free(current_theme);
   free(current_variant);

   return 0;
}
