/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: main.c 19 2004-07-29 02:45:55Z jzajpt $
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
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "metathemeInt.h"
#include "about.h"
#include "colors.h"
#include "color_scheme.h"
#include "fonts.h"
#include "preview.h"
#include "themes.h"
#include "util.h"
#include "main.h"


static int      load_metatheme_config( void );
static void     save_metatheme_config( void );
static int      load_themes( char* dir, GList** list );
static int      load_colors( char* dir, gboolean removable, GList** list );
static int      mtconfig_init( void );
static gboolean mtconfig_quit( gpointer data );
static void     ok_clicked( GtkWidget* widget, gpointer* data );
static void     apply_clicked( GtkWidget* widget, gpointer* data );
static void     cancel_clicked( GtkWidget* widget, gpointer* data );
static void     signal_handler( int signum );


/* User's private metatheme directory, Ussualy $HOME/.metatheme. */
char metatheme_user_dir[ 1024 ];
/* User's metatheme config file. Ussualy $HOME/.metatheme/config. */
char metatheme_user_config_file[ 1024 ];
/* User's private directory with color schemes. Ussualy $HOME/.metatheme/colors. */
char metatheme_user_colors_dir[ 1024 ];

/* Pointer to main window widget. */
GtkWidget*       main_window        = NULL;
GtkWidget*       button_apply       = NULL;
/* Vertical SizeGroup for buttons and option menus. */
GtkSizeGroup*    vsizegroup         = NULL;
/* List which contains all available themes. */
GList*           list_themes_info   = NULL;
/* List which contains all available color schemes. */
GList*           list_color_schemes = NULL;
MT_CONFIG*       metatheme_conf;
gboolean         is_embed           = FALSE;
int              fd[ 2 ];
pid_t            parent_pid;


/*
 * Invokes the 'metatheme-install' script.
 */
static gboolean
metatheme_install( void )
{
	GtkWidget *dlg;
	gint response;

	dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		_("Metatheme is not activated. Activation might have effect of crashing all programs in this X session. "
		  "Please save all your unsaved work before proceeding.\n\n"
		  "Do you want to activate MetaTheme?"));

	response = gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);

	if (response != GTK_RESPONSE_ACCEPT) return FALSE;

	system("metatheme-install -q");

	return TRUE;
}


/*
 * Loads settings from metatheme configuration file.
 * Returns non-zero in case of failure, otherwise return 0.
 */
static int
load_metatheme_config( void )
{
	const char*  ptr;
	unsigned int i;

retry:
	/* Load metatheme configuration. */
	metatheme_conf = metatheme_load_config( metatheme_user_config_file );
	if ( !metatheme_conf )
	{
		printf( _( "Warning: Unable to load metatheme config file! (%s)\n" ), metatheme_user_config_file );

		/* No metatheme configuration file was found, so we invoke
		 * the 'metatheme-install' script.
		 */
		if (!metatheme_install()) return -1;
		goto retry;
	}

	/* Load theme name and application font name from configuration file. */
	settings_set_theme( metatheme_get_config_option( metatheme_conf, "theme", NULL ) );
	settings_set_application_font( metatheme_get_config_option( metatheme_conf, "font", NULL ) );

	/* Get name of color scheme from configuration file, then we need to
	 * find corresponding color scheme in list_color_scheme and copy it to
	 * current_scheme.scheme.
	 */
	ptr = metatheme_get_config_option( metatheme_conf, "colors", NULL );
	for ( i = 0 ; i < g_list_length( list_color_schemes ) ; i++ )
	{
		MTCONFIG_COLOR_SCHEME* scheme = g_list_nth_data( list_color_schemes, i );

		if ( strcmp( ptr, scheme->file ) == 0 )
		{
			settings_set_color_scheme( scheme );
		}
	}

	/* If color scheme from metatheme configuration doesn't
	 * exist or we just haven't found it, we have to choose
	 * first color scheme to prevent segfault.
	 */
	if ( current_settings.color_scheme == NULL )
		settings_set_color_scheme( g_list_nth_data( list_color_schemes, 0 ) );

	return 0;
}


