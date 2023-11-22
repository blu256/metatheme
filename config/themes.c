/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: themes.c 19 2004-07-29 02:45:55Z jzajpt $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "themes.h"
#include "util.h"
#include "metathemeInt.h"
#include "main.h"


static void configure_theme( GtkWidget* widget, gpointer data );
static void set_theme_description( GtkWidget* widget, const char* description, const char* author );
static void theme_selected( GtkWidget* widget );


/* GtkOptionMenu with all themes. */
GtkWidget* optmenu_themes    = NULL;
/* Configure button. */
GtkWidget* button_configure  = NULL;
/* Label with theme description and author. */
GtkWidget* label_description = NULL;
gboolean   first = TRUE;


/*
 * Creates new theme selection page for GtkNotebook.
 */
GtkWidget*
theme_selection_page_new( void )
{
	GtkWidget* table             = NULL;
	GSList*    list_themes_names = NULL;
	int        i;

	table = gtk_table_new( 2, 2, FALSE );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 12 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 6 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 6 );

	/* Get names of all available themes for GtkOptionMenu. */
	for ( i = 0 ; i < g_list_length( list_themes_info ) ; i++ )
	{
		MTCONFIG_THEME_INFO* theme_info = g_list_nth_data( list_themes_info, i );

		if ( strcasecmp( theme_info->name, current_settings.theme ) == 0 )
			settings_set_current_theme( i );

		list_themes_names = g_slist_append( list_themes_names,
			( char* ) theme_info->name );
	}

	/* Create option menu with available themes list. */
	optmenu_themes = optionmenu_from_gslist( list_themes_names );
	g_slist_free( list_themes_names );
	gtk_size_group_add_widget( vsizegroup, optmenu_themes );
	gtk_table_attach( GTK_TABLE( table ), optmenu_themes, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, 0, 0, 0 );
	g_signal_connect( G_OBJECT( optmenu_themes ), "changed", G_CALLBACK( theme_selected ), NULL );

	/* Create configure button. */
	button_configure = gtk_button_new_with_label( _( "Configure..." ) );
	gtk_size_group_add_widget( vsizegroup, button_configure );
	gtk_table_attach( GTK_TABLE( table ), button_configure, 1, 2, 0, 1, GTK_SHRINK, GTK_FILL, 0, 0 );
	g_signal_connect( G_OBJECT( button_configure ), "clicked", G_CALLBACK( configure_theme ), NULL );

	/* Create label with theme description and theme author. */
	label_description = gtk_label_new( NULL );
	gtk_label_set_line_wrap( GTK_LABEL( label_description ), TRUE );
	gtk_misc_set_alignment( GTK_MISC( label_description ), 0.0, 0.0 );
	gtk_label_set_justify( GTK_LABEL( label_description ), GTK_JUSTIFY_LEFT );
	gtk_table_attach( GTK_TABLE( table ), label_description, 0, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0 );

	gtk_option_menu_set_history( GTK_OPTION_MENU( optmenu_themes ), current_settings.current_theme );
	theme_selected( optmenu_themes );

	return table;
}


/*
 * Callback activated after pressing configure button.
 */
static void
configure_theme( GtkWidget* widget, gpointer data )
{
	MTCONFIG_THEME_INFO* theme_info;
	char* tmp;
	GError* error;
	gchar* argv[2];

	/* Get informations about currently selected theme. */
	theme_info = g_list_nth_data( list_themes_info, current_settings.current_theme );

	if (theme_info->config[0] != '/') {
		tmp = g_strdup_printf(METATHEMEDIR "/themes/%s", theme_info->config);
	}
	else {
		tmp = g_strdup(theme_info->config);
	}

	argv[0] = tmp;
	argv[1] = NULL;
	if (!g_spawn_async(NULL, argv, NULL, 0, NULL, NULL, NULL, &error)) {
		g_free(tmp);
		return;
	}

	g_free(tmp);
}


/*
 * Util function, sets theme description.
 */
static void
set_theme_description( GtkWidget* widget, const char* description, const char* author )
{
	char buffer[ 2048 ];
	char* esc_description = g_markup_escape_text( description, strlen( description ) );
	char* esc_author = g_markup_escape_text( author, strlen( author ) );
	snprintf( buffer, 2048,
		"<b>Description</b>: %s\n"
		"<b>Author</b>: %s\n",
		esc_description,
		esc_author
	);
	free( esc_description );
	free( esc_author );
	gtk_label_set_markup( GTK_LABEL( widget ), buffer );
}


/*
 * Updates theme description.
 */
static void
theme_selected( GtkWidget* widget )
{
	MTCONFIG_THEME_INFO* theme_info;

	settings_set_current_theme( gtk_option_menu_get_history( GTK_OPTION_MENU( widget ) ) );

	/* Get informations about currently selected theme. */
	theme_info = g_list_nth_data( list_themes_info, current_settings.current_theme );

	/* Update theme description. */
	set_theme_description( label_description,
		theme_info->description,
		theme_info->author
	);
	if ( first == TRUE )
	{
		GtkRequisition requisition;
		gtk_widget_size_request( label_description, &requisition );
		gtk_widget_set_size_request( label_description, requisition.width, -1 );
		first = FALSE;
	}

	settings_set_theme( theme_info->directory );

	/* TODO: Check if configure should be enabled or not. */
	gtk_widget_set_sensitive( button_configure, (theme_info->config != NULL) );
}
