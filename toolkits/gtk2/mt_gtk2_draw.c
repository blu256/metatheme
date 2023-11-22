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

#include <stdio.h>
#include "mt_gtk2.h"
#include "mt_gtk2_patches.h"
#include "metatheme.h"


typedef void (*dispose_func)(GObject *object);
typedef void (*realize_func)(GtkStyle *style);

static dispose_func old_dispose, old_finalize;
static realize_func old_realize, old_unrealize;

typedef struct {
  GtkTreeModel *model;

  gint col_column;
  gint row_column;

  gint wrap_width;

  gint active_item;

  GtkWidget *tree_view;
  GtkTreeViewColumn *column;

  GtkWidget *cell_view;
  GtkWidget *cell_view_frame;

  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *arrow;
  GtkWidget *separator;

  GtkWidget *popup_widget;
  GtkWidget *popup_window;
  GtkWidget *popup_frame;

  guint inserted_id;
  guint deleted_id;
  guint reordered_id;
  guint changed_id;

  gint width;
  GSList *cells;

  guint popup_in_progress : 1;
  guint destroying : 1;
} ComboBoxPrivate;

typedef struct {
  GtkBin parent_instance;
  ComboBoxPrivate *priv;
} ComboBox;

static void metatheme_style_init(MetaThemeStyle *style);
static void metatheme_style_class_init(MetaThemeStyleClass *klass);

static void draw_hline (GtkStyle * style,
         GdkWindow * window,
         GtkStateType state_type,
         GdkRectangle * area,
         GtkWidget * widget,
         const gchar * detail, gint x1, gint x2, gint y);

static void draw_vline (GtkStyle * style,
         GdkWindow * window,
         GtkStateType state_type,
         GdkRectangle * area,
         GtkWidget * widget,
         const gchar * detail, gint y1, gint y2, gint x);

static void draw_shadow (GtkStyle * style,
          GdkWindow * window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle * area,
          GtkWidget * widget,
          const gchar * detail,
          gint x, gint y, gint width, gint height);

static void draw_arrow (GtkStyle * style,
         GdkWindow * window,
         GtkStateType state_type,
         GtkShadowType shadow_type,
         GdkRectangle * area,
         GtkWidget * widget,
         const gchar * detail,
         GtkArrowType arrow_type,
         gint fill,
         gint x, gint y, gint width, gint height);

static void draw_box (GtkStyle * style,
            GdkWindow * window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle * area,
            GtkWidget * widget,
            const gchar * detail,
            gint x, gint y, gint width, gint height);

static void draw_flat_box (GtkStyle * style,
                 GdkWindow * window,
                 GtkStateType state_type,
                 GtkShadowType shadow_type,
                 GdkRectangle * area,
                 GtkWidget * widget,
                 const gchar * detail,
                 gint x, gint y, gint width, gint height);

static void draw_check (GtkStyle * style,
         GdkWindow * window,
         GtkStateType state_type,
         GtkShadowType shadow_type,
         GdkRectangle * area,
         GtkWidget * widget,
         const gchar * detail,
         gint x, gint y, gint width, gint height);

static void draw_option (GtkStyle * style,
          GdkWindow * window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle * area,
          GtkWidget * widget,
          const gchar * detail,
          gint x, gint y, gint width, gint height);

static void draw_gap (GtkStyle * style,
            GdkWindow * window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle * area,
            GtkWidget * widget,
            const gchar * detail,
            gint x,
            gint y,
            gint width,
            gint height,
            GtkPositionType gap_side,
            gint gap_x, gint gap_width);

static void draw_extension (GtkStyle * style,
             GdkWindow * window,
             GtkStateType state_type,
             GtkShadowType shadow_type,
             GdkRectangle * area,
             GtkWidget * widget,
             const gchar * detail,
             gint x,
             gint y,
             gint width,
             gint height, GtkPositionType gap_side);

static void draw_slider (GtkStyle * style,
          GdkWindow * window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle * area,
          GtkWidget * widget,
          const gchar * detail,
          gint x,
          gint y,
          gint width,
          gint height, GtkOrientation orientation);

static void draw_handle (GtkStyle * style,
          GdkWindow * window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle * area,
          GtkWidget * widget,
          const gchar * detail,
          gint x,
          gint y,
          gint width,
          gint height, GtkOrientation orientation);

static void draw_resize_grip (GtkStyle * style, GdkWindow * window,
               GtkStateType state_type, GdkRectangle * area,
               GtkWidget * widget, const gchar * detail,
               GdkWindowEdge edge, gint x, gint y,
               gint width, gint height);

static void draw_layout (GtkStyle * style, GdkWindow * window,
          GtkStateType state_type, gboolean use_text,
          GdkRectangle * area, GtkWidget * widget,
          const gchar * detail, gint x, gint y,
          PangoLayout * layout);
          
static GtkStyleClass *parent_class = NULL;
static int tmp_x, tmp_y, tmp_w, tmp_h;
static int groove_start, groove_size;


#ifdef USE_AUTO_CLIPPING
static void retrieve_clip_area(GdkRectangle *area)
{
   if (area) {
      mt_clip_rect = *((MT_RECTANGLE *)area);
   }
   else {
      mt_clip_rect.x = 0;
      mt_clip_rect.y = 0;
      mt_clip_rect.width = -1;
      mt_clip_rect.height = -1;
   }
}
#else
#define retrieve_clip_area(a)
#endif


