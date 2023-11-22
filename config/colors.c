/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: colors.c 19 2004-07-29 02:45:55Z jzajpt $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "metathemeInt.h"
#include "main.h"
#include "util.h"
#include "colors.h"

static void       color_scheme_changed( GtkWidget* widget );
static void       widget_changed( GtkWidget* widget );
static GtkWidget* button_with_color_new( GdkColor* color_ptr );
static gboolean   expose_event( GtkWidget* widget, GdkEventExpose* event, gpointer data );
static void       color_button_clicked( GtkWidget* widget );
static void       color_selected( GtkWidget* widget, gpointer data );
static void       save_as_clicked( GtkWidget* widget );
static void       remove_clicked( GtkWidget* widget );


/* GtkOptionMenu which contains color schemes. */
GtkWidget*  optmenu_color_schemes  = NULL;
/* GtkOptionMenu which contains widget types. */
GtkWidget*  optmenu_widgets        = NULL;
/* "Save As" button. */
GtkWidget*  button_save_as         = NULL;
/* "Remove" button. */
GtkWidget*  button_remove          = NULL;
/* Button with foreground color. */
GtkWidget*  button_foreground      = NULL;
/* Button with background color. */
GtkWidget*  button_background      = NULL;
/* Pointer to color selection dialog. */
GtkWidget*  dialog_color_selection = NULL;
/* Current color of foreground button. */
GdkColor    current_fg_color       = { 0, 0, 0, 0 };
/* Current color of background button. */
GdkColor    current_bg_color       = { 0, 0, 0, 0 };


