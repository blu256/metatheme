/**
 * This file is part of MetaTheme.
 * Copyright (c) 2004 Martin Dvorak <jezek2@advel.cz>
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

/**
 * This file contains code from GTK toolkit.
 *
 * GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "mt_gtk2_patches.h"
#include "mt_gtk2.h"

#define PATCH_INSTALL(klass, type, name, field) \
   patch_install ((GtkObjectClass *)klass, offsetof(type, field), \
                  (GtkFunction) _ ## name ## _ ## field, (GtkFunction *) &old_ ## name ## _ ## field)

#define PATCH_RESTORE(klass, type, name, field) \
   patch_restore ((GtkObjectClass *)klass, offsetof(type, field), \
                  (GtkFunction) _ ## name ## _ ## field, (GtkFunction) old_ ## name ## _ ## field)

#define CHECK_STYLE(action) \
   if (!widget->style || !METATHEME_IS_STYLE(widget->style) || !METATHEME_STYLE(widget->style)->engine) { \
      action; \
   } \
   else mt_style = METATHEME_STYLE(widget->style);

static gint patch_count = 0;

static gint _menu_item_enter_notify_event(GtkWidget *, GdkEventCrossing *, gpointer);
static gint _menu_item_leave_notify_event(GtkWidget *, GdkEventCrossing *, gpointer);
static gint _menu_item_expose_event(GtkWidget *, GdkEventExpose *);
static gint _menu_item_size_request(GtkWidget *, GtkRequisition *);
static void _tearoff_menu_item_size_request(GtkWidget *, GtkRequisition *);
static gint _tearoff_menu_item_expose_event(GtkWidget *, GdkEventExpose *);
static void _menu_bar_size_allocate(GtkWidget *, GtkAllocation *);
static void _scrolled_window_size_allocate(GtkWidget *, GtkAllocation *);
static gint _scrolled_window_expose_event(GtkWidget *, GdkEventExpose *);
static gint _notebook_size_request(GtkWidget *, GtkRequisition *);
static gint _notebook_expose_event(GtkWidget *, GdkEventExpose *);
/* static gint _notebook_button_press_event(GtkWidget *, GdkEventButton *); */
/* static gint _notebook_button_release_event(GtkWidget *, GdkEventButton *); */
static gint _spin_button_enter_notify_event(GtkWidget *, GdkEventCrossing *, gpointer);
static gint _spin_button_leave_notify_event(GtkWidget *, GdkEventCrossing *, gpointer);
/* static gint _option_menu_button_press_event(GtkWidget *, GdkEventButton *); */
/* static gint _option_menu_button_release_event(GtkWidget *, GdkEventButton *); */
static void _combo_size_allocate(GtkWidget *, GtkAllocation *);
static void _hbutton_box_size_allocate(GtkWidget *, GtkAllocation *);
static gint _range_button_press_event(GtkWidget *, GdkEventButton *);
static gint _range_button_release_event(GtkWidget *, GdkEventButton *);

static gint (*old_menu_item_enter_notify_event)(GtkWidget *, GdkEventCrossing *, gpointer);
static gint (*old_menu_item_leave_notify_event)(GtkWidget *, GdkEventCrossing *, gpointer);
static gint (*old_menu_item_expose_event)(GtkWidget *, GdkEventExpose *);
static gint (*old_menu_item_size_request)(GtkWidget *, GtkRequisition *);
static void (*old_tearoff_menu_item_size_request)(GtkWidget *, GtkRequisition *);
static gint (*old_tearoff_menu_item_expose_event)(GtkWidget *, GdkEventExpose *);
static void (*old_menu_bar_size_allocate)(GtkWidget *, GtkAllocation *);
static void (*old_scrolled_window_size_allocate)(GtkWidget *, GtkAllocation *);
static gint (*old_scrolled_window_expose_event)(GtkWidget *, GdkEventExpose *);
static gint (*old_notebook_size_request)(GtkWidget *, GtkRequisition *);
static gint (*old_notebook_expose_event)(GtkWidget *, GdkEventExpose *);
/* static gint (*old_notebook_button_press_event)(GtkWidget *, GdkEventButton *); */
/* static gint (*old_notebook_button_release_event)(GtkWidget *, GdkEventButton *); */
static gint (*old_spin_button_enter_notify_event)(GtkWidget *, GdkEventCrossing *, gpointer);
static gint (*old_spin_button_leave_notify_event)(GtkWidget *, GdkEventCrossing *, gpointer);
/* static gint (*old_option_menu_button_press_event)(GtkWidget *, GdkEventButton *); */
/* static gint (*old_option_menu_button_release_event)(GtkWidget *, GdkEventButton *); */
static void (*old_combo_size_allocate)(GtkWidget *, GtkAllocation *);
static void (*old_hbutton_box_size_allocate)(GtkWidget *, GtkAllocation *);
static gint (*old_range_button_press_event)(GtkWidget *, GdkEventButton *);
static gint (*old_range_button_release_event)(GtkWidget *, GdkEventButton *);


static void patch_install(GtkObjectClass *klass, size_t offset, GtkFunction func, GtkFunction *old_func)
{
   GtkFunction *ptr;

   ptr = (GtkFunction *)((guchar *)klass + offset);

   *old_func = *ptr;
   if (*ptr == func) {
      exit(1);
   }

   *ptr = func;
}


static void patch_restore(GtkObjectClass *klass, size_t offset, GtkFunction func, GtkFunction old_func)
{
   /* GList *child; */
   GtkFunction *ptr;

   ptr = (GtkFunction *)((guchar *)klass + offset);
   if (*ptr == func)
   *ptr = old_func;

   /*
   TODO: needs more investigation on this
   for (child = gtk_type_children_types(klass->type); child; child = child->next) {
      patch_restore (gtk_type_class(GPOINTER_TO_UINT(child->data)), offset, func, old_func);
   }
   */
}


void mt_gtk2_install_patches()
{
	GtkWidgetClass	*widget_class;

   if (patch_count++ != 0) return;

	/* GtkMenuItem: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_menu_item_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, menu_item, enter_notify_event);
   PATCH_INSTALL(widget_class, GtkWidgetClass, menu_item, leave_notify_event);
   PATCH_INSTALL(widget_class, GtkWidgetClass, menu_item, expose_event);
   PATCH_INSTALL(widget_class, GtkWidgetClass, menu_item, size_request);

	/* GtkTearoffMenuItem: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_tearoff_menu_item_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, tearoff_menu_item, size_request);
   PATCH_INSTALL(widget_class, GtkWidgetClass, tearoff_menu_item, expose_event);

  	/* GtkMenuBar: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_menu_bar_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, menu_bar, size_allocate);

  	/* GtkScrolledWindow: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_scrolled_window_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, scrolled_window, size_allocate);
   PATCH_INSTALL(widget_class, GtkWidgetClass, scrolled_window, expose_event);

  	/* GtkNotebook: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_notebook_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, notebook, size_request);
   PATCH_INSTALL(widget_class, GtkWidgetClass, notebook, expose_event);
   /*
   PATCH_INSTALL(widget_class, GtkWidgetClass, notebook, button_press_event);
   PATCH_INSTALL(widget_class, GtkWidgetClass, notebook, button_release_event);
   */

  	/* GtkSpinButton: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_spin_button_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, spin_button, enter_notify_event);
   PATCH_INSTALL(widget_class, GtkWidgetClass, spin_button, leave_notify_event);

  	/* GtkOptionMenu: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_option_menu_get_type());
   /*
   PATCH_INSTALL(widget_class, GtkWidgetClass, option_menu, button_press_event);
   PATCH_INSTALL(widget_class, GtkWidgetClass, option_menu, button_release_event);
   */

  	/* GtkCombo: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_combo_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, combo, size_allocate);

  	/* GtkHButtonBox: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_hbutton_box_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, hbutton_box, size_allocate);

  	/* GtkRange: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_range_get_type());
   PATCH_INSTALL(widget_class, GtkWidgetClass, range, button_press_event);
   PATCH_INSTALL(widget_class, GtkWidgetClass, range, button_release_event);
}


void mt_gtk2_uninstall_patches()
{
	GtkWidgetClass	*widget_class;

	if (patch_count == 0) return;
	patch_count = 0;

	/* GtkMenuItem: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_menu_item_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, menu_item, enter_notify_event);
   PATCH_RESTORE(widget_class, GtkWidgetClass, menu_item, leave_notify_event);
   PATCH_RESTORE(widget_class, GtkWidgetClass, menu_item, expose_event);
   PATCH_RESTORE(widget_class, GtkWidgetClass, menu_item, size_request);

	/* GtkTearoffMenuItem: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_tearoff_menu_item_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, tearoff_menu_item, size_request);
   PATCH_RESTORE(widget_class, GtkWidgetClass, tearoff_menu_item, expose_event);

  	/* GtkMenuBar: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_menu_bar_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, menu_bar, size_allocate);

  	/* GtkScrolledWindow: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_scrolled_window_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, scrolled_window, size_allocate);
   PATCH_RESTORE(widget_class, GtkWidgetClass, scrolled_window, expose_event);

  	/* GtkNotebook: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_notebook_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, notebook, size_request);
   PATCH_RESTORE(widget_class, GtkWidgetClass, notebook, expose_event);
   /*
   PATCH_RESTORE(widget_class, GtkWidgetClass, notebook, button_press_event);
   PATCH_RESTORE(widget_class, GtkWidgetClass, notebook, button_release_event);
   */

  	/* GtkSpinButton: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_spin_button_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, spin_button, enter_notify_event);
   PATCH_RESTORE(widget_class, GtkWidgetClass, spin_button, leave_notify_event);

  	/* GtkCombo: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_combo_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, combo, size_allocate);

  	/* GtkHButtonBox: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_hbutton_box_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, hbutton_box, size_allocate);

  	/* GtkRange: */
   widget_class = (GtkWidgetClass *)gtk_type_class(gtk_range_get_type());
   PATCH_RESTORE(widget_class, GtkWidgetClass, range, button_press_event);
   PATCH_RESTORE(widget_class, GtkWidgetClass, range, button_release_event);
}


