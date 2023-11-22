/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: about.c 19 2004-07-29 02:45:55Z jzajpt $
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
#include "metathemeInt.h"
#include "main.h"
#include "about.h"


GtkWidget*
about_page_new( void )
{
	GtkWidget* table       = NULL;
	GtkWidget* label_about = NULL;

	table = gtk_table_new( 1, 1, TRUE );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 12 );

	label_about = gtk_label_new( NULL );
	gtk_label_set_justify ( GTK_LABEL( label_about ), GTK_JUSTIFY_CENTER );
	gtk_label_set_selectable( GTK_LABEL( label_about ), TRUE );
	gtk_label_set_markup( GTK_LABEL( label_about ),
		"<span size=\"x-large\"><b>MetaTheme Configuration Tool</b></span>\n\n"
		"An utility for changing look'n'feel of MetaTheme.\n\n\n"
		"<small>Copyright \302\251 2004 Jirka Zajpt &lt;jzajpt@blueberry.cz&gt;</small>"
	);
	gtk_table_attach( GTK_TABLE( table ), label_about, 0, 1, 0, 1,
		GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );

	return table;
}

