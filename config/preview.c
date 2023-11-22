/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: preview.c 19 2004-07-29 02:45:55Z jzajpt $
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
#include "metathemeInt.h"
#include "util.h"
#include "main.h"
#include "preview.h"


GtkWidget*
preview_new( void )
{
	GtkWidget* preview_frame = NULL;
	GtkWidget* table         = NULL;
	GtkWidget* buttons_frame = NULL;
	GtkWidget* frame_vbox    = NULL;
	GtkWidget* check_button  = NULL;
	GtkWidget* radio_button  = NULL;
	GtkWidget* label_button  = NULL;
	GtkWidget* hscale        = NULL;
	GtkWidget* progress_bar  = NULL;
	GtkWidget* vscrollbar    = NULL;
	GtkWidget* option_menu   = NULL;
	GSList*     list          = NULL;

	preview_frame = gtk_frame_new( _( "Preview" ) );

	table = gtk_table_new( 3, 3, FALSE );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 6 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 6 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 6 );
	gtk_container_add( GTK_CONTAINER( preview_frame ), table );

	buttons_frame = gtk_frame_new( _( "Buttons" ) );
	gtk_table_attach_defaults( GTK_TABLE( table ), buttons_frame, 0, 1, 0, 3 );

	frame_vbox = gtk_vbox_new( FALSE, 6 );
	gtk_container_set_border_width( GTK_CONTAINER( frame_vbox ), 6 );
	gtk_container_add( GTK_CONTAINER( buttons_frame ), frame_vbox );

	/* Create check button. */
	check_button = gtk_check_button_new_with_label( _( "Check button" ) );
	gtk_box_pack_start( GTK_BOX( frame_vbox ), check_button, FALSE, FALSE, 0 );

	/* Create radio button. */
	radio_button = gtk_radio_button_new_with_label( NULL, _( "Radio button" ) );
	gtk_box_pack_start( GTK_BOX( frame_vbox ), radio_button, FALSE, FALSE, 0 );

	/* Create button with label. */
	label_button = gtk_button_new_with_label( _( "Button" ) );
	gtk_box_pack_start( GTK_BOX( frame_vbox ), label_button, FALSE, FALSE, 0 );

	/* Create Horizontal scale. */
	hscale = gtk_hscale_new_with_range( 0, 100, 1 );
	gtk_table_attach_defaults( GTK_TABLE( table ), hscale, 1, 2, 0, 1 );

	/* Create option menu. */
	list = g_slist_append( list, "Combobox" );
	list = g_slist_append( list, "Value 1" );
	list = g_slist_append( list, "Value 2" );
	list = g_slist_append( list, "Value 3" );
	option_menu = optionmenu_from_gslist( list );
	g_slist_free( list );
	gtk_table_attach( GTK_TABLE( table ), option_menu, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );

	/* Create progress bar. */
	progress_bar = gtk_progress_bar_new();
	gtk_progress_bar_set_text( GTK_PROGRESS_BAR( progress_bar ), "Progress Bar" );
	gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( progress_bar ), 0.5 );
	gtk_table_attach_defaults( GTK_TABLE( table ), progress_bar, 1, 2, 2, 3 );

	/* Create scroll bar. */
	vscrollbar = gtk_vscrollbar_new( NULL );
	gtk_table_attach( GTK_TABLE( table ), vscrollbar, 2, 3, 0, 3, GTK_SHRINK, GTK_FILL | GTK_EXPAND, 0, 0 );

	return preview_frame;
}