/**
 * GtkMenuItem patches:
 *
 * Purposes:
 *
 * 1. Add tracking of mouse hover state in GtkMenuBar items.
 * 2. Enforce height of menu separator.
 * 
 * Note:
 *
 * The GTK_STATE_ACTIVE and GTK_STATE_PRELIGHT are swapped (it's then fixed in the drawing routine).
 */


static gint _menu_item_enter_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
   /* sets ACTIVE state when mouse is on top of menubar item: */
   if (widget && widget->parent && GTK_IS_MENU_BAR(widget->parent)) {
      gtk_widget_set_state(widget, GTK_STATE_ACTIVE);
      gtk_widget_queue_draw(widget);
   }
   return old_menu_item_enter_notify_event(widget, event, data);
}


static gint _menu_item_leave_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
   /* unsets ACTIVE state when mouse is out of menubar item: */
   if (widget && widget->parent && GTK_IS_MENU_BAR(widget->parent) && widget->state == GTK_STATE_ACTIVE) {
      gtk_widget_set_state(widget, GTK_STATE_NORMAL);
      gtk_widget_queue_draw(widget);
   }
   return old_menu_item_leave_notify_event(widget, event, data);
}


static gint _menu_item_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
   gint width, height;
   gint x, y;
   gint border_width = GTK_CONTAINER(widget)->border_width;

   x = widget->allocation.x + border_width;
   y = widget->allocation.y + border_width;
   width = widget->allocation.width - border_width * 2;
   height = widget->allocation.height - border_width * 2;

   /* draws shadow for our GTK_STATE_ACTIVE (only for menubar items): */
   if (GTK_WIDGET_DRAWABLE(widget) && widget->parent && GTK_IS_MENU_BAR(widget->parent) && widget->state == GTK_STATE_ACTIVE && GTK_BIN(widget)->child) {
      gtk_paint_box (widget->style,
         widget->window,
         GTK_STATE_ACTIVE,
         GTK_SHADOW_NONE,
         &event->area, widget, "menuitem",
         x, y, width, height);
   }
   /* draws shadow (and background) for inactive menu items: */
   else if (GTK_WIDGET_DRAWABLE(widget) && !(widget->parent && GTK_IS_MENU_BAR(widget->parent)) && (widget->state == GTK_STATE_NORMAL || widget->state == GTK_STATE_INSENSITIVE) && GTK_BIN(widget)->child) {
      gtk_paint_box (widget->style,
         widget->window,
         widget->state,
         GTK_SHADOW_NONE,
         &event->area, widget, "menuitem",
         x, y, width, height);
   }

   return old_menu_item_expose_event(widget, event);
}


static gint _menu_item_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
   gint ret = old_menu_item_size_request(widget, requisition);
   if (!mt_default_engine) return ret;
   
   if (GTK_IS_SEPARATOR_MENU_ITEM(widget) || !GTK_BIN(widget)->child) {
      requisition->height = mt_default_engine->metric[MT_MENU_SEPARATOR_HEIGHT];
   }
   return ret;
}


/**
 * GtkTearoffMenuItem patches:
 *
 * Purpose: Enforce height of tearoff item and custom drawing.
 */


static void _tearoff_menu_item_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
   if (!mt_default_engine) {
      return old_tearoff_menu_item_size_request(widget, requisition);
   }
   
   requisition->width = (GTK_CONTAINER(widget)->border_width + widget->style->xthickness + 3) * 2;

   if (GTK_TEAROFF_MENU_ITEM(widget)->torn_off) {
      requisition->height = mt_default_engine->metric[MT_DETACHED_MENU_TEAROFF_HEIGHT];
   }
   else {
      requisition->height = mt_default_engine->metric[MT_MENU_TEAROFF_HEIGHT];
   }
}


static gint _tearoff_menu_item_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
   gint width, height;
   gint x, y;
   gint border_width = GTK_CONTAINER(widget)->border_width;

   x = widget->allocation.x + border_width;
   y = widget->allocation.y + border_width;
   width = widget->allocation.width - border_width * 2;
   height = widget->allocation.height - border_width * 2;

   gtk_paint_box (widget->style,
      widget->window,
      widget->state,
      GTK_SHADOW_NONE,
      &event->area, widget, "tearoffmenuitem",
      x, y, width, height);

   return FALSE;
}


/**
 * GtkMenuBar patches:
 *
 * Purpose: Enforce spacing between MenuBar items.
 *
 * UGLY: we must replicate full code from GTK sources. :(
 */


#define BORDER_SPACING 0
#define CHILD_SPACING mt_style->engine->metric[MT_MENUBAR_ITEM_SPACING]


static GtkShadowType get_shadow_type(GtkMenuBar *menubar)
{
   GtkShadowType shadow_type = GTK_SHADOW_OUT;

   gtk_widget_style_get (GTK_WIDGET (menubar),
      "shadow_type", &shadow_type,
      NULL);

   return shadow_type;
}


static void _menu_bar_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
   GtkMenuBar *menu_bar;
   GtkMenuShell *menu_shell;
   GtkWidget *child;
   GList *children;
   GtkAllocation child_allocation;
   GtkRequisition child_requisition;
   guint offset;
   gint ipadding;
   GtkTextDirection direction;
   gint ltr_x;

   CHECK_STYLE(
      old_menu_bar_size_allocate(widget, allocation);
      return;
   );

   g_return_if_fail (GTK_IS_MENU_BAR (widget));
   g_return_if_fail (allocation != NULL);

   menu_bar = GTK_MENU_BAR (widget);
   menu_shell = GTK_MENU_SHELL (widget);

   direction = gtk_widget_get_direction (widget);

   widget->allocation = *allocation;
   if (GTK_WIDGET_REALIZED (widget))
      gdk_window_move_resize (widget->window,
               allocation->x, allocation->y,
               allocation->width, allocation->height);

   gtk_widget_style_get (widget, "internal_padding", &ipadding, NULL);
   
   if (menu_shell->children) {
      child_allocation.x = (GTK_CONTAINER (menu_bar)->border_width + ipadding + BORDER_SPACING);
      child_allocation.y = (GTK_CONTAINER (menu_bar)->border_width + ipadding + BORDER_SPACING);

      if (get_shadow_type (menu_bar) != GTK_SHADOW_NONE) {
         child_allocation.x += widget->style->xthickness;
         child_allocation.y += widget->style->ythickness;
      }
         
      child_allocation.height = MAX (1, (gint)allocation->height - child_allocation.y * 2);

      offset = child_allocation.x;    /* Window edge to menubar start */
      ltr_x = child_allocation.x;

      children = menu_shell->children;
      while (children) {
         gint toggle_size;

         child = children->data;
         children = children->next;

         gtk_menu_item_toggle_size_request (GTK_MENU_ITEM (child), &toggle_size);
         gtk_widget_get_child_requisition (child, &child_requisition);

         child_requisition.width += toggle_size;
               
         /* Support for the right justified help menu */
         if ((children == NULL) && (GTK_IS_MENU_ITEM(child)) && (GTK_MENU_ITEM(child)->right_justify)) {
            ltr_x = allocation->width - child_requisition.width - offset;
         }
         if (GTK_WIDGET_VISIBLE (child)) {
            if (direction == GTK_TEXT_DIR_LTR) {
               child_allocation.x = ltr_x;
            }
            else {
               child_allocation.x = allocation->width - child_requisition.width - ltr_x;
            }
            child_allocation.width = child_requisition.width;

            gtk_menu_item_toggle_size_allocate (GTK_MENU_ITEM (child), toggle_size);
            gtk_widget_size_allocate (child, &child_allocation);

            ltr_x += child_allocation.width + CHILD_SPACING;
         }
      }
   }
}