static void retrieve_state(GtkStateType state_type, GtkWidget *widget, int *state)
{
   *state = 0;

   switch (state_type) {
      case GTK_STATE_NORMAL:      *state = MT_NORMAL; break;
      case GTK_STATE_PRELIGHT:    *state = MT_HOVER;  break;
      case GTK_STATE_ACTIVE:      *state = MT_ACTIVE; break;
      case GTK_STATE_INSENSITIVE: *state = MT_DISABLED; break;
      default: break;
   }

   if (widget) {
      if (GTK_WIDGET_HAS_FOCUS(widget))     *state |= MT_SELECTED;
      if (!GTK_WIDGET_IS_SENSITIVE(widget)) *state |= MT_DISABLED;
   }
}


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


static void sanitize_size(GdkDrawable *window, gint *width, gint *height)
{
   if ((*width == -1) && (*height == -1)) {
      gdk_drawable_get_size (window, width, height);
   }
   else if (*width == -1) {
      gdk_drawable_get_size (window, width, NULL);
   }
   else if (*height == -1) {
      gdk_drawable_get_size (window, NULL, height);
   }
}


static void clear_background(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GdkRectangle *area, gint x, gint y, gint width, gint height)
{
   GdkGC *gc = style->bg_gc[state_type];
   
   gdk_gc_set_clip_rectangle(gc, area? area:NULL);
   gdk_draw_rectangle(window, gc, TRUE, x, y, width, height);
   if (area) gdk_gc_set_clip_rectangle(gc, NULL);
}


static int is_gecko(GtkWidget *widget)
{
   return (HAS_PARENT2(widget) && GTK_IS_FIXED(PARENT(widget)) && GTK_IS_WINDOW(PARENT2(widget)) && !PARENT3(widget));
}


static void check_unscrollable(GtkWidget *widget, MT_WIDGET_DATA *data)
{
   GtkAdjustment *adj;

   if (!widget || !GTK_IS_RANGE(widget)) return;

   adj = GTK_RANGE(widget)->adjustment;
   if (!adj) return;
      
   if (adj->upper > adj->page_size) return;

   /* bypass Gecko: */
   if (is_gecko(widget)) {
      return;
   }

   data->flags |= MT_SCROLLBAR_UNSCROLLABLE;
}


static void draw_hline (GtkStyle *style,
         GdkWindow *window,
         GtkStateType state_type,
         GdkRectangle *area,
         GtkWidget *widget,
         const gchar *detail, gint x1, gint x2, gint y) {

   int type = MT_HLINE;
   int state = 0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("toolbar") && widget && GTK_IS_TOOLBAR(widget)) {
      int maxw = GTK_TOOLBAR(widget)->button_maxw;
      x1 = widget->allocation.x + (widget->allocation.width - maxw)/2;
      x2 = widget->allocation.x + (widget->allocation.width + maxw)/2 - 1;
      type = MT_TOOLBAR_SEPARATOR;
      data.orientation = MT_VERTICAL;
   }

   if (DETAIL("menuitem")) {
      type = MT_MENU_ITEM_SEPARATOR;

      /* For GTK 2.4.x and above: */
      if (GTK_FULL_VERSION >= 0x020400) {
         x1 -= widget->style->xthickness;
         x2 += widget->style->xthickness + 1;
         y -= (widget->allocation.height - widget->style->ythickness) / 2;
      }

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x1, y, x2-x1+1, mt_style->engine->metric[MT_MENU_SEPARATOR_HEIGHT], &data);
      return;
   }

   mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x1, y, x2-x1+1, 0, &data);
}


static void draw_vline (GtkStyle *style,
         GdkWindow *window,
         GtkStateType state_type,
         GdkRectangle *area,
         GtkWidget *widget,
         const gchar *detail, gint y1, gint y2, gint x) {

   int type = MT_VLINE;
   int state=0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;
   
   if (HAS_PARENT3(widget) && IS_OBJECT(PARENT3(widget), "GtkComboBox")) return;

   if (DETAIL("optionmenu")) {
      return;
   }

   if (DETAIL("toolbar") && widget && GTK_IS_TOOLBAR(widget)) {
      int maxh = GTK_TOOLBAR(widget)->button_maxh;
      y1 = widget->allocation.y + (widget->allocation.height - maxh)/2;
      y2 = widget->allocation.y + (widget->allocation.height + maxh)/2 - 1;
      type = MT_TOOLBAR_SEPARATOR;
      data.orientation = MT_HORIZONTAL;
   }
   
   mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y1, 0, y2-y1+1, &data);
}


