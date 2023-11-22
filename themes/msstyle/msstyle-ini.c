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

#include <stdio.h>
#include <strings.h>
#include "msstyle-ini.h"


static void ini_free(INI_FILE *ini)
{
   if (ini->sections) {
      g_hash_table_destroy(ini->sections);
      g_ptr_array_free(ini->sorted, FALSE);
   }
}


static void section_free(INI_SECTION *s)
{
   if (s->keys) g_hash_table_destroy(s->keys);
}


static guint str_ignorecase_hash(gconstpointer key)
{
   const char *p = key;
   guint h = tolower(*p);
   char c;

   if (h) {
      for (p += 1; *p != '\0'; p++) {
         h = (h << 5) - h + tolower(*p);
      }
   }

   return h;
}


static gboolean str_ignorecase_equal(gconstpointer v, gconstpointer v2)
{
   return !strcasecmp((char *)v, (char *)v2);
}


GHashTable *msstyle_ini_files_new()
{
   return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)ini_free);
}


#define STATE_IGNORE     0
#define STATE_LINE_START 1
#define STATE_SECTION    2
#define STATE_KEY        3
#define STATE_VALUE      4
#define STATE_VALUE2     5

static void msstyle_load_ini(INI_FILE *file, const char *name)
{
   int i, state = STATE_LINE_START, state2 = STATE_LINE_START;
   char *c;
   INI_SECTION *section = NULL;
   GString *str = g_string_new(NULL);
   gchar *key = NULL;
   char *tmp, *tmp2;

   file->sections = g_hash_table_new_full(str_ignorecase_hash, str_ignorecase_equal, g_free, (GDestroyNotify)section_free);
   file->sorted = g_ptr_array_new();

   for (i=0; i<file->length/2; i++) {
      c = file->start + (i*2);

      if (*c == '\r') continue;

      if (*c == '\n' && state != STATE_VALUE2) {
         nextline:
         state = state2;
         continue;
      }

      switch (state) {
         case STATE_LINE_START:
            if (!isspace(*c)) {
               if (*c == '[') {
                  state = STATE_SECTION;
                  state2 = STATE_LINE_START;
                  g_string_truncate(str, 0);
               }
               else {
                  /* not a section, ignore to next line: */
                  state = STATE_IGNORE;
                  state2 = STATE_LINE_START;
               }
            }
            break;

         case STATE_SECTION:
            if (*c == ']') {
               section = g_new0(INI_SECTION, 1);
               section->keys = g_hash_table_new_full(str_ignorecase_hash, str_ignorecase_equal, g_free, g_free);
               tmp2 = g_strdup(str->str);
               g_hash_table_insert(file->sections, tmp2, section);
               g_ptr_array_add(file->sorted, tmp2);

               /* ignore to next line and then parse key: */
               state = STATE_IGNORE;
               state2 = STATE_KEY;
               tmp = 0;
               g_string_truncate(str, 0);
            }
            else {
               g_string_append_c(str, *c);
            }
            break;
         
         case STATE_KEY:
            if (*c == ';' && !tmp) {
               state = STATE_IGNORE;
               state2 = STATE_KEY;
            }
            else if (*c == '[' && !tmp) {
               state = STATE_SECTION;
               state2 = STATE_LINE_START;
               g_string_truncate(str, 0);
            }
            else if (!isspace(*c) && !tmp) {
               tmp = c;
            }
            else if ((isspace(*c) || *c == '=') && tmp) {
               if (key) g_free(key);
               key = g_strdup(str->str);
               
               tmp = NULL;
               state = (*c == '=')? STATE_VALUE2 : STATE_VALUE;
               state2 = STATE_KEY;
               tmp = 0;
               g_string_truncate(str, 0);
            }

            if (tmp) {
               g_string_append_c(str, *c);
            }
            break;

         case STATE_VALUE:
            if (*c == '=') {
               state = STATE_VALUE2;
               state2 = STATE_KEY;
            }
            break;

         case STATE_VALUE2:
            if (!isspace(*c) && !tmp) {
               tmp = c;
            }
            else if ((*c == ';' || *c == '\n') && tmp) {
               int l;
               
               if (!section) {
                  fprintf(stderr, "error: not in section");
                  exit(1);
               }

               /* trim from right: */
               l = strlen(str->str);
               while (isspace(str->str[l-1])) l--;
               g_string_truncate(str, l);
               
               g_hash_table_insert(section->keys, key, g_strdup(str->str));
               key = NULL;

               tmp = NULL;
               state = STATE_IGNORE;
               state2 = STATE_KEY;
               tmp = 0;
               g_string_truncate(str, 0);
            }

            if (tmp) {
               g_string_append_c(str, *c);
            }

            if (*c == '\n') goto nextline;
            break;
      }
   }

   if (key) g_free(key);
   g_string_free(str, TRUE);
}


INI_FILE *msstyle_ini_get(GHashTable *ini_files, const char *name)
{
   INI_FILE *file = g_hash_table_lookup(ini_files, name);

   if (!file->sections) {
      msstyle_load_ini(file, name);
   }

   return file;
}


char *msstyle_ini_get_option(INI_FILE *ini, char *section, char *name)
{
   INI_SECTION *s = g_hash_table_lookup(ini->sections, section);
   if (!s) return NULL;
   
   return g_hash_table_lookup(s->keys, name);
}


char *msstyle_ini_get_class_option(INI_FILE *ini, char *class1, char *class2, char *state, char *name)
{
   char *tmp;
   GString *str = g_string_new(NULL);
   
   if (class2) {
      if (state) {
         g_string_printf(str, "%s.%s(%s)", class1, class2, state);
         tmp = msstyle_ini_get_option(ini, str->str, name);
         if (tmp) goto found;
      }

      g_string_printf(str, "%s.%s", class1, class2);
      tmp = msstyle_ini_get_option(ini, str->str, name);
      if (tmp) goto found;
   }
   
   if (state) {
      g_string_printf(str, "%s(%s)", class1, state);
      tmp = msstyle_ini_get_option(ini, str->str, name);
      if (tmp) goto found;
   }
   
   g_string_printf(str, "%s", class1);
   tmp = msstyle_ini_get_option(ini, str->str, name);

found:
   g_string_free(str, TRUE);
   return tmp;
}