/*
 * Saves current settings to metatheme configuration file. Also saves
 * current color scheme if was modified in some way.
 */
static void
save_metatheme_config( void )
{
	char       color_scheme_file_name[ 1024 ];
	MT_CONFIG* conf;

	conf = metatheme_load_config( metatheme_user_config_file );

	metatheme_set_config_option( conf, "theme", NULL, current_settings.theme );
	metatheme_set_config_option( conf, "colors", NULL, current_settings.color_scheme->file );
	metatheme_set_config_option( conf, "font", NULL, current_settings.application_font );
	metatheme_save_config( conf, metatheme_user_config_file );
	metatheme_free_config( conf );

	if ( current_settings.color_scheme->modified == TRUE )
	{
		/* If color scheme was modified, try to save it in
		 * its original location. If this fails, try to save
		 * it in user's private metatheme directory.
		 */
		snprintf( color_scheme_file_name, 1024, "%s%s",
			current_settings.color_scheme->path,
			current_settings.color_scheme->file );
		if ( ! metatheme_save_config( current_settings.color_scheme->conf, color_scheme_file_name ) )
		{
			/* It's a possible that colors dir doesn't exist, so we
			 * try to create it just for sure.
			 */
			mkdir( metatheme_user_colors_dir, S_IRWXU );

			snprintf( color_scheme_file_name, 1024, "%s%s",
				metatheme_user_colors_dir,
				current_settings.color_scheme->file );

			metatheme_save_config( current_settings.color_scheme->conf, color_scheme_file_name );
		}
	}
}


/*
 * Loads all themerc files from directory pointed by dir.
 * Results are appended into list.
 */
static int
load_themes( char* dir, GList** list )
{
	struct dirent* direntry   = NULL;
	DIR*           themes_dir = NULL;

	/* Open directory with themes. */
	themes_dir = opendir( dir );
	if ( !themes_dir )
		return -1;

	/* Read all items in metatheme themes directory. */
	while ( ( direntry = readdir( themes_dir ) ) != NULL )
	{
		/* Skip . and .. and regular files. */
		if ( ( strcmp( direntry->d_name, "." ) != 0 ) &&
			( strcmp( direntry->d_name, ".." ) != 0 ) )
		{
			MT_CONFIG*           themerc;
			MTCONFIG_THEME_INFO* info;
			char                 buffer[ 1024 ];
			char*                tmp;

			snprintf( buffer, 1024, "%s%s%s", dir, direntry->d_name, "/themerc" );

			/* Attempt to load themerc file. */
			themerc = metatheme_load_config( buffer );

			/* If we're unable to open file for any reason,
			 * we gently write out warning and skip this...
			 */
			if ( themerc == NULL )
			{
				printf( _( "Warning: Unable to load themerc file (%s)\n" ), buffer );
				continue;
			}

			info = ( MTCONFIG_THEME_INFO* ) malloc( sizeof( MTCONFIG_THEME_INFO ) );
			ALLOC_CHECK( info );

			/* Load required values from themerc file. */
			info->name = strdup( metatheme_get_config_option( themerc, "name", NULL ) );
			info->engine = strdup( metatheme_get_config_option( themerc, "engine", NULL ) );
			info->description = strdup( metatheme_get_config_option( themerc, "description", NULL ) );
			info->author = strdup( metatheme_get_config_option( themerc, "author", NULL ) );
			info->directory = strdup( direntry->d_name );
			tmp = metatheme_get_config_option( themerc, "config", NULL );
			info->config = tmp ? strdup( tmp ) : NULL;

			/* Append structure with theme informations to list. */
			*list = g_list_append( *list, info );

			metatheme_free_config( themerc );
		}
	}
	closedir( themes_dir );

	return 0;
}


/*
 * Loads all available color schemes from directory pointed
 * by variable dir and appends it to the list.
 */