static void draw_shadow (GtkStyle *style, GdkWindow *window,
          GtkStateType state_type, GtkShadowType shadow_type,
          GdkRectangle *area,
          GtkWidget *widget, const gchar *detail,
          gint x, gint y, gint width, gint height) {

   int state=0, type=0;
   int found=0;
   MT_WIDGET_DATA data;

   g_return_if_fail (style != NULL);
   g_return_if_fail (window != NULL);

   sanitize_size (GDK_DRAWABLE (window), &width, &height); 

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("button")) {
      type = MT_BUTTON;

      if (widget && GTK_IS_BUTTON(widget)) {
         GtkWindow *win = GTK_WINDOW(gtk_widget_get_toplevel(widget));
         if (win && win->default_widget == widget) {
            type = MT_BUTTON_DEFAULT;
         }
      }
      
      if (widget && widget->parent && (
            GTK_IS_TOOLBAR(widget->parent) ||
            IS_OBJECT(widget->parent, "GtkToolButton") ||
            IS_OBJECT(widget->parent, "GtkToggleToolButton") ||
            IS_OBJECT(widget->parent, "BonoboUIToolbarButtonItem") ||
            IS_OBJECT(widget->parent, "BonoboUIToolbarToggleButtonItem")
         ))
      {
         type = MT_TOOLBAR_ITEM;

         if (GTK_IS_TOGGLE_BUTTON(widget)) {
            type = MT_TOOLBAR_ITEM_TOGGLE;
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) state |= MT_ACTIVE;
         }

         mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
         return;
      }

      if (widget && widget->parent && (GTK_IS_CLIST(widget->parent))) { /* TODO */
         type = MT_BUTTON_HEADER;
         mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
         return;
      }

      if (widget && GTK_IS_TOGGLE_BUTTON(widget)) {
         type = MT_BUTTON_TOGGLE;
         if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) state |= MT_ACTIVE; 
      }

      if (widget && widget->parent && IS_OBJECT(widget, "GtkToggleButton") && IS_OBJECT(widget->parent, "GtkComboBox")) {
         ComboBoxPrivate *priv = ((ComboBox *)(widget->parent))->priv;

         if (priv->box) {
            mt_rectangle_set(mt_clip_rect, x, y, width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height);
            mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHOICE, state, x, y, width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height, &data);
            mt_rectangle_set(mt_clip_rect, x+width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], y, mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height);
            mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHOICE_BUTTON, state, x+width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], y, mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height, &data);
         }
         else {
            if (widget->allocation.width != mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH]) {
               gtk_widget_set_size_request(widget, mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], widget->allocation.height);
            }
            mt_rectangle_set(mt_clip_rect, x, y, width, height);
            mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHOICE_BUTTON, state, x, y, width, height, &data);
         }
         return;
      }

      if (widget && widget->parent && (GTK_IS_COMBO(widget->parent) || IS_OBJECT(widget->parent, "GtkComboBox") || IS_OBJECT(widget->parent, "GtkComboBoxEntry"))) {
         if (widget->allocation.width != mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH]) {
            gtk_widget_set_size_request(widget, mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], widget->allocation.height);
         }
         mt_rectangle_set(mt_clip_rect, x, y, width, height);
         mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHOICE_BUTTON, state, x, y, width, height, &data);
         return;
      }

      if (type == MT_BUTTON && (!widget || !GTK_IS_BUTTON(widget))) {
         mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_BORDER_OUT, state, x, y, width, height, &data);
         return;
      }

      /* fix disabled state in Gecko: */
      if (type == MT_BUTTON || type == MT_BUTTON_DEFAULT) {
         if (state_type != GTK_STATE_INSENSITIVE && is_gecko(widget)) state &= ~MT_DISABLED;
      }

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("buttondefault")) return;
   
   found = 0;
   if (DETAIL("menu")) { type = MT_MENU; found=1; }
   if (DETAIL("menubar")) { type = MT_MENUBAR; found=1; }
   if (DETAIL("menuitem")) {
      if (widget && widget->parent && GTK_IS_MENU_BAR(widget->parent)) {
         type = MT_MENUBAR_ITEM;
         state ^= MT_HOVER | MT_ACTIVE;
      }
      else {
         type = MT_MENU_ITEM;
         state ^= MT_HOVER | MT_ACTIVE;
         if (state & MT_ACTIVE) state = (state & ~MT_ACTIVE) | MT_SELECTED;
         
         if (state_type == GTK_STATE_NORMAL || state_type == GTK_STATE_INSENSITIVE) {
            state &= ~(MT_SELECTED);
         }
      }
      found = 1;
   }
   if (found) {
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
      return;
   }

   found = 0;
   if (widget && GTK_IS_SCROLLBAR(widget) && DETAIL("trough")) {
      data.flags = 0;
      data.orientation = GTK_IS_HSCROLLBAR(widget)? MT_HORIZONTAL : MT_VERTICAL;
      check_unscrollable(widget, &data);
      type = MT_SCROLLBAR;
      found = 1;

      if (data.orientation == MT_VERTICAL) {
         groove_start = y;
         groove_size = height;
      }
      else {
         groove_start = x;
         groove_size = width;
      }
      
      if (widget && !is_gecko(widget) && (data.flags & MT_SCROLLBAR_UNSCROLLABLE) == 0) {
         int page = mt_is_range_page_pressed(widget);
         data.flags |= page;
         mt_range_get_slider_position(widget, &data.handle_position, &data.groove_size);
      }
   }
   if (DETAIL("hscrollbar") || DETAIL("vscrollbar")) {
      /* we save position so we can draw scrollbar arrow button at draw_arrow later: */
      tmp_x = x;
      tmp_y = y;
      tmp_w = width;
      tmp_h = height;
      return;
   }

   if (found) {
      if (widget && GTK_IS_RANGE(widget) && GTK_RANGE(widget)->adjustment &&
         GTK_RANGE(widget)->adjustment->upper <= GTK_RANGE(widget)->adjustment->page_size) {
         state |= MT_DISABLED;
      }
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("bar")) { type = MT_PROGRESSBAR_SLIDER; found = 1; }
   if (widget && GTK_IS_PROGRESS_BAR(widget) && DETAIL("trough")) { type = MT_PROGRESSBAR; found = 1; }
   if (found) {
      data.flags = 0;
      data.orientation = MT_HORIZONTAL;
      if (widget) {
         GtkProgressBarOrientation pbo = gtk_progress_bar_get_orientation(GTK_PROGRESS_BAR(widget));
         if (pbo == GTK_PROGRESS_BOTTOM_TO_TOP || pbo == GTK_PROGRESS_TOP_TO_BOTTOM) {
            data.orientation = MT_VERTICAL;
         }
         if (pbo == GTK_PROGRESS_RIGHT_TO_LEFT || pbo == GTK_PROGRESS_TOP_TO_BOTTOM) {
            data.flags |= MT_PROGRESSBAR_REVERSED;
         }
      }
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
      return;
   }

   found = 0;
   if (DETAIL("toolbar")) { type = MT_TOOLBAR; found = 1; }
   if (found) {
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("optionmenu")) {
      mt_rectangle_set(mt_clip_rect, x, y, width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height);
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHOICE, state, x, y, width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height, &data);
      mt_rectangle_set(mt_clip_rect, x+width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], y, mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height);
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHOICE_BUTTON, state, x+width - mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], y, mt_style->engine->metric[MT_CHOICE_BUTTON_WIDTH], height, &data);
      return;
   }

   if (DETAIL("trough") && widget && GTK_IS_SCALE(widget)) {
      data.orientation = GTK_IS_HSCALE(widget)? MT_HORIZONTAL : MT_VERTICAL;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_SCALE, state, x, y, width, height, &data);
      return;
   }
   
   if (DETAIL("frame") && widget && widget->parent && GTK_IS_STATUSBAR(widget->parent)) {
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_STATUSBAR_SECTION, state, x, y, width, height, &data);
      return;
   }

   found = 0;
   if (DETAIL("spinbutton")) return;
   if (DETAIL("spinbutton_up")) { type = MT_SPINBUTTON_UP; found = 1; }
   if (DETAIL("spinbutton_down")) { type = MT_SPINBUTTON_DOWN; found = 1; }
   if (found) {
      data.flags = 0;
      if (widget && widget->state == GTK_STATE_PRELIGHT) data.flags |= MT_SPINBUTTON_HOVER;

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("entry")) {
      type = MT_ENTRY_BORDER;
      if (widget && GTK_IS_SPIN_BUTTON(widget)) type = MT_SPINBUTTON;
      
      if (widget && widget->parent && (GTK_IS_COMBO(widget->parent) || IS_OBJECT(widget->parent, "GtkComboBoxEntry"))) type = MT_CHOICE;

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("scrolled_window")) {
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_SCROLLED_WINDOW, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("handlebox") || DETAIL("dockitem_bin")) {
      if (DETAIL("dockitem_bin")) {
         if (width > height) {
            width = 10;
         }
         else {
            height = 10;
         }
      }
      data.orientation = (width == 10)? MT_HORIZONTAL : MT_VERTICAL;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_TOOLBAR_HANDLE, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("handlebox_bin")) {
      return;
   }

   if (DETAIL("hruler") || DETAIL("vruler")) {
      clear_background(style, window, state_type, area, x, y, width, height);
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_BORDER_OUT, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("frame") && HAS_PARENT(widget) && IS_OBJECT(PARENT(widget), "GtkComboBox")) {
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHOICE, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("tearoffmenuitem")) {
      data.flags = 0;

      if (GTK_TEAROFF_MENU_ITEM(widget)->torn_off) data.flags |= MT_MENU_DETACHED;
      
      if (widget->state == GTK_STATE_PRELIGHT) {
         state |= MT_SELECTED;
      }

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_MENU_TEAROFF, state, x, y, width, height, &data);
      return;
   }

   switch (shadow_type) {
      case GTK_SHADOW_IN:  type = MT_BORDER_IN; break;
      case GTK_SHADOW_OUT: type = MT_BORDER_OUT; break;
      default: return;
   }
   mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
   
   DEBUG_MSG("shadow %s\n", detail);
}


