/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: util.c 19 2004-07-29 02:45:55Z jzajpt $
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

#include <glib.h>
#include <gtk/gtk.h>
#include <signal.h>
#include <string.h>
#include "metathemeInt.h"
#include "main.h"
#include "util.h"

static void dialog_response( GtkWidget* widget );


GtkWidget* dialog_get_name = NULL;
char*      dialog_str = NULL;


GtkWidget*
optionmenu_new( char* values[], unsigned int num )
{
	GtkWidget*   options_menu = NULL;
	GtkWidget*   menu         = NULL;
	unsigned int i;

	menu = gtk_menu_new();
	for ( i = 0 ; i < num ; i++ )
		gtk_menu_shell_append( GTK_MENU_SHELL( menu ),
			gtk_menu_item_new_with_label( values[ i ] ) );

	options_menu = gtk_option_menu_new();
	gtk_option_menu_set_menu( GTK_OPTION_MENU( options_menu ), menu );

	return options_menu;
}


/*
 * Creates new GtkOptionMenu and appends items from list.
 */
GtkWidget*
optionmenu_from_gslist( GSList* list )
{
	GtkWidget*   options_menu = NULL;
	GtkWidget*   menu         = NULL;
	unsigned int i;

	menu = gtk_menu_new();
	for ( i = 0 ; i < g_slist_length( list ) ; i++ )
		gtk_menu_shell_append( GTK_MENU_SHELL( menu ),
			gtk_menu_item_new_with_label( g_slist_nth_data( list, i ) ) );

	options_menu = gtk_option_menu_new();
	gtk_option_menu_set_menu( GTK_OPTION_MENU( options_menu ), menu );

	return options_menu;
}


/*
 * Creates dialog with input field and returns entered string.
 * Dialog will have title pointed by dialog_title parameter,
 * and will write dialog_text parameter into dialog.
 */
char*
get_name_dialog( GtkWidget* parent, char* dialog_title, char* dialog_text )
{
	GtkWidget* vbox         = NULL;
	GtkWidget* label        = NULL;
	GtkWidget* entry        = NULL;

	/* Create dialog with ok and cancel buttons. */
	dialog_get_name = gtk_dialog_new_with_buttons( dialog_title, GTK_WINDOW( parent ),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_NONE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_NONE,
		NULL );
	gtk_window_set_position( GTK_WINDOW( dialog_get_name ), GTK_WIN_POS_CENTER );
	gtk_window_set_resizable( GTK_WINDOW( dialog_get_name ), FALSE );

	vbox = gtk_vbox_new( FALSE, 6 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 6 );
	gtk_container_add( GTK_CONTAINER( GTK_DIALOG( dialog_get_name )->vbox ), vbox );

	/* Create label. */
	label = gtk_label_new( dialog_text );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );

	/* Create input field. */
	entry = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( entry ), 255 );
	gtk_widget_set_size_request( entry, 300, -1 );
	gtk_box_pack_start( GTK_BOX( vbox ), entry, TRUE, TRUE, 0 );

	g_signal_connect_swapped( G_OBJECT( dialog_get_name ), "response",
		G_CALLBACK( dialog_response ), entry );

	/* Show dialog and wait for it's termination. */
	gtk_widget_show_all( dialog_get_name );
	gtk_dialog_run( GTK_DIALOG( dialog_get_name ) );
	gtk_widget_destroy( dialog_get_name );

	return dialog_str;
}


static void
dialog_response( GtkWidget* widget )
{
	dialog_str = strdup( gtk_entry_get_text( GTK_ENTRY( widget ) ) );
}


/*
 * Popups an error dialog with error_text.
 */
void
error_dialog( char* error_text )
{
	GtkWidget* dialog = gtk_message_dialog_new(
		GTK_WINDOW( main_window ),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		error_text
	);
	gtk_dialog_run( GTK_DIALOG( dialog ) );
	gtk_widget_destroy( dialog );
}