static int
load_colors( char* dir, gboolean removable, GList** list )
{
	struct dirent* direntry   = NULL;
	DIR*           colors_dir = NULL;

	/* Open directory with color schemes. */
	colors_dir = opendir( dir );
	if ( !colors_dir )
		return -1;

	while ( ( direntry = readdir( colors_dir ) ) != NULL )
	{
		/* If we have regular file... */
		if ( ( strcmp( direntry->d_name, "." ) != 0 ) &&
			( strcmp( direntry->d_name, ".." ) != 0 ) )
		{
			MTCONFIG_COLOR_SCHEME* scheme;
			int                    i;
			char                   path[ 1024 ];
			MT_CONFIG*             colors_conf = NULL;
			gboolean               append      = TRUE;

			snprintf( path, 1024, "%s%s", dir, direntry->d_name );

			/* Load color scheme configuration file. */
			colors_conf = metatheme_load_config( path );
			if ( colors_conf == NULL )
			{
				printf( _( "Warning: Unable to load colors file (%s)\n" ), path );
				continue;
			}

			/* Check if color scheme configuration file has all required items. */
			if ( check_colors_conf( colors_conf ) == FALSE )
			{
				metatheme_free_config( colors_conf );
				printf( _( "Warning: %s is not valid color scheme!\n" ), path );
				continue;
			}

			/*
			 * Check if color scheme with same name doesn't already
			 * exists...
			 */
			for ( i = 0 ; i < g_list_length( *list ) ; i++ )
			{
				MTCONFIG_COLOR_SCHEME* ptr = g_list_nth_data( *list, i );
				if ( strcmp( ptr->file, direntry->d_name ) == 0 )
					append = FALSE;
			}
			if ( append == FALSE )
			{
				metatheme_free_config( colors_conf );
				continue;
			}

			scheme = ( MTCONFIG_COLOR_SCHEME* ) malloc( sizeof( MTCONFIG_COLOR_SCHEME ) );
			ALLOC_CHECK( scheme );

			scheme->file = strdup( direntry->d_name );
			scheme->path = strdup( dir );
			scheme->name = strdup( metatheme_get_config_option( colors_conf, "name", NULL ) );
			scheme->modified = FALSE;
			scheme->removable = removable;
			scheme->conf = colors_conf;

			*list  = g_list_append( *list, scheme );
		}
	}

	return 0;
}


/*
 * Initialize mt-config internal stuff.
 *
 * Returns zero in case success, otherwise returns non-zero.
 */
static int
mtconfig_init( void )
{
	snprintf( metatheme_user_dir, 1024, "%s%s", getenv( "HOME" ), "/.metatheme" );
	snprintf( metatheme_user_config_file, 1024, "%s%s", metatheme_user_dir, "/config" );
	snprintf( metatheme_user_colors_dir, 1024, "%s%s", metatheme_user_dir, "/colors/" );

	settings_initialize();

	/* Load themes... */
	load_themes( METATHEMEDIR "/themes/", &list_themes_info );
	/* If no themes was found, we popup a error dialog and then exit. */
	if ( list_themes_info == NULL )
	{
		error_dialog( _( "Unable to find any working themes!" ) );

		printf( _( "Error: Unable to find any working themes!\n" ) );
		return -1;
	}

	/* First, we have to load color schemes from user's metatheme private
	 * directory.
	 */
	load_colors( metatheme_user_colors_dir, TRUE, &list_color_schemes );

	/* Then, load colors from system-wide metatheme directory. */
	load_colors( METATHEMEDIR "/colors/", FALSE, &list_color_schemes );

	/* Finally, load metatheme configuration. */
	if ( load_metatheme_config() < 0 )
		return -1;

	if ( is_embed )
		signal( SIGUSR1, signal_handler );

	return 0;
}


static void
free_theme_info( MTCONFIG_THEME_INFO* theme_info )
{
	mtconfig_free( theme_info->name );
	mtconfig_free( theme_info->engine );
	mtconfig_free( theme_info->author );
	mtconfig_free( theme_info->description );
	mtconfig_free( theme_info->directory );
	mtconfig_free( theme_info->config );
	mtconfig_free( theme_info );
}