static void draw_arrow (GtkStyle *style,
         GdkWindow *window,
         GtkStateType state_type,
         GtkShadowType shadow_type,
         GdkRectangle *area,
         GtkWidget *widget,
         const gchar *detail,
         GtkArrowType arrow_type,
         gboolean fill, gint x, gint y,
         gint width, gint height) {
  
   int state=0, type=0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("arrow") && HAS_PARENT2(widget)) {
      if ((GTK_IS_COMBO(PARENT2(widget)) ||
         IS_OBJECT(PARENT2(widget), "GtkComboBox") ||
         IS_OBJECT(PARENT2(widget), "GtkComboBoxEntry"))) return;

      if (HAS_PARENT3(widget) && IS_OBJECT(PARENT3(widget), "GtkComboBox")) return;
   }

   if (DETAIL("spinbutton")) return;

   switch (arrow_type) {
      case GTK_ARROW_UP:    type = MT_ARROW_UP; break;
      case GTK_ARROW_DOWN:  type = MT_ARROW_DOWN; break;
      case GTK_ARROW_LEFT:  type = MT_ARROW_LEFT; break;
      case GTK_ARROW_RIGHT: type = MT_ARROW_RIGHT; break;
   }

   if (DETAIL("menuitem")) {
      if (state & MT_HOVER) state = (state & ~MT_HOVER) | MT_SELECTED;

      /* For GTK 2.4.x and above: */
      if (GTK_FULL_VERSION >= 0x020400) {
         x -= 2;
      }
      
      type = MT_MENU_ITEM_ARROW;
   }

   if (DETAIL("hscrollbar") || DETAIL("vscrollbar")) {
      data.flags = 0;
      check_unscrollable(widget, &data);

      if (data.flags & MT_SCROLLBAR_UNSCROLLABLE) {
         state &= ~MT_ACTIVE;
      }

      x = tmp_x;
      y = tmp_y;
      width = tmp_w;
      height = tmp_h;
      type = type - MT_ARROW_UP + MT_SCROLLBAR_ARROW_UP;
   }

   if (DETAIL("notebook")) {
      type = (type == MT_ARROW_LEFT)? MT_NOTEBOOK_ARROW_LEFT : MT_NOTEBOOK_ARROW_RIGHT;
   }
   
   mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
   
   if (type == 0) {
      DEBUG_MSG("arrow %s\n", detail);
   }
}