/**
 * GtkScrolledWindow patches:
 *
 * Purpose: Move scrollbars to the inner part of the frame.
 *
 * UGLY: we must replicate full code from GTK sources. :(
 */

#define SW_BORDER_SPACING sw_border_spacing(widget)

static int sw_border_spacing(GtkWidget *widget)
{
   GtkWidget *child = GTK_BIN(widget)->child;

   if (child && (GTK_IS_VIEWPORT(child) || GTK_IS_TREE_VIEW(child) || GTK_IS_CLIST(child) || GTK_IS_LAYOUT(child))) {
      return 2;
   }
   
   return 0;
}


static gint mt_gtk_scrolled_window_get_scrollbar_spacing(GtkScrolledWindow *scrolled_window)
{
   GtkScrolledWindowClass *class;
 
   g_return_val_if_fail(GTK_IS_SCROLLED_WINDOW(scrolled_window), 0);

   class = GTK_SCROLLED_WINDOW_GET_CLASS(scrolled_window);

   if (class->scrollbar_spacing >= 0) {
      return class->scrollbar_spacing;
   }

   return 0;
}


static void gtk_scrolled_window_relative_allocation(GtkWidget *widget, GtkAllocation *allocation)
{
   GtkScrolledWindow *scrolled_window;
   gint scrollbar_spacing;

   g_return_if_fail (widget != NULL);
   g_return_if_fail (allocation != NULL);

   scrolled_window = GTK_SCROLLED_WINDOW (widget);
   scrollbar_spacing = mt_gtk_scrolled_window_get_scrollbar_spacing (scrolled_window);

   allocation->x = GTK_CONTAINER (widget)->border_width;
   allocation->y = GTK_CONTAINER (widget)->border_width;

   if (scrolled_window->shadow_type != GTK_SHADOW_NONE) {
      allocation->x += widget->style->xthickness;
      allocation->y += widget->style->ythickness;
   }
   
   allocation->width = MAX (1, (gint)widget->allocation.width - allocation->x * 2);
   allocation->height = MAX (1, (gint)widget->allocation.height - allocation->y * 2);

   if (scrolled_window->vscrollbar_visible) {
      GtkRequisition vscrollbar_requisition;
      
      gtk_widget_get_child_requisition (scrolled_window->vscrollbar, &vscrollbar_requisition);
   
      if (scrolled_window->window_placement == GTK_CORNER_TOP_RIGHT || scrolled_window->window_placement == GTK_CORNER_BOTTOM_RIGHT) {
         allocation->x += (vscrollbar_requisition.width +   scrollbar_spacing + SW_BORDER_SPACING);
      }

      allocation->width = MAX (1, allocation->width - (vscrollbar_requisition.width + scrollbar_spacing + SW_BORDER_SPACING*0));
   }

   if (scrolled_window->hscrollbar_visible) {
      GtkRequisition hscrollbar_requisition;
      gtk_widget_get_child_requisition (scrolled_window->hscrollbar, &hscrollbar_requisition);
   
      if (scrolled_window->window_placement == GTK_CORNER_BOTTOM_LEFT || scrolled_window->window_placement == GTK_CORNER_BOTTOM_RIGHT) {
         allocation->y += (hscrollbar_requisition.height + scrollbar_spacing + SW_BORDER_SPACING);
      }

      allocation->height = MAX (1, allocation->height - (hscrollbar_requisition.height + scrollbar_spacing + SW_BORDER_SPACING*0));
   }
}


