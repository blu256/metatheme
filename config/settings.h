/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: settings.h 19 2004-07-29 02:45:55Z jzajpt $
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

#ifndef _METATHEME_CONFIG_SETTINGS__H
#define _METATHEME_CONFIG_SETTINGS__H

typedef struct
{
	char*                  application_font;
	char*                  theme;
	MTCONFIG_COLOR_SCHEME* color_scheme;

	int           current_color_scheme;
	int           current_widget;
	int           current_theme;
	gboolean      watch;
	void ( *notify_func )( void );
} MTCONFIG_CURRENT_SETTINGS;

extern MTCONFIG_CURRENT_SETTINGS current_settings;


void settings_initialize( void );
void settings_destruct( void );
void settings_enable_watch( void( *func )( void ) );
void settings_set_application_font( const char* name );
void settings_set_theme( const char* name );
void settings_set_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme );
void settings_set_current_color_scheme( int i );
void settings_set_current_widget( int i );
void settings_set_current_theme( int i );



#endif /* _METATHEME_CONFIG_SETTINGS__H */