GtkWidget*
colors_page_new( void )
{
	GtkWidget*   main_table              = NULL;
	GtkWidget*   label                   = NULL;
	GtkWidget*   hbox                    = NULL;
	GtkTooltips* tooltips                = NULL;
	/* List which contains names of all available color schemes. */
	GSList*      list_color_scheme_names = NULL;
	/* List which contains widget types. */
	GSList*      list_widget_names       = NULL;
	int          i;

	/* Create main layout table. */
	main_table = gtk_table_new( 4, 2, FALSE );
	gtk_container_set_border_width( GTK_CONTAINER( main_table ), 12 );
	gtk_table_set_row_spacings( GTK_TABLE( main_table ), 6 );
	gtk_table_set_col_spacings( GTK_TABLE( main_table ), 6 );

	/* Create list with names of all available color schemes
	 * and find current color scheme number in this list.
	 */
	for ( i = 0 ; i < g_list_length( list_color_schemes ) ; i++ )
	{
		MTCONFIG_COLOR_SCHEME* color_scheme =
			g_list_nth_data( list_color_schemes, i );

		if ( strcmp( color_scheme->name, current_settings.color_scheme->name ) == 0 )
			settings_set_current_color_scheme( i );

		list_color_scheme_names = g_slist_append( list_color_scheme_names, color_scheme->name );
	}

	/* Create labels. */
	label = gtk_label_new( _( "Widget type: " ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( main_table ), label, 0, 1, 1, 2,
		GTK_FILL, GTK_FILL, 0, 0 );

	label = gtk_label_new( _( "Foreground: " ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( main_table ), label, 0, 1, 2, 3,
		GTK_FILL, GTK_FILL, 0, 0 );

	label = gtk_label_new( _( "Background: " ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_table_attach( GTK_TABLE( main_table ), label, 0, 1, 3, 4,
		GTK_FILL, GTK_FILL, 0, 0 );

	hbox = gtk_hbox_new( FALSE, 6 );
	gtk_table_attach( GTK_TABLE( main_table ), hbox, 0, 2, 0, 1,
		GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );

	/* Get names of color schemes and create GtkOptionMenu with theme. */
	optmenu_color_schemes = optionmenu_from_gslist( list_color_scheme_names );
	g_slist_free( list_color_scheme_names );
	gtk_size_group_add_widget( vsizegroup, optmenu_color_schemes );
	gtk_option_menu_set_history( GTK_OPTION_MENU( optmenu_color_schemes ), current_settings.current_color_scheme );
	gtk_box_pack_start( GTK_BOX( hbox ), optmenu_color_schemes, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( optmenu_color_schemes ), "changed",
		G_CALLBACK( color_scheme_changed ), NULL );

	/* Create list with widget names and crete GtkOptionMenu with theme. */
	list_widget_names = g_slist_append( list_widget_names, _( "Base" ) );
	list_widget_names = g_slist_append( list_widget_names, _( "Window" ) );
	list_widget_names = g_slist_append( list_widget_names, _( "Button" ) );
	list_widget_names = g_slist_append( list_widget_names, _( "Selected" ) );
	list_widget_names = g_slist_append( list_widget_names, _( "Disabled" ) );
	optmenu_widgets = optionmenu_from_gslist( list_widget_names );
	g_slist_free( list_widget_names );
	gtk_size_group_add_widget( vsizegroup, optmenu_widgets );
	gtk_option_menu_set_history( GTK_OPTION_MENU( optmenu_widgets ), current_settings.current_widget );
	gtk_table_attach( GTK_TABLE( main_table ), optmenu_widgets, 1, 2, 1, 2,
		GTK_FILL | GTK_EXPAND, 0, 0, 0 );
	g_signal_connect( G_OBJECT( optmenu_widgets ), "changed",
		G_CALLBACK( widget_changed ), NULL );

	/* Create Save As button. */
	button_save_as = gtk_button_new_with_label( _( "Save As" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), button_save_as, FALSE, FALSE, 0 );
	gtk_size_group_add_widget( vsizegroup, button_save_as );
	g_signal_connect( G_OBJECT( button_save_as ), "clicked",
		G_CALLBACK( save_as_clicked ), NULL );

	/* Create Remove button. */
	button_remove = gtk_button_new_with_label( _( "Remove" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), button_remove, FALSE, FALSE, 0 );
	gtk_size_group_add_widget( vsizegroup, button_remove );
	g_signal_connect( G_OBJECT( button_remove ), "clicked",
		G_CALLBACK( remove_clicked), NULL );

	/* Create button with color - foreground color. */
	button_foreground = button_with_color_new( &current_fg_color );
	gtk_widget_set_name( button_foreground, "button_foreground" );
	gtk_size_group_add_widget( vsizegroup, button_foreground );
	gtk_table_attach( GTK_TABLE( main_table ), button_foreground, 1, 2, 2, 3,
		GTK_FILL, GTK_FILL, 0, 0 );
	g_signal_connect( G_OBJECT( button_foreground ), "clicked",
		G_CALLBACK( color_button_clicked ), NULL );

	/* Create button with color - background color. */
	button_background = button_with_color_new( &current_bg_color );
	gtk_widget_set_name( button_background, "button_background" );
	gtk_size_group_add_widget( vsizegroup, button_background );
	gtk_table_attach( GTK_TABLE( main_table ), button_background, 1, 2, 3, 4,
		GTK_FILL, GTK_FILL, 0, 0 );
	g_signal_connect( G_OBJECT( button_background ), "clicked",
		G_CALLBACK( color_button_clicked ), NULL );

	/* Set tooltips. */
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip( tooltips, optmenu_widgets,
		_( "Widget type." ),
		NULL
	);
	gtk_tooltips_set_tip( tooltips, button_foreground,
		_( "Foreground color of widget." ),
		NULL
	);
	gtk_tooltips_set_tip( tooltips, button_background,
		_( "Background color of widget." ),
		NULL
	);

	color_scheme_changed( optmenu_color_schemes );
	widget_changed( optmenu_widgets );

	return main_table;
}


static void
color_scheme_changed( GtkWidget* widget )
{
	MTCONFIG_COLOR_SCHEME* color_scheme;

	settings_set_current_color_scheme( gtk_option_menu_get_history( GTK_OPTION_MENU( widget ) ) );
	color_scheme = g_list_nth_data( list_color_schemes, current_settings.current_color_scheme );

	settings_set_color_scheme( color_scheme );

	gtk_widget_set_sensitive( button_remove, color_scheme->removable );

	/* Redraw buttons with new colors */
	widget_changed( optmenu_widgets );
}


static GdkColor*
mtcolor_to_gdkcolor( MT_COLOR* mtcolor, GdkColor* output )
{
	output->red = mtcolor->r << 8;
	output->green = mtcolor->g << 8;
	output->blue = mtcolor->b << 8;

	return output;
}


/*
 * Makes string rgb representation from Gdk Color
 * Returned pointer should be freed with free().
 */
static char*
make_str_rgb( GdkColor* color )
{
	char* output = ( char* ) malloc( 19 * sizeof( char ) );
	ALLOC_CHECK( output );
	snprintf( output, 18, "%d, %d, %d",
		color->red >> 8,
		color->green >> 8,
		color->blue >> 8 );
	return output;
}


static void
widget_changed( GtkWidget* widget )
{
	char     name_bg[ 12 ];
	char     name_fg[ 12 ];
	MT_COLOR tmp_color;

	settings_set_current_widget( gtk_option_menu_get_history( GTK_OPTION_MENU( widget ) ) );

	switch ( current_settings.current_widget )
	{
	case 0:
		strncpy( name_fg, "base_fg", 12 );
		strncpy( name_bg, "base_bg", 12 );
		break;
	case 1:
		strncpy( name_fg, "window_fg", 12 );
		strncpy( name_bg, "window_bg", 12 );
		break;
	case 2:
		strncpy( name_fg, "button_fg", 12 );
		strncpy( name_bg, "button_bg", 12 );
		break;
	case 3:
		strncpy( name_fg, "selected_fg", 12 );
		strncpy( name_bg, "selected_bg", 12 );
		break;
	case 4:
		strncpy( name_fg, "disabled_fg", 12 );
		strncpy( name_bg, "disabled_bg", 12 );
		break;
	}

	/* Get foreground color of given widget type. */
	metatheme_color_set_from_config( current_settings.color_scheme->conf, &tmp_color,
		name_fg, NULL );
	mtcolor_to_gdkcolor( &tmp_color, &current_fg_color );

	/* Get background color of given widget type. */
	metatheme_color_set_from_config( current_settings.color_scheme->conf, &tmp_color,
		name_bg, NULL );
	mtcolor_to_gdkcolor( &tmp_color, &current_bg_color );

	/* Redraw buttons. */
	gtk_widget_queue_draw( button_foreground );
	gtk_widget_queue_draw( button_background );
}


static GtkWidget*
button_with_color_new( GdkColor* color_ptr )
{
	GtkWidget* button    = NULL;
	GtkWidget* draw_area = NULL;

	button = gtk_button_new();

	draw_area = gtk_drawing_area_new();
	gtk_container_add( GTK_CONTAINER( button ), draw_area );
	g_signal_connect( G_OBJECT( draw_area ), "expose_event",
		G_CALLBACK( expose_event ), color_ptr );

	return button;
}


static gboolean
expose_event( GtkWidget* widget, GdkEventExpose* event, gpointer data )
{
	GdkGCValues current_gc_values;

	/* Retrieve current GC values. */
	gdk_gc_get_values( widget->style->fg_gc[ GTK_WIDGET_STATE( widget ) ], 	&current_gc_values );

	/* Set foreground color. */
	gdk_gc_set_rgb_fg_color( widget->style->fg_gc[ GTK_WIDGET_STATE( widget ) ],
		( GdkColor* ) data );

	gdk_draw_rectangle( widget->window, widget->style->fg_gc[ GTK_WIDGET_STATE( widget ) ],
		TRUE, 0, 0, widget->allocation.width, widget->allocation.height );

	/* Set original GC foreground. */
	gdk_gc_set_values( widget->style->fg_gc[ GTK_WIDGET_STATE( widget ) ],
		&current_gc_values, GDK_GC_FOREGROUND );

	return TRUE;
}


static void
color_button_clicked( GtkWidget* widget )
{
	GtkBox*     box         = NULL;
	GdkColor*   color       = NULL;
	const char* widget_name = gtk_widget_get_name( widget );

	dialog_color_selection = gtk_color_selection_dialog_new( _( "Color selection" ) );
	gtk_window_set_modal( GTK_WINDOW( dialog_color_selection ), TRUE );
	gtk_window_set_transient_for( GTK_WINDOW( dialog_color_selection ), GTK_WINDOW( main_window ) );

	if ( strcmp( widget_name, "button_foreground" ) == 0 )
		color = &current_fg_color;
	else if ( strcmp( widget_name, "button_background" ) == 0 )
		color = &current_bg_color;

	gtk_color_selection_set_current_color(
		GTK_COLOR_SELECTION( GTK_COLOR_SELECTION_DIALOG( dialog_color_selection )->colorsel ),
		color );
	g_signal_connect( G_OBJECT( GTK_COLOR_SELECTION_DIALOG( dialog_color_selection )->ok_button ),
		"clicked", G_CALLBACK( color_selected ), ( gpointer ) widget_name );
	g_signal_connect_swapped( G_OBJECT( GTK_COLOR_SELECTION_DIALOG( dialog_color_selection )->cancel_button ),
		"clicked", G_CALLBACK( gtk_widget_destroy ), G_OBJECT( dialog_color_selection ) );

	/* Swap OK and Cancel buttons. */
	box = GTK_BOX( ( GTK_COLOR_SELECTION_DIALOG( dialog_color_selection )->ok_button )-> parent );
	gtk_box_reorder_child( box, GTK_COLOR_SELECTION_DIALOG( dialog_color_selection )->ok_button, 0 );
	gtk_box_reorder_child( box, GTK_COLOR_SELECTION_DIALOG( dialog_color_selection )->cancel_button, 1 );

	gtk_widget_show( dialog_color_selection );
}


static void
color_selected( GtkWidget* widget, gpointer data )
{
	GtkWidget* color_selection = GTK_COLOR_SELECTION_DIALOG( dialog_color_selection )->colorsel;
	GdkColor   color;
	char       widget_name[ 12 ];
	char*      rgb_color;

	gtk_color_selection_get_current_color( GTK_COLOR_SELECTION( color_selection ), &color );

	switch ( current_settings.current_widget )
	{
	case 0:
		strncpy( widget_name, "base_", 12 );
		break;
	case 1:
		strncpy( widget_name, "window_", 12 );
		break;
	case 2:
		strncpy( widget_name, "button_", 12 );
		break;
	case 3:
		strncpy( widget_name, "selected_", 12 );
		break;
	case 4:
		strncpy( widget_name, "disabled_", 12 );
		break;
	}

	if ( strcmp( data, "button_foreground" ) == 0 )
	{
		strcat( widget_name, "fg" );
		memcpy( &current_fg_color, &color, sizeof( color ) );
	}
	else if ( strcmp( data, "button_background" ) == 0 )
	{
		strcat( widget_name, "bg" );
		memcpy( &current_bg_color, &color, sizeof( color ) );
	}

	rgb_color = make_str_rgb( &color );
	metatheme_set_config_option( current_settings.color_scheme->conf, widget_name, NULL, rgb_color );
	free( rgb_color );
	current_settings.color_scheme->modified = TRUE;

	gtk_widget_destroy( dialog_color_selection );
}


static void
save_as_clicked( GtkWidget* widget )
{
	GtkWidget*             menu_colors   = NULL;
	GtkWidget*             new_menu_item = NULL;
	MTCONFIG_COLOR_SCHEME* new_scheme    = NULL;
	char *new_name = get_name_dialog( main_window,
		_( "Save Color Scheme As" ),
		_( "Enter name for color scheme: " ) );

	/* If dialog was closed or empty string was entered. */
	if ( ( new_name == NULL ) || ( strcmp( new_name, "" ) == 0 ) )
		return;

	/* Create copy of current color scheme. */
	new_scheme = copy_color_scheme( current_settings.color_scheme );

	/* Set options which are different from parent. */
	new_scheme->name = strdup( new_name );
	new_scheme->file = strdup( new_name );
	new_scheme->path = strdup( metatheme_user_colors_dir );
	new_scheme->modified = FALSE;
	new_scheme->removable = TRUE;
	metatheme_set_config_option( new_scheme->conf, "name", NULL, new_name );

	/* Save new color scheme. */
	save_color_scheme( new_scheme );

	/* Append newly created color scheme to list with all color schemes .*/
	list_color_schemes = g_list_append( list_color_schemes, new_scheme );

	/* Add new color scheme name to the optionmenu and select it. */
	menu_colors = gtk_option_menu_get_menu( GTK_OPTION_MENU( optmenu_color_schemes ) );
	new_menu_item = gtk_menu_item_new_with_label( new_name );
	gtk_menu_shell_append( GTK_MENU_SHELL( menu_colors ), new_menu_item );
	gtk_option_menu_set_history( GTK_OPTION_MENU( optmenu_color_schemes ), g_list_length( list_color_schemes ) - 1 );
	gtk_option_menu_set_menu( GTK_OPTION_MENU( optmenu_color_schemes ), menu_colors );
	gtk_widget_show_all( optmenu_color_schemes );

	free( new_name );
}


static void
remove_clicked( GtkWidget* widget )
{
	GtkWidget*             menu_colors  = NULL;
	GtkWidget*             menu_item    = NULL;
	MTCONFIG_COLOR_SCHEME* color_scheme = g_list_nth_data( list_color_schemes, current_settings.current_color_scheme );
	char                   color_scheme_file_name[ 1024 ];

	snprintf( color_scheme_file_name, 1024, "%s%s", color_scheme->path, color_scheme->file );

	/* Remove selected color scheme item from menu. */
	menu_colors = gtk_option_menu_get_menu( GTK_OPTION_MENU( optmenu_color_schemes ) );
	menu_item = gtk_menu_get_active( GTK_MENU( menu_colors ) );
	gtk_container_remove( GTK_CONTAINER( menu_colors ), menu_item );

	/* Remove color scheme from list_color_schemes and free memory. */
	list_color_schemes = g_list_remove( list_color_schemes, color_scheme );
	free_color_scheme( color_scheme );

	/* Remove color scheme configuration file from filesystem. */
	remove( color_scheme_file_name );

	/* Select first item in menu. */
	gtk_option_menu_set_history( GTK_OPTION_MENU( optmenu_color_schemes ), 0 );
	gtk_widget_show_all( optmenu_color_schemes );

}