static void draw_box (GtkStyle *style,
            GdkWindow *window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle *area,
            GtkWidget *widget,
            const gchar *detail,
            gint x, gint y, gint width, gint height) {

   g_return_if_fail (style != NULL);
   g_return_if_fail (window != NULL);
   
   sanitize_size (GDK_DRAWABLE (window), &width, &height);

   if (area) {
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
   }

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);

   gtk_style_apply_default_background (style, window,
      widget && !GTK_WIDGET_NO_WINDOW (widget),
      state_type, area, x, y, width, height);

   if (area) {
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
   }

   gtk_paint_shadow (style, window, state_type, 
      shadow_type, area, widget, 
      detail, x, y, width, height);
}


static void draw_flat_box (GtkStyle *style,
            GdkWindow *window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle *area,
            GtkWidget *widget,
            const gchar *detail,
            gint x, gint y, gint width, gint height) {

   int state=0, type=0;
   MT_WIDGET_DATA data;

   g_return_if_fail (style != NULL);
   g_return_if_fail (window != NULL);
  
   sanitize_size (GDK_DRAWABLE (window), &width, &height);

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("base"))        { type = MT_BASE; goto found; }
   if (DETAIL("viewportbin")) { type = MT_WINDOW_BASE; goto found; }

   if (DETAIL("checkbutton")) {
      if (widget && GTK_IS_RADIO_BUTTON(widget)) { type = MT_RADIO_BUTTON; goto found; }
      if (widget && GTK_IS_CHECK_BUTTON(widget)) { type = MT_CHECK_BUTTON; goto found; }
   }

   if (DETAIL("tooltip")) { type = MT_TOOLTIP; goto found; }

   if (DETAIL("entry_bg")) { /* MT_ENTRY (we draw it in shadow handler) */
      type = MT_ENTRY;
      goto found;
   }

   if (DETAIL("curve_bg")) {
      clear_background(style, window, state_type, area, x, y, width, height);
      return;
   }

   /*
   eventbox
   listitem
   text
   treeitem
   */

   DEBUG_MSG("flat_box %s\n", detail);
   parent_class->draw_flat_box(style, window, state_type, shadow_type, area, widget, detail, x, y, width, height);
   return;

found:
   mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
}


static void draw_check (GtkStyle *style,
         GdkWindow *window,
         GtkStateType state_type,
         GtkShadowType shadow_type,
         GdkRectangle *area,
         GtkWidget *widget,
         const gchar *detail,
         gint x, gint y, gint width, gint height)  {

   int state=0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("check") && widget && GTK_IS_CHECK_MENU_ITEM(widget)) {
      if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
         state |= MT_ACTIVE;
      }
      if (state & MT_HOVER) state = (state & ~MT_HOVER) | MT_SELECTED;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_MENU_ITEM_CHECK, state, x, y, width, height, &data);
      return;
   }
   
   if (widget && GTK_IS_CHECK_BUTTON(widget)) {
      if (BIT_IS_SET(state, MT_ACTIVE)) {
         state |= MT_MOUSE_ACTIVE;
         state &= ~MT_ACTIVE;
      }
      
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
         state |= MT_ACTIVE;
      }

      data.flags = 0;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_CHECK_BOX, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("cellcheck") || DETAIL("cellradio")) {
      if (state & MT_ACTIVE) state |= MT_SELECTED;
      state &= ~MT_ACTIVE;
      if (shadow_type == GTK_SHADOW_IN) state |= MT_ACTIVE;

      x -= (mt_style->engine->metric[MT_CHECKBOX_SIZE] - width) / 2;
      y -= (mt_style->engine->metric[MT_CHECKBOX_SIZE] - height) / 2;
      width = mt_style->engine->metric[MT_CHECKBOX_SIZE];
      height = mt_style->engine->metric[MT_CHECKBOX_SIZE];

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, DETAIL("cellcheck")? MT_CHECK_CELL : MT_RADIO_CELL, state, x, y, width, height, &data);
      return;
   }
}

