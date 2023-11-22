/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: fonts.c 19 2004-07-29 02:45:55Z jzajpt $
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

#include <gtk/gtk.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "metathemeInt.h"
#include "main.h"
#include "util.h"
#include "fonts.h"


static void font_selected( GtkWidget* widget );
static void show_fonts_dialog( GtkWidget* widget );


GtkWidget* button_font           = NULL;
GtkWidget* dialog_font_selection = NULL;


GtkWidget*
font_selection_page_new( void )
{
	GtkWidget* table      = NULL;
	GtkWidget* label_font = NULL;

	table = gtk_table_new( 1, 2, FALSE );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 6 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 6 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 12 );

	label_font = gtk_label_new( _( "Font: " ) );
	gtk_table_attach( GTK_TABLE( table ), label_font, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0 );

	button_font = gtk_button_new_with_label( current_settings.application_font );
	gtk_size_group_add_widget( vsizegroup, button_font );
	gtk_table_attach( GTK_TABLE( table ), button_font, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0 );
	g_signal_connect( G_OBJECT( button_font ), "clicked", G_CALLBACK( show_fonts_dialog ), NULL );

	return table;
}


static void
font_selected( GtkWidget* widget )
{
	GtkFontSelection* fontsel = GTK_FONT_SELECTION( 
		GTK_FONT_SELECTION_DIALOG( dialog_font_selection )->fontsel
	);
	char* family = g_strdup( pango_font_family_get_name( fontsel->family ) );
	int size = atoi( gtk_entry_get_text( GTK_ENTRY( fontsel->size_entry ) ) );
	char buffer[ 1024 ];

	snprintf( buffer, 1024, "%s %d", family, size );

	settings_set_application_font( buffer );
	gtk_button_set_label( GTK_BUTTON( button_font ), current_settings.application_font );

	gtk_widget_destroy( GTK_WIDGET( dialog_font_selection ) );
}


static void
show_fonts_dialog( GtkWidget* widget )
{
	GtkBox* box = NULL;

	/* Prepare font selection dialog. */
	dialog_font_selection = gtk_font_selection_dialog_new( _( "Font Selection" ) );
	gtk_window_set_modal( GTK_WINDOW( dialog_font_selection ), TRUE );
	gtk_window_set_transient_for( GTK_WINDOW( dialog_font_selection ), GTK_WINDOW( main_window ) );

	/* Setup necessary callbacks. */
	g_signal_connect( G_OBJECT( GTK_FONT_SELECTION_DIALOG( dialog_font_selection )->ok_button ),
		"clicked", G_CALLBACK( font_selected ), dialog_font_selection );
	g_signal_connect_swapped( G_OBJECT( GTK_FONT_SELECTION_DIALOG( dialog_font_selection )->cancel_button ),
		"clicked", G_CALLBACK( gtk_widget_destroy ), G_OBJECT( dialog_font_selection ) );

	gtk_font_selection_dialog_set_font_name(
		GTK_FONT_SELECTION_DIALOG( dialog_font_selection ),
		current_settings.application_font
	);

	/* Swap OK and Cancel buttons. */
	box = GTK_BOX( ( GTK_FONT_SELECTION_DIALOG( dialog_font_selection )->ok_button )-> parent );
	gtk_box_reorder_child( box, GTK_FONT_SELECTION_DIALOG( dialog_font_selection )->ok_button, 0 );
	gtk_box_reorder_child( box, GTK_FONT_SELECTION_DIALOG( dialog_font_selection )->cancel_button, 1 );

	/* Show&Run dialog. */
	gtk_widget_show( dialog_font_selection );
}