static void _scrolled_window_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
/*
   gint ret = old_scrolled_window_size_allocate(widget, allocation);
   GtkAllocation child_allocation;
   
   child_allocation = GTK_BIN(widget)->child->allocation;
   child_allocation.x += 2;
   child_allocation.y += 2;
   child_allocation.width -= 6;
   child_allocation.height -= 4;
   gtk_widget_size_allocate(GTK_BIN(widget)->child, &child_allocation);

   child_allocation = GTK_SCROLLED_WINDOW(widget)->vscrollbar->allocation;
   child_allocation.x -= 4;
   child_allocation.y += 2;
   child_allocation.height -= 4;
   gtk_widget_size_allocate(GTK_SCROLLED_WINDOW(widget)->vscrollbar, &child_allocation);

   return ret;
*/
  GtkScrolledWindow *scrolled_window;
  GtkBin *bin;
  GtkAllocation relative_allocation;
  GtkAllocation child_allocation;
  gint scrollbar_spacing;
  
  g_return_if_fail (GTK_IS_SCROLLED_WINDOW (widget));
  g_return_if_fail (allocation != NULL);

  scrolled_window = GTK_SCROLLED_WINDOW (widget);
  bin = GTK_BIN (scrolled_window);

  if (mt_get_ancestor(widget, "NautilusDesktopWindow")) {
    return old_scrolled_window_size_allocate(widget, allocation);
  }

  if (GTK_IS_VIEWPORT(bin->child)) {
    gtk_viewport_set_shadow_type(GTK_VIEWPORT(bin->child), GTK_SHADOW_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(bin->child), 0);
  }
  if (GTK_IS_TREE_VIEW(bin->child)) {
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget), GTK_SHADOW_NONE);
  }
  if (GTK_IS_CLIST(bin->child) && GTK_CLIST(bin->child)->shadow_type != GTK_SHADOW_NONE) {
    /*
    gtk_container_set_border_width(GTK_CONTAINER(bin->child), 0);
    */
    gtk_clist_set_shadow_type(GTK_CLIST(bin->child), GTK_SHADOW_NONE);
  }
  if (GTK_IS_LAYOUT(bin->child)) {
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget), GTK_SHADOW_NONE);
  }

  scrollbar_spacing = mt_gtk_scrolled_window_get_scrollbar_spacing (scrolled_window);

  widget->allocation = *allocation;

  if (scrolled_window->hscrollbar_policy == GTK_POLICY_ALWAYS)
    scrolled_window->hscrollbar_visible = TRUE;
  else if (scrolled_window->hscrollbar_policy == GTK_POLICY_NEVER)
    scrolled_window->hscrollbar_visible = FALSE;
  if (scrolled_window->vscrollbar_policy == GTK_POLICY_ALWAYS)
    scrolled_window->vscrollbar_visible = TRUE;
  else if (scrolled_window->vscrollbar_policy == GTK_POLICY_NEVER)
    scrolled_window->vscrollbar_visible = FALSE;

  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
      gboolean previous_hvis;
      gboolean previous_vvis;
      guint count = 0;
      
      do
	{
	  gtk_scrolled_window_relative_allocation (widget, &relative_allocation);
	  
	  child_allocation.x = relative_allocation.x + allocation->x + SW_BORDER_SPACING;
	  child_allocation.y = relative_allocation.y + allocation->y + SW_BORDER_SPACING;
	  child_allocation.width = MAX(1, relative_allocation.width - SW_BORDER_SPACING*2);
	  child_allocation.height = MAX(1, relative_allocation.height - SW_BORDER_SPACING*2);
	  
	  previous_hvis = scrolled_window->hscrollbar_visible;
	  previous_vvis = scrolled_window->vscrollbar_visible;
	  
	  gtk_widget_size_allocate (bin->child, &child_allocation);

	  /* If, after the first iteration, the hscrollbar and the
	   * vscrollbar flip visiblity, then we need both.
	   */
	  if (count &&
	      previous_hvis != scrolled_window->hscrollbar_visible &&
	      previous_vvis != scrolled_window->vscrollbar_visible)
	    {
	      scrolled_window->hscrollbar_visible = TRUE;
	      scrolled_window->vscrollbar_visible = TRUE;

	      /* a new resize is already queued at this point,
	       * so we will immediatedly get reinvoked
	       */
	      return;
	    }
	  
	  count++;
	}
      while (previous_hvis != scrolled_window->hscrollbar_visible ||
	     previous_vvis != scrolled_window->vscrollbar_visible);
    }
  else
    gtk_scrolled_window_relative_allocation (widget, &relative_allocation);
  
  if (scrolled_window->hscrollbar_visible)
    {
      GtkRequisition hscrollbar_requisition;
      gtk_widget_get_child_requisition (scrolled_window->hscrollbar,
					&hscrollbar_requisition);
  
      if (!GTK_WIDGET_VISIBLE (scrolled_window->hscrollbar))
	gtk_widget_show (scrolled_window->hscrollbar);

      child_allocation.x = relative_allocation.x;
      if (scrolled_window->window_placement == GTK_CORNER_TOP_LEFT ||
	  scrolled_window->window_placement == GTK_CORNER_TOP_RIGHT)
	child_allocation.y = (relative_allocation.y +
			      relative_allocation.height +
			      scrollbar_spacing +
			      (scrolled_window->shadow_type == GTK_SHADOW_NONE ?
			       0 : widget->style->ythickness));
      else
	child_allocation.y = GTK_CONTAINER (scrolled_window)->border_width;

      child_allocation.width = relative_allocation.width;
      child_allocation.height = hscrollbar_requisition.height;
      child_allocation.x += allocation->x;
      child_allocation.y += allocation->y;

      if (scrolled_window->shadow_type != GTK_SHADOW_NONE)
	{
	  child_allocation.x -= widget->style->xthickness;
	  child_allocation.width += 2 * widget->style->xthickness;
	}

      child_allocation.x += SW_BORDER_SPACING;
      if (scrolled_window->window_placement == GTK_CORNER_BOTTOM_LEFT || scrolled_window->window_placement == GTK_CORNER_BOTTOM_RIGHT) {
         child_allocation.y += SW_BORDER_SPACING;
      }
      else {
         child_allocation.y -= SW_BORDER_SPACING;
      }
      child_allocation.width = MAX(1, child_allocation.width - 2*SW_BORDER_SPACING);
      gtk_widget_size_allocate (scrolled_window->hscrollbar, &child_allocation);
    }
  else if (GTK_WIDGET_VISIBLE (scrolled_window->hscrollbar))
    gtk_widget_hide (scrolled_window->hscrollbar);

  if (scrolled_window->vscrollbar_visible)
    {
      GtkRequisition vscrollbar_requisition;
      if (!GTK_WIDGET_VISIBLE (scrolled_window->vscrollbar))
	gtk_widget_show (scrolled_window->vscrollbar);

      gtk_widget_get_child_requisition (scrolled_window->vscrollbar,
					&vscrollbar_requisition);

      if (scrolled_window->window_placement == GTK_CORNER_TOP_LEFT ||
	  scrolled_window->window_placement == GTK_CORNER_BOTTOM_LEFT)
	child_allocation.x = (relative_allocation.x +
			      relative_allocation.width +
			      scrollbar_spacing +
			      (scrolled_window->shadow_type == GTK_SHADOW_NONE ?
			       0 : widget->style->xthickness));
      else
	child_allocation.x = GTK_CONTAINER (scrolled_window)->border_width;

      child_allocation.y = relative_allocation.y;
      child_allocation.width = vscrollbar_requisition.width;
      child_allocation.height = relative_allocation.height;
      child_allocation.x += allocation->x;
      child_allocation.y += allocation->y;

      if (scrolled_window->shadow_type != GTK_SHADOW_NONE)
	{
	  child_allocation.y -= widget->style->ythickness;
	  child_allocation.height += 2 * widget->style->ythickness;
	}

      if (scrolled_window->window_placement == GTK_CORNER_TOP_RIGHT || scrolled_window->window_placement == GTK_CORNER_BOTTOM_RIGHT) {
         child_allocation.x += SW_BORDER_SPACING;
      }
      else {
         child_allocation.x -= SW_BORDER_SPACING;
      }
      child_allocation.y += SW_BORDER_SPACING;
      child_allocation.height = MAX(1, child_allocation.height - 2*SW_BORDER_SPACING);
      gtk_widget_size_allocate (scrolled_window->vscrollbar, &child_allocation);
    }
  else if (GTK_WIDGET_VISIBLE (scrolled_window->vscrollbar))
    gtk_widget_hide (scrolled_window->vscrollbar);
}


static gint _scrolled_window_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
   GtkAllocation allocation;

   if (SW_BORDER_SPACING > 0) {
      allocation.x = GTK_CONTAINER(widget)->border_width;
      allocation.y = GTK_CONTAINER(widget)->border_width;
      allocation.width = MAX(1, (gint)widget->allocation.width - allocation.x * 2);
      allocation.height = MAX(1, (gint)widget->allocation.height - allocation.y * 2);

      gtk_paint_box (widget->style,
         widget->window,
         GTK_STATE_NORMAL,
         GTK_SHADOW_IN,
         &event->area, widget, "scrolled_window",
         widget->allocation.x + allocation.x, widget->allocation.y + allocation.y, allocation.width, allocation.height);
   }

   return old_scrolled_window_expose_event(widget, event);
}


/**
 * GtkNotebook patches:
 *
 * Purpose: Enforce metrics and drawing.
 */


/*
static gint _notebook_size_request(GtkWidget *widget, GtkRequisition *req)
{
   gint ret = old_notebook_size_request(widget, req);

   if (GTK_NOTEBOOK(widget)->tab_pos == GTK_POS_TOP || GTK_NOTEBOOK(widget)->tab_pos == GTK_POS_BOTTOM) {
      req->width += 2;
   }
   else {
      req->height += 2;
   }

   return ret;
}
*/

#define TAB_OVERLAP    (mt_style->engine->metric[MT_NOTEBOOK_TAB_OVERLAP])
#define TAB_CURVATURE  1
#define ARROW_SIZE     12
#define ARROW_SPACING  0

#define GTK_NOTEBOOK_PAGE(_glist_)         ((GtkNotebookPage *)((GList *)(_glist_))->data)

struct _GtkNotebookPage
{
   GtkWidget *child;
   GtkWidget *tab_label;
   GtkWidget *menu_label;
   GtkWidget *last_focus_child;	/* Last descendant of the page that had focus */

   guint default_menu : 1;	/* If true, we create the menu label ourself */
   guint default_tab  : 1;	/* If true, we create the tab label ourself */
   guint expand       : 1;
   guint fill         : 1;
   guint pack         : 1;

   GtkRequisition requisition;
   GtkAllocation allocation;

   guint mnemonic_activate_signal;
};

enum {
   SWITCH_PAGE,
   FOCUS_TAB,
   SELECT_PAGE,
   CHANGE_CURRENT_PAGE,
   MOVE_FOCUS_OUT,
   LAST_SIGNAL
};

enum {
   STEP_PREV,
   STEP_NEXT
};


static GList *gtk_notebook_search_page(GtkNotebook *notebook,
            GList       *list,
            gint         direction,
            gboolean     find_visible)
{
   GtkNotebookPage *page = NULL;
   GList *old_list = NULL;
   gint flag = 0;

   g_return_val_if_fail(GTK_IS_NOTEBOOK(notebook), NULL);

   switch (direction) {
      case STEP_PREV:
         flag = GTK_PACK_END;
         break;

      case STEP_NEXT:
         flag = GTK_PACK_START;
         break;
   }

   if (list) page = list->data;

   if (!page || page->pack == flag) {
      if (list) {
         old_list = list;
         list = list->next;
      }
      else {
         list = notebook->children;
      }

      while (list) {
         page = list->data;
         if (page->pack == flag && (!find_visible || GTK_WIDGET_VISIBLE (page->child))) return list;
         old_list = list;
         list = list->next;
      }
      list = old_list;
   }
   else {
      old_list = list;
      list = list->prev;
   }

   while (list) {
      page = list->data;
      if (page->pack != flag && (!find_visible || GTK_WIDGET_VISIBLE (page->child))) return list;
      old_list = list;
      list = list->prev;
   }

   return NULL;
}