static void draw_option (GtkStyle *style,
          GdkWindow *window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle *area,
          GtkWidget *widget,
          const gchar *detail,
          gint x, gint y, gint width, gint height) {

   int state=0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("option") && widget && GTK_IS_RADIO_MENU_ITEM(widget)) {
      if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
         state |= MT_ACTIVE;
      }
      if (state & MT_HOVER) state = (state & ~MT_HOVER) | MT_SELECTED;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_MENU_ITEM_RADIO, state, x, y, width, height, &data);
      return;
   }
   
   if (widget && GTK_IS_RADIO_BUTTON(widget)) {
      if (BIT_IS_SET(state, MT_ACTIVE)) {
         state |= MT_MOUSE_ACTIVE;
         state &= ~MT_ACTIVE;
      }
      
      if (shadow_type == GTK_SHADOW_IN) {
         state |= MT_ACTIVE;
      }

      data.flags = 0;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_RADIO_BOX, state, x, y, width, height, &data);
      return;
   }
}


static void draw_gap (GtkStyle *style,
            GdkWindow *window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle *area,
            GtkWidget *widget,
            const gchar *detail,
            gint x, gint y, gint width, gint height,
            GtkPositionType gap_side,
            gint gap_x, gint gap_width) {

   int state = 0;
   MT_WIDGET_DATA data;

   g_return_if_fail (style != NULL);
   g_return_if_fail (window != NULL);

   sanitize_size (GDK_DRAWABLE (window), &width, &height);

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   data.gap_x = gap_x;
   data.gap_width = gap_width;
   switch (gap_side) {
      case GTK_POS_LEFT:   data.gap_position = MT_POSITION_LEFT; break;
      case GTK_POS_RIGHT:  data.gap_position = MT_POSITION_RIGHT; break;
      case GTK_POS_TOP:    data.gap_position = MT_POSITION_TOP; break;
      case GTK_POS_BOTTOM: data.gap_position = MT_POSITION_BOTTOM; break;
   }

   if (DETAIL("notebook")) {
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_NOTEBOOK, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("frame")) {
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_STATICBOX, state, x, y, width, height, &data);
      return;
   }

   DEBUG_MSG("draw_gap %s\n", detail);
}


static void draw_extension (GtkStyle *style,
             GdkWindow *window,
             GtkStateType state_type,
             GtkShadowType shadow_type,
             GdkRectangle *area,
             GtkWidget *widget,
             const gchar *detail,
             gint x, gint y, gint width, gint height,
             GtkPositionType gap_side)  {

   int state = 0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("tab")) {
      switch (gap_side) {
         case GTK_POS_LEFT:   data.gap_position = MT_POSITION_RIGHT; break;
         case GTK_POS_RIGHT:  data.gap_position = MT_POSITION_LEFT; break;
         case GTK_POS_TOP:    data.gap_position = MT_POSITION_BOTTOM; break;
         case GTK_POS_BOTTOM: data.gap_position = MT_POSITION_TOP; break;
      }
      state ^= MT_ACTIVE; /* TODO: needs more investigation... */
      if (state & MT_ACTIVE) state = (state & ~MT_ACTIVE) | MT_SELECTED;

      DEBUG_MSG("%d\n", state);
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_NOTEBOOK_TAB, state, x, y, width, height, &data);
      return;
   }

   DEBUG_MSG("extension %s\n", detail);
}


static void draw_slider (GtkStyle *style,
          GdkWindow *window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle *area,
          GtkWidget *widget,
          const gchar *detail,
          gint x, gint y, gint width, gint height,
          GtkOrientation orientation) {

   int state=0;
   MT_WIDGET_DATA data;
   GdkRectangle *trough;
   
   g_return_if_fail (style != NULL);
   g_return_if_fail (window != NULL);
  
   sanitize_size (GDK_DRAWABLE (window), &width, &height); 
   
   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   switch (orientation) {
      case GTK_ORIENTATION_HORIZONTAL: data.orientation = MT_HORIZONTAL; break;
      case GTK_ORIENTATION_VERTICAL:   data.orientation = MT_VERTICAL; break;
   }
   
   if (DETAIL("slider")) {
      data.flags = 0;
      check_unscrollable(widget, &data);
      if (!(data.flags & MT_SCROLLBAR_UNSCROLLABLE) && widget && GTK_IS_RANGE(widget) && mt_is_range_slider_pressed(widget)) {
         state |= MT_ACTIVE;
      }

      data.handle_position = 0;
      data.groove_size = 0;
      
      if (widget) {
         if (data.orientation == MT_VERTICAL) {
            data.handle_position = y - groove_start - mt_style->engine->metric[MT_SCROLLBAR_SIZE];
         }
         else {
            data.handle_position = x - groove_start - mt_style->engine->metric[MT_SCROLLBAR_SIZE];
         }

         data.groove_size = groove_size - 2*mt_style->engine->metric[MT_SCROLLBAR_SIZE];
      }

      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_SCROLLBAR_HANDLE, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("hscale") || DETAIL("vscale")) {
      data.orientation = DETAIL("hscale")? MT_HORIZONTAL : MT_VERTICAL;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_SCALE_HANDLE, state, x, y, width, height, &data);
      return;
   }
   
   DEBUG_MSG("slider %s\n", detail);
}