static gboolean
mtconfig_quit( gpointer data )
{
	int i;

		/* Free all theme informations. */
	for ( i = 0 ; i < g_list_length( list_themes_info ) ; i++ )
		free_theme_info( g_list_nth_data( list_themes_info, i ) );
	g_list_free( list_themes_info );
	list_themes_info = NULL;

	/* Free all color schemes. */
	for ( i = 0 ; i < g_list_length( list_color_schemes ) ; i++ )
		free_color_scheme( g_list_nth_data( list_color_schemes, i ) );
	g_list_free( list_color_schemes );
	list_color_schemes = NULL;

	settings_destruct();

	metatheme_free_config( metatheme_conf );

	return TRUE;
}


static void
refresh( void )
{
	GtkWidget *dlg;
	gint response;

	dlg = gtk_message_dialog_new(main_window, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
		_("The on-the-fly switching of themes is currently an experimental feature. It might crash "
		  "all your applications in this X session. Please save all your unsaved work before proceeding.\n\n"
		  "Do you really want to continue?"));

	response = gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);

	if (response != GTK_RESPONSE_YES) return;
	
	gdk_error_trap_push();
	metatheme_refresh( gdk_x11_display_get_xdisplay( gdk_display_get_default() ) );
	gdk_flush();
	gdk_error_trap_pop();
}


static void
ok_clicked( GtkWidget *widget, gpointer* data )
{
	save_metatheme_config();
	/*
	TODO:
	refresh();
	gtk_main_quit();
	*/
}


static void
apply_clicked( GtkWidget* widget, gpointer* data )
{
	save_metatheme_config();
	refresh();
}


static void
cancel_clicked( GtkWidget* widget, gpointer* data )
{
	gtk_main_quit();
}

void
config_changed( void )
{
	if ( is_embed )
		kill( parent_pid, SIGUSR1 );
	else
		gtk_widget_set_sensitive( button_apply, TRUE );
	/* no write now 'cause we have only one type of action
	write( fd[ 1 ], &height, sizeof( int ) );
	*/
}



#define ACTION_LOAD 0x1
#define ACTION_SAVE 0x2
static void
signal_handler( int signum )
{
	int action;
	read( fd[ 0 ], &action, sizeof( action ) );
	switch( action )
	{
		case ACTION_LOAD:
			load_metatheme_config();
			break;
		case ACTION_SAVE:
			save_metatheme_config();
			break;
		default:
			return;
	}
}


/*
 * Creates new metatheme-config window.
 */