static void gtk_notebook_switch_page(GtkNotebook *notebook,
            GtkNotebookPage *page,
            gint             page_num)
{ 
   g_return_if_fail(GTK_IS_NOTEBOOK(notebook));
   g_return_if_fail(page != NULL);
 
   if (notebook->cur_page == page) return;

   if (page_num < 0) page_num = g_list_index(notebook->children, page);

   g_signal_emit(notebook,
         g_signal_lookup("switch_page", G_TYPE_FROM_CLASS(G_OBJECT_GET_CLASS(notebook))),
         0,
         page,
         page_num);
}

/* UGLY: copied from mt_gtk2_draw.c: */
static void retrieve_colors(GtkStateType state_type, GtkStyle *style, MT_WIDGET_DATA *data)
{
   if (style) {
      GdkColor *c = &(style->bg[state_type]);
      mt_color_set(data->background_color, c->red >> 8, c->green >> 8, c->blue >> 8);
   }
   else {
      mt_color_set(data->background_color, 0, 0, 0);
   }
}

static void gtk_notebook_draw_tab(GtkNotebook *notebook, GtkNotebookPage *page, GdkRectangle *area)
{
   GdkRectangle child_area;
   GdkRectangle page_area;
   int state = 0;
   MT_WIDGET_DATA data;
   GList *tmp;
 
   g_return_if_fail(notebook != NULL);
   g_return_if_fail(page != NULL);
   g_return_if_fail(area != NULL);

   if (!GTK_WIDGET_MAPPED(page->tab_label) || (page->allocation.width == 0) || (page->allocation.height == 0)) return;

   page_area.x = page->allocation.x;
   page_area.y = page->allocation.y;
   page_area.width = page->allocation.width + mt_style->engine->metric[MT_NOTEBOOK_TAB_OVERLAP] - 2;
   page_area.height = page->allocation.height;

   /* TODO: */
   switch (notebook->tab_pos) {
      case GTK_POS_BOTTOM:
         page_area.y -= mt_style->engine->metric[MT_NOTEBOOK_OVERLAP];
      case GTK_POS_TOP:
         page_area.height += mt_style->engine->metric[MT_NOTEBOOK_OVERLAP];
         break;
   }

   if (gdk_rectangle_intersect(&page_area, area, &child_area)) {
      GtkWidget *widget;
      GdkRectangle tab_area = page_area;

      widget = GTK_WIDGET(notebook);
 
      switch (notebook->tab_pos) {
         case GTK_POS_LEFT:   data.gap_position = MT_POSITION_LEFT; break;
         case GTK_POS_RIGHT:  data.gap_position = MT_POSITION_RIGHT; break;
         case GTK_POS_TOP:    data.gap_position = MT_POSITION_TOP; break;
         case GTK_POS_BOTTOM: data.gap_position = MT_POSITION_BOTTOM; break;
      }

      state = 0;
      if (notebook->cur_page == page) {
         state |= MT_SELECTED;
      }
      else {
         /* TODO: */
         if (notebook->tab_pos == GTK_POS_TOP) {
            tab_area.y -= mt_style->engine->metric[MT_NOTEBOOK_TEXT_OFFSET];
            tab_area.height += mt_style->engine->metric[MT_NOTEBOOK_TEXT_OFFSET];
         }
         else if (notebook->tab_pos == GTK_POS_BOTTOM) {
            tab_area.height += mt_style->engine->metric[MT_NOTEBOOK_TEXT_OFFSET];
         }
      }

      data.flags = 0;
      if ((void *)notebook->first_tab == (void *)page) {
         data.flags |= MT_NOTEBOOK_FIRST_VISIBLE_TAB;
      }

      tmp = g_list_first(notebook->children);
      if (tmp && tmp->data == (void *)page) data.flags |= MT_NOTEBOOK_FIRST_TAB;

      tmp = g_list_last(notebook->children);
      if (tmp && tmp->data == (void *)page) data.flags |= MT_NOTEBOOK_LAST_TAB;

      retrieve_colors(GTK_STATE_NORMAL, widget->style, &data);
      data.widget = (MT_WIDGET)widget;

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)widget->window, (MT_RECTANGLE *)area, MT_NOTEBOOK_TAB, state, tab_area.x, tab_area.y, tab_area.width, tab_area.height, &data);

      if ((GTK_WIDGET_HAS_FOCUS(widget)) && notebook->focus_tab && (notebook->focus_tab->data == page)) {
         gint focus_width;
 
         gtk_widget_style_get(widget, "focus-line-width", &focus_width, NULL);
 
         gtk_paint_focus(widget->style, widget->window, GTK_WIDGET_STATE (widget),
               area, widget, "tab",
               page->tab_label->allocation.x - focus_width,
               page->tab_label->allocation.y - focus_width,
               page->tab_label->allocation.width + 2 * focus_width,
               page->tab_label->allocation.height + 2 * focus_width);
      }

      if (gtk_widget_intersect(page->tab_label, area, &child_area) && GTK_WIDGET_DRAWABLE (page->tab_label)) {
         GdkEvent *expose_event = gdk_event_new(GDK_EXPOSE);

         /* This is a lame hack since all this code needs rewriting anyhow */
 
         expose_event->expose.window = g_object_ref(page->tab_label->window);
         expose_event->expose.area = child_area;
         expose_event->expose.region = gdk_region_rectangle(&child_area);
         expose_event->expose.send_event = TRUE;
         expose_event->expose.count = 0;

         gtk_container_propagate_expose(GTK_CONTAINER(notebook), page->tab_label, (GdkEventExpose *)expose_event);

         gdk_event_free (expose_event);
      }
   }
}

static gboolean
gtk_notebook_get_event_window_position (GtkNotebook  *notebook,
					GdkRectangle *rectangle)
{
  GtkWidget *widget = GTK_WIDGET (notebook);
  gint border_width = GTK_CONTAINER (notebook)->border_width;
  GtkNotebookPage *visible_page = NULL;
  GList *tmp_list;

  for (tmp_list = notebook->children; tmp_list; tmp_list = tmp_list->next)
    {
      GtkNotebookPage *page = tmp_list->data;
      if (GTK_WIDGET_VISIBLE (page->child))
	{
	  visible_page = page;
	  break;
	}
    }

  if (notebook->show_tabs && visible_page)
    {
      if (rectangle)
	{
	  rectangle->x = widget->allocation.x + border_width;
	  rectangle->y = widget->allocation.y + border_width;
	  
	  switch (notebook->tab_pos)
	    {
	    case GTK_POS_TOP:
	    case GTK_POS_BOTTOM:
	      rectangle->width = widget->allocation.width - 2 * border_width;
	      rectangle->height = visible_page->requisition.height;
	      if (notebook->tab_pos == GTK_POS_BOTTOM)
		rectangle->y += widget->allocation.height - 2 * border_width - rectangle->height;
	      break;
	    case GTK_POS_LEFT:
	    case GTK_POS_RIGHT:
	      rectangle->width = visible_page->requisition.width;
	      rectangle->height = widget->allocation.height - 2 * border_width;
	      if (notebook->tab_pos == GTK_POS_RIGHT)
		rectangle->x += widget->allocation.width - 2 * border_width - rectangle->width;
	      break;
	    }
	}

      return TRUE;
    }
  else
    {
      if (rectangle)
	{
	  rectangle->x = rectangle->y = 0;
	  rectangle->width = rectangle->height = 10;
	}
    }

  return FALSE;
}

static void
gtk_notebook_get_arrow_rect (GtkNotebook  *notebook,
			     GdkRectangle *rectangle)
{
  GdkRectangle event_window_pos;

  if (gtk_notebook_get_event_window_position (notebook, &event_window_pos))
    {
      rectangle->width = 2 * ARROW_SIZE + ARROW_SPACING;
      rectangle->height = ARROW_SIZE;

      switch (notebook->tab_pos)
	{
	case GTK_POS_LEFT:
	case GTK_POS_RIGHT:
	  rectangle->x = event_window_pos.x + (event_window_pos.width - rectangle->width) / 2;
	  rectangle->y = event_window_pos.y + event_window_pos.height - rectangle->height;
	  break;
	case GTK_POS_TOP:
	case GTK_POS_BOTTOM:
	  rectangle->x = event_window_pos.x + event_window_pos.width - rectangle->width;
	  rectangle->y = event_window_pos.y + (event_window_pos.height - rectangle->height) / 2;
	  break;
	}
    }
}