static void draw_handle (GtkStyle *style,
          GdkWindow *window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle *area,
          GtkWidget *widget,
          const gchar *detail,
          gint x, gint y, gint width, gint height,
          GtkOrientation orientation) {

   int state = 0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;
   data.orientation = (orientation == GTK_ORIENTATION_HORIZONTAL)? MT_HORIZONTAL : MT_VERTICAL;

   if (DETAIL("handlebox")) {
      data.orientation = (width == 10)? MT_HORIZONTAL : MT_VERTICAL;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_TOOLBAR_HANDLE, state, x, y, width, height, &data);
      return;
   }

   if (DETAIL("paned")) {
      data.orientation = (width > height)? MT_HORIZONTAL : MT_VERTICAL;
      mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_SPLITTER, state, x, y, width, height, &data);
      return;
   }
   
   DEBUG_MSG("handle '%s'\n", detail);
}


static void draw_resize_grip (GtkStyle *style, GdkWindow *window,
               GtkStateType state_type, GdkRectangle *area,
               GtkWidget *widget, const gchar *detail,
               GdkWindowEdge edge, gint x, gint y,
               gint width, gint height) {

   int state=0;
   MT_WIDGET_DATA data;
   
   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, MT_RESIZE_GRIP, state, x, y, width, height, &data);
}


static void draw_layout (GtkStyle *style, GdkWindow *window,
          GtkStateType state_type, gboolean use_text,
          GdkRectangle *area, GtkWidget *widget,
          const gchar *detail, gint x, gint y,
          PangoLayout *layout) {

   MT_GTK_STRING str;
   int type = 0;
   int state = 0;

   /*if (state_type == GTK_STATE_INSENSITIVE) state |= MT_DISABLED;*/
   retrieve_state(state_type, widget, &state);

   str.color = NULL;

   /*
   TODO:
   - MT_BUTTON_HEADER in file dialog is wrongly detected as a MT_BUTTON
   - "radio buttons" test in testgtk is not affected
   */

   if ((HAS_PARENT(widget) && DETAIL("label")) || DETAIL("accellabel")) {
      GtkWidget *w;

      if ((w = mt_get_ancestor(PARENT(widget), "GtkButton")) && !GTK_IS_CLIST(PARENT3(widget))) {
         GtkWindow *win = GTK_WINDOW(gtk_widget_get_toplevel(widget));
         if (win && win->default_widget == PARENT(widget)) {
            type = MT_BUTTON_DEFAULT;
         }
         else {
            type = MT_BUTTON;
         }
      }
      else if (IS_OBJECT(PARENT(widget), "GtkToggleButton")) {
         type = MT_BUTTON_TOGGLE;
         state &= ~MT_DISABLED;
         if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(PARENT(widget)))) state |= MT_ACTIVE;
      }
      else if (GTK_IS_MENU_ITEM(PARENT(widget))) {
         type = MT_MENU_ITEM;

         state &= ~MT_SELECTED;
         if ((state & MT_HOVER)) state |= MT_SELECTED;
         state &= (MT_DISABLED | MT_SELECTED);
      }

      /* filter out MT_TOOLBAR_*: */
      if (HAS_PARENT3(widget) && (
            GTK_IS_TOOLBAR(PARENT3(widget)) ||
            IS_OBJECT(PARENT3(widget), "GtkToolButton") ||
            IS_OBJECT(PARENT3(widget), "GtkToggleToolButton") ||
            IS_OBJECT(PARENT3(widget), "BonoboUIToolbarButtonItem") ||
            IS_OBJECT(PARENT3(widget), "BonoboUIToolbarToggleButtonItem")
         ))
      {
         type = MT_NONE;
      }
   }

   mt_style = METATHEME_STYLE(style);

   if (widget && GTK_IS_LABEL(widget) && widget->parent && GTK_IS_MENU_ITEM(widget->parent) &&
       widget->parent->parent && GTK_IS_MENU_BAR(widget->parent->parent)) {

      type = MT_MENUBAR_ITEM;
      state &= MT_DISABLED;
      if (state_type == GTK_STATE_PRELIGHT) state |= MT_SELECTED;
      if (state_type == GTK_STATE_ACTIVE) state |= MT_HOVER;

      if (state & MT_SELECTED) {
         /* TODO: should be done in another way so not only text is affected... */
         x += mt_style->engine->metric_size[MT_MENUBAR_ITEM_TEXT_OFFSET].x;
         y += mt_style->engine->metric_size[MT_MENUBAR_ITEM_TEXT_OFFSET].y;
      }
   }

   if (DETAIL("progressbar")) {
      type = (state_type == GTK_STATE_PRELIGHT)? MT_PROGRESSBAR_SLIDER : MT_PROGRESSBAR;
      state &= MT_DISABLED;

      if (type == MT_PROGRESSBAR_SLIDER) {
         str.color = &mt_style->engine->palette[MT_SELECTED_FOREGROUND];
      }
   }

   /* filter out unsupported states: */
   if (type == MT_NONE) state &= MT_DISABLED;

   str.parent_class = parent_class;
   str.style = style;
   str.state_type = state_type;
   str.use_text = use_text;
   str.area = area;
   str.widget = widget;
   str.detail = detail;
   str.x = x;
   str.y = y;
   str.layout = layout;
   
   mt_style->engine->draw_string(mt_style->engine, (MT_WINDOW *)window, type, state, (MT_STRING *)(&str));
}


