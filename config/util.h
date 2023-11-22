/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: util.h 19 2004-07-29 02:45:55Z jzajpt $
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

#ifndef _METATHEME_CONFIG_UTIL__H
#define _METATHEME_CONFIG_UTIL__H


/* No gettext for now. */
#define _( String ) String

/*
 * Checks whether pointer ptr has NULL values.
 * If so, print's error and exits application.
 */
#define ALLOC_CHECK( ptr ) \
	if ( ptr == NULL ) \
	{ \
		fprintf( stderr, _( "Error: Not enough memory!\n" ) ); \
		exit( 1 ); \
	}

/*
 * Frees pointer ptr with standart free() and
 * sets it to the NULL.
 */
#define mtconfig_free( ptr ) \
	free( ptr ); \
	ptr = NULL;


GtkWidget* optionmenu_new( char* values[], unsigned int num );
GtkWidget* optionmenu_from_gslist( GSList* list );
char*      get_name_dialog( GtkWidget* parent, char* dialog_title, char* dialog_text );
void       error_dialog( char* error_text );


#endif /* METATHEME_CONFIG_UTIL__H */