static void
gtk_notebook_draw_arrow (GtkNotebook *notebook,
			 guint        arrow)
{
  GtkStateType state_type;
  GtkShadowType shadow_type;
  GtkWidget *widget;
  GdkRectangle arrow_rect;

  gtk_notebook_get_arrow_rect (notebook, &arrow_rect);

  g_return_if_fail (GTK_IS_NOTEBOOK (notebook));

  widget = GTK_WIDGET(notebook);

   arrow_rect.height = notebook->cur_page->allocation.height + mt_style->engine->metric[MT_NOTEBOOK_OVERLAP];

   if (notebook->tab_pos == GTK_POS_TOP) {
      arrow_rect.y = widget->allocation.y + gtk_container_get_border_width(GTK_CONTAINER(widget));
   }
   else if (notebook->tab_pos == GTK_POS_BOTTOM) {
      arrow_rect.y = widget->allocation.y + widget->allocation.height - gtk_container_get_border_width(GTK_CONTAINER(widget)) - arrow_rect.height;
   }
   else {
      return;
   }

  if (GTK_WIDGET_DRAWABLE (notebook))
    {
      if (notebook->in_child == arrow)
        {
          if (notebook->click_child == arrow)
            state_type = GTK_STATE_ACTIVE;
          else
            state_type = GTK_STATE_PRELIGHT;
        }
      else
        state_type = GTK_WIDGET_STATE (widget);

      if (notebook->click_child == arrow)
        shadow_type = GTK_SHADOW_IN;
      else
        shadow_type = GTK_SHADOW_OUT;

      if (arrow == GTK_ARROW_LEFT)
	{
	  if (notebook->focus_tab &&
	      !gtk_notebook_search_page (notebook, notebook->focus_tab,
					 STEP_PREV, TRUE))
	    {
	      shadow_type = GTK_SHADOW_ETCHED_IN;
	      state_type = GTK_STATE_INSENSITIVE;
	    }

	  if (notebook->tab_pos == GTK_POS_LEFT ||
	      notebook->tab_pos == GTK_POS_RIGHT)
	    arrow = GTK_ARROW_UP;

	  gtk_paint_arrow (widget->style, widget->window, state_type, 
			   shadow_type, NULL, widget, "notebook",
			   arrow, TRUE, 
			   arrow_rect.x, arrow_rect.y, ARROW_SIZE, arrow_rect.height);
	}
      else
	{
	  if (notebook->focus_tab &&
	      !gtk_notebook_search_page (notebook, notebook->focus_tab,
					 STEP_NEXT, TRUE))
	    {
	      shadow_type = GTK_SHADOW_ETCHED_IN;
	      state_type = GTK_STATE_INSENSITIVE;
	    }

	  if (notebook->tab_pos == GTK_POS_LEFT ||
	      notebook->tab_pos == GTK_POS_RIGHT)
	    arrow = GTK_ARROW_DOWN;

	   gtk_paint_arrow (widget->style, widget->window, state_type, 
			    shadow_type, NULL, widget, "notebook",
			    arrow, TRUE, arrow_rect.x + ARROW_SIZE + ARROW_SPACING,
			    arrow_rect.y, ARROW_SIZE, arrow_rect.height);
	}
    }
}

static void gtk_notebook_paint(GtkWidget *widget, GdkRectangle *area)
{
   GtkNotebook *notebook;
   GtkNotebookPage *page;
   GList *children;
   gboolean showarrow;
   gint width, height;
   gint x, y;
   gint border_width = GTK_CONTAINER(widget)->border_width;
   gint gap_x = 0, gap_width = 0;
 
   g_return_if_fail(GTK_IS_NOTEBOOK(widget));
   g_return_if_fail(area != NULL);

   if (!GTK_WIDGET_DRAWABLE(widget)) return;

   notebook = GTK_NOTEBOOK(widget);

   if ((!notebook->show_tabs && !notebook->show_border) || !notebook->cur_page || !GTK_WIDGET_VISIBLE(notebook->cur_page->child)) return;

   x = widget->allocation.x + border_width;
   y = widget->allocation.y + border_width;
   width = widget->allocation.width - border_width * 2;
   height = widget->allocation.height - border_width * 2;

   if (notebook->show_border && (!notebook->show_tabs || !notebook->children)) {
      gtk_paint_box(widget->style, widget->window,
            GTK_STATE_NORMAL, GTK_SHADOW_OUT,
            area, widget, "notebook",
            x, y, width, height);
      return;
   }

   if (!GTK_WIDGET_MAPPED(notebook->cur_page->tab_label)) {
      page = notebook->first_tab->data;

      switch (notebook->tab_pos) {
         case GTK_POS_TOP:
            y += page->allocation.height + widget->style->ythickness;
         case GTK_POS_BOTTOM:
            height -= page->allocation.height + widget->style->ythickness;
            break;

         case GTK_POS_LEFT:
            x += page->allocation.width + widget->style->xthickness;
         case GTK_POS_RIGHT:
            width -= page->allocation.width + widget->style->xthickness;
            break;
      }
      
      gtk_paint_box(widget->style, widget->window,
            GTK_STATE_NORMAL, GTK_SHADOW_OUT,
            area, widget, "notebook",
            x, y, width, height);
   }
   else {
      switch (notebook->tab_pos) {
         case GTK_POS_TOP:
            y += notebook->cur_page->allocation.height;
         case GTK_POS_BOTTOM:
            height -= notebook->cur_page->allocation.height;
            break;

         case GTK_POS_LEFT:
            x += notebook->cur_page->allocation.width;
         case GTK_POS_RIGHT:
            width -= notebook->cur_page->allocation.width;
         break;
      }

      gap_x = 0;
      gap_width = 0;
      gtk_paint_box_gap(widget->style, widget->window,
            GTK_STATE_NORMAL, GTK_SHADOW_OUT,
            area, widget, "notebook",
            x, y, width, height,
            notebook->tab_pos, gap_x, gap_width);
   }

   showarrow = FALSE;
   children = gtk_notebook_search_page(notebook, NULL, STEP_PREV, TRUE);
   while (children) {
      page = children->data;
      children = gtk_notebook_search_page(notebook, children, STEP_PREV, TRUE);
      if (!GTK_WIDGET_VISIBLE(page->child)) continue;

      if (!GTK_WIDGET_MAPPED(page->tab_label)) {
         showarrow = TRUE;
      }
      else if (page != notebook->cur_page) {
         gtk_notebook_draw_tab(notebook, page, area);
      }
   }

   if (showarrow && notebook->scrollable) {
      gtk_notebook_draw_arrow(notebook, GTK_ARROW_LEFT);
      gtk_notebook_draw_arrow(notebook, GTK_ARROW_RIGHT);
   }

   gtk_notebook_draw_tab(notebook, notebook->cur_page, area);
}