static void draw_focus (GtkStyle *style,
          GdkWindow *window,
          GtkStateType state_type,
          GdkRectangle *area,
          GtkWidget *widget,
          const gchar *detail,
          gint x, gint y, gint width, gint height) {

   int type=0;
   int state=0;
   MT_WIDGET_DATA data;

   mt_style = METATHEME_STYLE(style);

   retrieve_clip_area(area);
   retrieve_state(state_type, widget, &state);
   retrieve_colors(state_type, style, &data);

   data.widget = (MT_WIDGET)widget;

   if (DETAIL("tab")) { type = MT_FOCUS_TAB; goto draw; }
   if (DETAIL("checkbutton")) { type = MT_FOCUS_CHECKBOX; goto draw; }

   DEBUG_MSG("focus %s\n", detail);
   return;

draw:
   mt_style->engine->draw_widget(mt_style->engine, (MT_WINDOW *)window, (MT_RECTANGLE *)area, type, state, x, y, width, height, &data);
}


static void draw_tab(GtkStyle *style,
         GdkWindow *window,
         GtkStateType state_type,
         GtkShadowType shadow_type,
         GdkRectangle *area,
		   GtkWidget *widget,
         const gchar *detail,
         gint x, gint y, gint width, gint height)
{
   mt_style = METATHEME_STYLE(style);

   if (DETAIL("optionmenutab")) return;
   DEBUG_MSG("tab '%s'\n", detail);
}


GType metatheme_type_style = 0;

void metatheme_style_register_type(GTypeModule *module)
{
   static const GTypeInfo object_info = {
      sizeof(MetaThemeStyleClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)metatheme_style_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(MetaThemeStyle),
      0, /* n_preallocs */
      (GInstanceInitFunc)metatheme_style_init
   };

   metatheme_type_style = g_type_module_register_type(module, GTK_TYPE_STYLE, "MetaThemeStyle", &object_info, 0);
}


static void metatheme_style_init(MetaThemeStyle *style)
{
   style->engine = NULL;
   style->realized = 0;
}


static void metatheme_style_realize(GtkStyle *style)
{
   mt_style = METATHEME_STYLE(style);

   mt_style->engine = mt_get_engine();
   if (!mt_style->engine->realized) {
      mt_style->engine->realize(mt_style->engine);
      mt_style->engine->realized = 1;
   }

   old_realize(style);
}


static void metatheme_style_unrealize(GtkStyle *style)
{
   mt_style = METATHEME_STYLE(style);

   mt_return_engine(mt_style->engine);
   mt_style->engine = NULL;
   
   old_unrealize(style);
}


static void metatheme_style_dispose(GObject *obj)
{
   mt_style = METATHEME_STYLE(obj);
   if (mt_style->engine) {
      mt_return_engine(mt_style->engine);
      mt_style->engine = NULL;
   }
   
   old_dispose(obj);
}


static void metatheme_style_finalize(GObject *obj)
{
   mt_style = METATHEME_STYLE(obj);
   if (mt_style->engine) {
      mt_return_engine(mt_style->engine);
      mt_style->engine = NULL;
   }

   old_finalize(obj);
}


static void metatheme_style_class_init(MetaThemeStyleClass *klass)
{
   GtkStyleClass *style_class = GTK_STYLE_CLASS(klass);
   GObjectClass *object_class = G_OBJECT_CLASS(klass);

   parent_class = g_type_class_peek_parent(klass);

   old_dispose = object_class->dispose;
   old_finalize = object_class->finalize;

   object_class->dispose = metatheme_style_dispose;
   object_class->finalize = metatheme_style_finalize;

   style_class->draw_hline = draw_hline;
   style_class->draw_vline = draw_vline;
   style_class->draw_shadow = draw_shadow;
   style_class->draw_arrow = draw_arrow;
   style_class->draw_box = draw_box;
   style_class->draw_flat_box = draw_flat_box;
   style_class->draw_check = draw_check;
   style_class->draw_option = draw_option;
   style_class->draw_shadow_gap = draw_gap;
   style_class->draw_box_gap = draw_gap;
   style_class->draw_extension = draw_extension;
   style_class->draw_slider = draw_slider;
   style_class->draw_handle = draw_handle;
   style_class->draw_resize_grip = draw_resize_grip;
   style_class->draw_layout = draw_layout;
   style_class->draw_focus = draw_focus;
   style_class->draw_tab = draw_tab;

   old_realize = style_class->realize;
   old_unrealize = style_class->unrealize;

   style_class->realize = metatheme_style_realize;
   style_class->unrealize = metatheme_style_unrealize;
}   