int
create_window( void )
{
	GtkWidget* vbox                  = NULL;
	GtkWidget* notebook              = NULL;
	GtkWidget* page_label            = NULL;
	GtkWidget* theme_selection_page  = NULL;
	GtkWidget* colors_selection_page = NULL;
	GtkWidget* font_selection_page   = NULL;
	GtkWidget* about_page            = NULL;
	GtkWidget* bbox_buttons          = NULL;
	GtkWidget* button_ok             = NULL;
	GtkWidget* button_cancel         = NULL;
	GtkWidget* preview               = NULL;


	if ( !is_embed )
	{
		/* Create window and set what is needed.*/
		main_window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
		gtk_window_set_title( GTK_WINDOW( main_window ), _( "MetaTheme Configuration Tool" ) );
		gtk_container_set_border_width( GTK_CONTAINER( main_window ), 6 );
		gtk_window_set_policy( GTK_WINDOW( main_window ), FALSE, FALSE, FALSE );
		gtk_window_set_position( GTK_WINDOW( main_window ), GTK_WIN_POS_CENTER );
	}
	else
		main_window = gtk_plug_new( 0 );
	g_signal_connect( G_OBJECT( main_window ), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );
	/*g_signal_connect( G_OBJECT( main_window ), "destroy", G_CALLBACK( gtk_main_quit ), NULL );*/
	gtk_quit_add( 0, mtconfig_quit, NULL );

	vbox = gtk_vbox_new( FALSE, 6 );
	gtk_container_add( GTK_CONTAINER( main_window ), vbox );

	/* Create new GtkSizeGroup for all option menus and buttons. */
	vsizegroup = gtk_size_group_new( GTK_SIZE_GROUP_VERTICAL );

	/* Create GtkNotebook. */
	notebook = gtk_notebook_new();
	gtk_box_pack_start( GTK_BOX( vbox ), notebook, FALSE, FALSE, 0 );

	/* Create themes page. */
	page_label = gtk_label_new( _( "Themes" ) );
	theme_selection_page = theme_selection_page_new();
	gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), theme_selection_page, page_label );

	/* Create color schemes page. */
	page_label = gtk_label_new( _( "Colors" ) );
	colors_selection_page = colors_page_new();
	gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), colors_selection_page, page_label );

	/* Create fonts page. */
	page_label = gtk_label_new( _( "Fonts" ) );
	font_selection_page = font_selection_page_new();
	gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), font_selection_page, page_label );

	/* Create about page. */
	page_label = gtk_label_new( _( "About" ) );
	about_page = about_page_new();
	gtk_notebook_append_page( GTK_NOTEBOOK( notebook ), about_page, page_label );

	/* Create preview. */
	preview = preview_new();
	gtk_box_pack_start( GTK_BOX( vbox ), preview, FALSE, FALSE, 0 );

	if ( is_embed == FALSE )
	{
		bbox_buttons = gtk_hbutton_box_new();
		gtk_button_box_set_layout( GTK_BUTTON_BOX( bbox_buttons ), GTK_BUTTONBOX_END );
		gtk_button_box_set_spacing( GTK_BUTTON_BOX( bbox_buttons ), 6 );
		gtk_box_pack_start( GTK_BOX( vbox ), bbox_buttons, FALSE, FALSE, 0 );

		/* Create OK button, not from stock. */
		button_ok = gtk_button_new_with_label( _( "Save" ) );
		gtk_box_pack_start( GTK_BOX( bbox_buttons ), button_ok, FALSE, FALSE, 0 );
		g_signal_connect( G_OBJECT( button_ok ), "clicked", G_CALLBACK( ok_clicked ), NULL );

		/* Create APPLY button, not from stock. */
		button_apply = gtk_button_new_with_label( _( "Apply" ) );
		gtk_widget_set_sensitive( button_apply, FALSE );
		gtk_box_pack_start( GTK_BOX( bbox_buttons ), button_apply, FALSE, FALSE, 0 );
		g_signal_connect( G_OBJECT( button_apply ), "clicked", G_CALLBACK( apply_clicked ), NULL );

		/* Crete CANCEL button, not from stock. */
		button_cancel = gtk_button_new_with_label( _( "Exit" ) );
		gtk_box_pack_start( GTK_BOX( bbox_buttons ), button_cancel, FALSE, FALSE, 0 );
		g_signal_connect( G_OBJECT( button_cancel ), "clicked", G_CALLBACK( cancel_clicked ), NULL );
	}

	/* Show all created widgets. */
	gtk_widget_show_all( main_window );

	if ( is_embed == TRUE )
	{
		int width, height;
		char buffer[ 256 ];
		int id = gdk_x11_drawable_get_xid( main_window->window );

		gtk_window_get_size( GTK_WINDOW( main_window ), &width, &height );
		snprintf( buffer, 256, "%d\n", id );
		write( fd[ 1 ], buffer, strlen( buffer ) );
		write( fd[ 1 ], &width, sizeof( int ) );
		write( fd[ 1 ], &height, sizeof( int ) );
	}
	settings_enable_watch( config_changed );

	return 0;
}


int main( int argc, char** argv )
{
	if ( gtk_init_check( &argc, &argv ) == FALSE )
	{
		printf( _( "Error: Unable to initialize GTK for some reason.\n" ) );
		return -1;
	}

	if ( argc == 5 )
	{
		if ( strcmp( argv[ 1 ], "--kcm-embed"  ) == 0 )
		{
			is_embed = TRUE;
			fd[ 0 ] = atoi( argv[ 2 ] );
			fd[ 1 ] = atoi( argv[ 3 ] );
			parent_pid = atoi( argv[ 4 ] );
		}
	}

	if ( mtconfig_init() < 0 )
	{
		printf( _( "Error: Unable to initialize mt-config internals.\n" ) );
		return -1;
	}

	if ( create_window() < 0 )
	{
		printf( _( "Error: Unable to create mt-cofnig window.\n" ) );
		return -1;
	}

	/* Finally, GTK's main loop. */
	gtk_main();

	return 0;
}