static gint _notebook_size_request(GtkWidget *widget, GtkRequisition *req)
{
   GtkNotebook *notebook = GTK_NOTEBOOK(widget);
   GtkNotebookPage *page;
   GList *children;
   GtkRequisition child_requisition;
   gboolean switch_page = FALSE;
   gint vis_pages;
   gint focus_width;

   CHECK_STYLE(return old_notebook_size_request(widget, req));

   gtk_widget_style_get(widget, "focus-line-width", &focus_width, NULL);

   widget->requisition.width = 0;
   widget->requisition.height = 0;

   for (children=notebook->children, vis_pages=0; children; children=children->next) {
      page = children->data;

      if (GTK_WIDGET_VISIBLE(page->child)) {
         vis_pages++;
         gtk_widget_size_request(page->child, &child_requisition);
 
         widget->requisition.width = MAX(widget->requisition.width, child_requisition.width);
         widget->requisition.height = MAX(widget->requisition.height, child_requisition.height);

         if (notebook->menu && page->menu_label->parent && !GTK_WIDGET_VISIBLE(page->menu_label->parent)) {
            gtk_widget_show (page->menu_label->parent);
         }
      }
      else {
         if (page == notebook->cur_page) switch_page = TRUE;
         
         if (notebook->menu && page->menu_label->parent && GTK_WIDGET_VISIBLE(page->menu_label->parent)) {
            gtk_widget_hide (page->menu_label->parent);
         }
      }
   }

   if (notebook->show_border || notebook->show_tabs) {
      widget->requisition.width += widget->style->xthickness * 2;
      widget->requisition.height += widget->style->ythickness * 2;

      if (notebook->show_tabs) {
         gint tab_width = 0;
         gint tab_height = 0;
         gint tab_max = 0;
         gint padding;
 
         for (children=notebook->children; children; children=children->next) {
            page = children->data;
 
            if (GTK_WIDGET_VISIBLE(page->child)) {
               if (!GTK_WIDGET_VISIBLE(page->tab_label)) gtk_widget_show (page->tab_label);

               gtk_widget_size_request(page->tab_label, &child_requisition);

               page->requisition.width = child_requisition.width + 2 * widget->style->xthickness;
               page->requisition.height = child_requisition.height + 2 * widget->style->ythickness;
 
               switch (notebook->tab_pos) {
                  case GTK_POS_TOP:
                  case GTK_POS_BOTTOM:
                     page->requisition.height += 2 * (notebook->tab_vborder + focus_width);
                     tab_height = MAX(tab_height, page->requisition.height);
                     tab_max = MAX(tab_max, page->requisition.width);
                     break;

                  case GTK_POS_LEFT:
                  case GTK_POS_RIGHT:
                     page->requisition.width += 2 * (notebook->tab_hborder + focus_width);
                     tab_width = MAX(tab_width, page->requisition.width);
                     tab_max = MAX(tab_max, page->requisition.height);
                     break;
               }
            }
            else if (GTK_WIDGET_VISIBLE(page->tab_label)) {
               gtk_widget_hide (page->tab_label);
            }
         }

         children = notebook->children;

         if (vis_pages) {
            switch (notebook->tab_pos) {
               case GTK_POS_TOP:
               case GTK_POS_BOTTOM:
                  if (tab_height == 0) break;

                  if (notebook->scrollable && vis_pages > 1 && widget->requisition.width < tab_width) {
                     tab_height = MAX (tab_height, ARROW_SIZE);
                  }

                  padding = 2 * (TAB_CURVATURE + focus_width + notebook->tab_hborder) - TAB_OVERLAP;
                  tab_max += padding;
                  while (children) {
                     page = children->data;
                     children = children->next;
 
                     if (!GTK_WIDGET_VISIBLE(page->child)) continue;

                     if (notebook->homogeneous) {
                        page->requisition.width = tab_max;
                     }
                     else {
                        page->requisition.width += padding;
                     }

                     tab_width += page->requisition.width;
                     page->requisition.height = tab_height;
                  }

                  if (notebook->scrollable && vis_pages > 1 && widget->requisition.width < tab_width) {
                     tab_width = tab_max + 2 * (ARROW_SIZE + ARROW_SPACING);
                  }

                  if (notebook->homogeneous && !notebook->scrollable) {
                     widget->requisition.width = MAX(widget->requisition.width, vis_pages * tab_max + TAB_OVERLAP);
                  }
                  else {
                     widget->requisition.width = MAX(widget->requisition.width, tab_width + TAB_OVERLAP);
                  }

                  widget->requisition.height += tab_height;
                  break;

               case GTK_POS_LEFT:
               case GTK_POS_RIGHT:
                  if (tab_width == 0) break;

                  if (notebook->scrollable && vis_pages > 1 && widget->requisition.height < tab_height) {
                     tab_width = MAX (tab_width, ARROW_SPACING +2 * ARROW_SIZE);
                  }

                  padding = 2 * (TAB_CURVATURE + focus_width + notebook->tab_vborder) - TAB_OVERLAP;
                  tab_max += padding;

                  while (children) {
                     page = children->data;
                     children = children->next;

                     if (!GTK_WIDGET_VISIBLE(page->child)) continue;

                     page->requisition.width = tab_width;

                     if (notebook->homogeneous) {
                        page->requisition.height = tab_max;
                     }
                     else {
                        page->requisition.height += padding;
                     }

                     tab_height += page->requisition.height;
                  }

                  if (notebook->scrollable && vis_pages > 1 && widget->requisition.height < tab_height) {
                     tab_height = tab_max + ARROW_SIZE + ARROW_SPACING;
                  }

                  widget->requisition.width += tab_width;

                  if (notebook->homogeneous && !notebook->scrollable) {
                     widget->requisition.height = MAX(widget->requisition.height, vis_pages * tab_max + TAB_OVERLAP);
                  }
                  else {
                     widget->requisition.height = MAX(widget->requisition.height, tab_height + TAB_OVERLAP);
                  }

                  if (!notebook->homogeneous || notebook->scrollable) vis_pages = 1;

                  widget->requisition.height = MAX(widget->requisition.height, vis_pages * tab_max + TAB_OVERLAP);
                  break;
            }
         }
      }
      else {
         for (children=notebook->children; children; children=children->next) {
            page = children->data;
 
            if (page->tab_label && GTK_WIDGET_VISIBLE(page->tab_label)) gtk_widget_hide (page->tab_label);
         }
      }
   }

   widget->requisition.width += GTK_CONTAINER(widget)->border_width * 2;
   widget->requisition.height += GTK_CONTAINER(widget)->border_width * 2;

   if (switch_page) {
      if (vis_pages) {
         for (children=notebook->children; children; children=children->next) {
            page = children->data;
            if (GTK_WIDGET_VISIBLE(page->child)) {
               gtk_notebook_switch_page(notebook, page, -1);
               break;
            }
         }
      }
      else if (GTK_WIDGET_VISIBLE(widget)) {
         widget->requisition.width = GTK_CONTAINER(widget)->border_width * 2;
         widget->requisition.height = GTK_CONTAINER(widget)->border_width * 2;
      }
   }

   if (vis_pages && !notebook->cur_page) {
      children = gtk_notebook_search_page(notebook, NULL, STEP_NEXT, TRUE);
      if (children) {
         notebook->first_tab = children;
         gtk_notebook_switch_page(notebook, GTK_NOTEBOOK_PAGE(children), -1);
      }
   }

   return 0;
}


static gint _notebook_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
   GtkNotebook *notebook;

   CHECK_STYLE(return old_notebook_expose_event(widget, event));
   
   if (GTK_WIDGET_DRAWABLE(widget)) {
      notebook = GTK_NOTEBOOK(widget);

      gtk_notebook_paint(widget, &event->area);
      
      if (notebook->cur_page) {
         gtk_container_propagate_expose(GTK_CONTAINER(notebook), notebook->cur_page->child, event);
      }
   }
   return FALSE;
}


#if 0
static gint _notebook_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
   gboolean showarrow;
   GtkNotebookPage *page;
   GtkNotebook *notebook = GTK_NOTEBOOK(widget);
   GList *children;
   GdkRectangle rect;

   showarrow = FALSE;
   children = gtk_notebook_search_page(notebook, NULL, STEP_PREV, TRUE);
   while (children) {
      page = children->data;
      children = gtk_notebook_search_page(notebook, children, STEP_PREV, TRUE);
      if (!GTK_WIDGET_VISIBLE(page->child)) continue;

      if (!GTK_WIDGET_MAPPED(page->tab_label)) {
         showarrow = TRUE;
      }
   }

   if (showarrow && notebook->scrollable && event->button == 1) {
      int border = gtk_container_get_border_width(GTK_CONTAINER(widget));
      rect.x = widget->allocation.x + widget->allocation.width - border*2 - 2*ARROW_SIZE;
      rect.y = widget->allocation.y + border;
      rect.width = 2*ARROW_SIZE;
      rect.height = notebook->cur_page->allocation.height + engine->metric[MT_NOTEBOOK_OVERLAP];

      if (event->x >= rect.x && event->x < rect.x+rect.width && event->y >= rect.y && event->y < rect.y+rect.height) {
         event->y = rect.height/2;
      }
   }
   
   return old_notebook_button_press_event(widget, event);
}


static gint _notebook_button_release_event(GtkWidget *widget, GdkEventButton *event)
{
   return old_notebook_button_release_event(widget, event);
}
#endif


/**
 * GtkSpinButton patches:
 *
 * Purpose: Add tracking of mouse hover state for the whole widget.
 */

static gint _spin_button_enter_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
   gint ret = old_spin_button_enter_notify_event(widget, event, data);
   gtk_widget_set_state(widget, GTK_STATE_PRELIGHT);
   gtk_widget_queue_draw(widget);
   return ret;
}


static gint _spin_button_leave_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
   gint ret = old_spin_button_leave_notify_event(widget, event, data);
   gtk_widget_set_state(widget, GTK_STATE_NORMAL);
   gtk_widget_queue_draw(widget);
   return ret;
}


/**
 * GtkOptionMenu patches:
 */

#if 0
static gboolean _option_menu_timeout(gpointer data)
{
   GtkWidget *widget = (GtkWidget *)data;
   /*
   GdkEventButton event;

   event.type = GDK_BUTTON_PRESS;
   event.button = 1;
   //gtk_widget_event(widget, (GdkEvent *)(&event));
   old_option_menu_button_press_event(widget, &event);
   */
   /*
   gtk_widget_set_state(widget, GTK_STATE_NORMAL);
   gtk_widget_queue_draw(widget);
   */

   return FALSE;
}


static gint _option_menu_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
//   GdkEventButton *fakeevent;
   int ret = old_option_menu_button_press_event(widget, event);
   
   if (event->button == 1) {
      gtk_widget_set_state(widget, GTK_STATE_ACTIVE);
      gtk_widget_queue_draw(widget);

  //    fakeevent = (GdkEventButton *)g_new(GdkEventButton *, 1);
//      *fakeevent = *event;
//      fakeevent->type = GDK_BUTTON_PRESS;
      //gtk_timeout_add(1000, _option_menu_timeout, widget);
   }
   return ret;
}


static gint _option_menu_button_release_event(GtkWidget *widget, GdkEventButton *event)
{

   /* for popuping window after mouse release: */
   if (event->button == 1) {
      gtk_widget_set_state(widget, GTK_STATE_NORMAL);
      gtk_widget_queue_draw(widget);

      event->type = GDK_BUTTON_PRESS;
      old_option_menu_button_press_event(widget, event);
      event->type = GDK_BUTTON_RELEASE;
   }

   return old_option_menu_button_release_event(widget, event);
}
#endif


/**
 * GtkCombo patches:
 *
 * Purpose: Enforce size of the button at right.
 */

static void _combo_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
   GtkCombo *combo;
   GtkHBoxClass *parent_class = (GtkHBoxClass *)gtk_type_class(gtk_hbox_get_type());
   GtkAllocation entry_allocation, button_allocation;

   CHECK_STYLE(
      old_combo_size_allocate(widget, allocation);
      return;
   );

   g_return_if_fail (GTK_IS_COMBO (widget));
   g_return_if_fail (allocation != NULL);

   GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
 
   combo = GTK_COMBO (widget);

   entry_allocation = *allocation;
   entry_allocation.width -= mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH];

   button_allocation = *allocation;
   button_allocation.x += entry_allocation.width;
   button_allocation.width = mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH];
   button_allocation.y += (allocation->height - combo->entry->requisition.height) / 2;
   button_allocation.height = combo->entry->requisition.height;

   gtk_widget_size_allocate (combo->entry, &entry_allocation);
   gtk_widget_size_allocate (combo->button, &button_allocation);
}


/**
 * GtkHButtonBox patches:
 *
 * Purpose: Revert the order of standard dialog buttons to normal.
 */

#define LABEL(value) (label && !strcmp(label, value))


static void reorder_buttons(GtkBox *box)
{
   GList *child = box->children;
   int pos_ok = -1, pos_cancel = -1, pos_apply = -1, pos_help = -1;
   const char *label;
   int pos, len;

   len = g_list_length(child);
   if (len < 2) return;

   pos = 0;
   while (child) {
      GtkWidget *widget = ((GtkBoxChild *)child->data)->widget;
      
      if (GTK_IS_BUTTON(widget)) {
         label = gtk_button_get_label(GTK_BUTTON(widget));

         if (LABEL("gtk-ok") || LABEL("gtk-open") || LABEL("gtk-print") || LABEL("gtk-save") || LABEL("gtk-yes") || LABEL("gtk-find")) pos_ok = pos;
         if (LABEL("gtk-cancel") || LABEL("gtk-close") || LABEL("gtk-no")) pos_cancel = pos;
         if (LABEL("gtk-apply")) pos_apply = pos;
         if (LABEL("gtk-help")) pos_help = pos;
      }

      child = child->next;
      pos++;
   }

   /* reverse order of buttons if needed: */
   if ((pos_cancel != -1 && pos_ok > pos_cancel) || (pos_cancel == 0 && len > 1) || (pos_help == 0 && pos_cancel == 1 && len > 2)) {
      child = box->children;

      pos = len-1;
      while (child) {
         GtkWidget *widget = ((GtkBoxChild *)child->data)->widget;

         gtk_box_reorder_child(box, widget, pos);
         
         child = child->next;
         pos--;
      }
   }

   /* move Apply between Ok and Cancel if needed: */
   if (pos_ok != -1 && pos_apply != -1 && pos_cancel == pos_ok + 1 && pos_apply != pos_ok + 1) {
      GtkWidget *widget = ((GtkBoxChild *)g_list_nth_data(box->children, pos_apply))->widget;
      gtk_box_reorder_child(box, widget, pos_ok + 1);
   }
}


static void _hbutton_box_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
   if (mt_swap_buttons) {
      reorder_buttons(GTK_BOX(widget));
   }

   old_hbutton_box_size_allocate(widget, allocation);
}


/**
 * GtkRange patches:
 *
 * Purposes:
 *
 * 1. Draw a pressed version of scrollbar handle.
 * 2. Highlight track when user is page scrolling.
 */

typedef enum {
  MOUSE_OUTSIDE,
  MOUSE_STEPPER_A,
  MOUSE_STEPPER_B,
  MOUSE_STEPPER_C,
  MOUSE_STEPPER_D,
  MOUSE_TROUGH,
  MOUSE_SLIDER,
  MOUSE_WIDGET /* inside widget but not in any of the above GUI elements */
} MouseLocation;

struct _GtkRangeLayout {
  /* These are in widget->window coordinates */
  GdkRectangle stepper_a;
  GdkRectangle stepper_b;
  GdkRectangle stepper_c;
  GdkRectangle stepper_d;
  /* The trough rectangle is the area the thumb can slide in, not the
   * entire range_rect
   */
  GdkRectangle trough;
  GdkRectangle slider;

  /* Layout-related state */
  
  MouseLocation mouse_location;
  /* last mouse coords we got, or -1 if mouse is outside the range */
  gint mouse_x;
  gint mouse_y;
  /* "grabbed" mouse location, OUTSIDE for no grab */
  MouseLocation grab_location;
  gint grab_button; /* 0 if none */
};

static GtkWidget *_scrollbar_subpage = NULL;
static GtkWidget *_scrollbar_addpage = NULL;


static gint _range_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
   GtkRange *range = GTK_RANGE(widget);
   gint slider_x = range->layout->slider.x;
   gint slider_y = range->layout->slider.y;
   gint result = old_range_button_press_event(widget, event);

   if (range->layout->grab_location == MOUSE_SLIDER) {
      gtk_widget_queue_draw(widget);
   }

   if (range->layout->grab_location == MOUSE_TROUGH && event->button == 1) {
      int mp, sp, size;
   
      if (range->orientation == GTK_ORIENTATION_VERTICAL) {
         mp = event->y;
         sp = slider_y;
         size = range->layout->slider.height;
      }
      else {
         mp = event->x;
         sp = slider_x;
         size = range->layout->slider.width;
      }

      if (mp < sp) {
         _scrollbar_subpage = widget;
      }
      else if (mp >= sp+size) {
         _scrollbar_addpage = widget;
      }

      gtk_widget_queue_draw(widget);
   }

   return result;
}


static gint _range_button_release_event(GtkWidget *widget, GdkEventButton *event)
{
   gint result = old_range_button_release_event(widget, event);

   if (mt_is_range_page_pressed(widget)) {
      _scrollbar_subpage = NULL;
      _scrollbar_addpage = NULL;
      gtk_widget_queue_draw(widget);
   }

   return result;
}


int mt_is_range_slider_pressed(GtkWidget *widget)
{
   return (GTK_RANGE(widget)->layout->grab_location == MOUSE_SLIDER);
}


int mt_is_range_page_pressed(GtkWidget *widget)
{
   return (_scrollbar_addpage == widget)? MT_SCROLLBAR_ADDPAGE_ACTIVE :
          (_scrollbar_subpage == widget)? MT_SCROLLBAR_SUBPAGE_ACTIVE :
          0;
}


void mt_range_get_slider_position(GtkWidget *widget, int *pos, int *size)
{
   GtkRange *range = GTK_RANGE(widget);

   if (range->orientation == GTK_ORIENTATION_VERTICAL) {
      *pos = range->layout->slider.y - range->layout->trough.y;
      *size = range->layout->slider.height;
   }
   else {
      *pos = range->layout->slider.x - range->layout->trough.x;
      *size = range->layout->slider.width;
   }
}
