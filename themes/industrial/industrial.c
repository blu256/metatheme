/**
 * A variant of the GTK+ style Industrial
 * Copyright (c) 2004 Olaf Leidinger <leidola@web.de>
 * 
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

/*
  This is a theme, based uppon the Industrial theme but sharing no line of code.

  Some parts are copied (and eventually modified) from the plastik theme.

*/

#include <stdio.h>
#include <malloc.h>
#include "metatheme.h"
#include "metathemeInt.h"


#ifndef FALSE
  #define FALSE 0
#endif



/*
  TODO:

  - add the missing widgets.

  - draw nice dots onto the scrollbar and the toolbar grip
    (doesn't it look good without them?!?)

  - fix the toolbar and the menu checkmark (problems with mt_qt?!?)

  - have a look at the MT_MENU_ITEM_RADIO; test if it behaves
    properly using mt_qt

  - do some testing to find out if all widgets behave properly

*/



/************************************************************/
/* configuration
 */


#define ALTERNATE_TOOLBAR
/* use this toolbar, as I can't create an industrial like toolbar
   atm. See comment later in the switch command
*/


#define ALTERNATE_MENU_CHECKMARK
/*
   mt_qt draws a checkmark as menuentry icon, if the entry has
   no icon.
*/

#define MT_MOTHER_THEME "redmond"


/************************************************************/


/* some flags for the drawing function */

#define BORDER_ROUND   1 << 0
#define BORDER_LEFT    1 << 1
#define BORDER_RIGHT   1 << 2
#define BORDER_TOP     1 << 3
#define BORDER_BOTTOM  1 << 4
#define BORDER_COMPLETE  (BORDER_LEFT | BORDER_RIGHT | BORDER_TOP | BORDER_BOTTOM)

#define GRADIENT_LEFT   1 << 5
#define GRADIENT_RIGHT  1 << 6
#define GRADIENT_TOP    1 << 7
#define GRADIENT_BOTTOM 1 << 8
#define GRADIENT_COMPLETE (GRADIENT_LEFT | GRADIENT_RIGHT | GRADIENT_TOP | GRADIENT_BOTTOM)

#define GRADIENT_SHARPEN 1 << 9


#define CONTROL_RAISED   1 << 10
#define CONTROL_INSERT   1 << 11
#define CONTROL_FLAT     (CONTROL_RAISED | CONTROL_INSERT)

#define BORDER_3D        1 << 12 /* use border line to create a 3D effect */
#define GRADIENT_3D      1 << 13 /* use gradient to create a 3D effect */
#define MEDIUM_3D        1 << 14 /* don't raise or sink the control too much*/

#define SELECTION_BORDER 1 << 15

#define BORDER_HIGHLIGHT_LEFT 1 << 16 /* add an lighter line around the border */
#define BORDER_HIGHLIGHT_TOP  1 << 17 /* only with BORDER_ROUND */

#define BORDER_HIGHLIGHT_COMPLETE (BORDER_HIGHLIGHT_TOP | BORDER_HIGHLIGHT_LEFT)

#define BORDER_ROUND_FILL_CORNER 1 << 18

#define BORDER_LIGHT_CORNER_LESS 1 << 19


#define DATA(name) ((ENGINE_DATA *)engine->data)->name

typedef struct {
  MT_COLOR col_dark1,
    col_dark2,
    col_white,
    col_black,
    col_grey,
    col_selected,
    col_yellow,
    col_tooltip;

  MT_GC *gc_dark1,
    *gc_dark2,
    *gc_white,
    *gc_black,
    *gc_grey,
    *gc_selected,
    *gc_yellow,
    *gc_tooltip;

} ENGINE_DATA;

static MT_ENGINE *engine;
static MT_ENGINE *mt_mother_theme;


/* private drawing functions */

void draw_box (MT_WINDOW *win, MT_WIDGET_DATA *data,
               MT_COLOR *fg, MT_COLOR *bg,
               int x, int y,
               int width, int height,
               int flags, int state);


/* checkmark_bits */
static unsigned char checkmark_base_bits[] = {
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    0x0,    0x80,   0x0,    0x40,   0x0,    0x4c,   0x0,    0x38,
    0x0,    0x30,   0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0};

static unsigned char checkmark_alpha1_bits[] = {
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x2,    0x0,
    0x1,    0x0,    0x0,    0x0,    0x0,    0x20,   0x0,    0x44,   0x0,
    0x0,    0x0,    0x20,   0x0,    0x0,    0x0,    0x0,    0x0};

static unsigned char checkmark_alpha2_bits[] = {
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x1,    0x80,
    0x0,    0x40,   0x0,    0x8c,   0x0,    0x2,    0x0,    0x0,
    0x0,    0x40,   0x0,    0x10,   0x0,    0x0,    0x0,    0x0,    0x0};

static unsigned char checkmark_alpha3_bits[] = {
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    0x2 ,   0x0,    0x1,    0x20,   0x0,    0x90,   0x0,    0x0,
    0x0,    0x8,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0};

#define CHECKMARK_BOX_SIZE 9+3

/************************************************************/
/* from the pastic theme */


#define CHECKMARK_SIZE 9 /* 9x9 */

/* radio button */

static unsigned char radiobutton_mask_bits[] = {
   0xf8, 0x03, 0xfc, 0x07, 0xfe, 0x0f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xfe, 0x0f, 0xfc, 0x07,
   0xf8, 0x03};

static unsigned char radiobutton_contour_bits[] = {
   0xf0, 0x01, 0x0c, 0x06, 0x02, 0x08, 0x02, 0x08, 0x01, 0x10, 0x01, 0x10,
   0x01, 0x10, 0x01, 0x10, 0x01, 0x10, 0x02, 0x08, 0x02, 0x08, 0x0c, 0x06,
   0xf0, 0x01};
static unsigned char radiobutton_aa_inside_bits[] = {
   0x00, 0x00, 0x10, 0x01, 0x04, 0x04, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x04, 0x04, 0x10, 0x01,
   0x00, 0x00};
static unsigned char radiobutton_aa_outside_bits[] = {
   0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00,
   0x08, 0x02};
static unsigned char radiobutton_highlight1_bits[] = {
   0x00, 0x00, 0xf0, 0x01, 0x1c, 0x07, 0x04, 0x04, 0x06, 0x0c, 0x02, 0x08,
   0x02, 0x08, 0x02, 0x08, 0x06, 0x0c, 0x04, 0x04, 0x1c, 0x07, 0xf0, 0x01,
   0x00, 0x00};
static unsigned char radiobutton_highlight2_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x18, 0x03, 0x08, 0x02, 0x04, 0x04,
   0x04, 0x04, 0x04, 0x04, 0x08, 0x02, 0x18, 0x03, 0xe0, 0x00, 0x00, 0x00,
   0x00, 0x00};

/* radiomark */
#define RADIOMARK_SIZE 9 /* 9x9 */
#define RADIOMARK_BOX_SIZE RADIOMARK_SIZE +4

static unsigned char radiomark_aa_bits[] = {
   0x00, 0x00, 0x44, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x82, 0x00, 0x44, 0x00, 0x00, 0x00};
static unsigned char radiomark_dark_bits[] = {
   0x00, 0x00, 0x38, 0x00, 0x44, 0x00, 0xf2, 0x00, 0xfa, 0x00, 0xfa, 0x00,
   0x7c, 0x00, 0x38, 0x00, 0x00, 0x00};
static unsigned char radiomark_light_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x04, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* functions */

static void alpha_blend_colors(MT_COLOR *result, MT_COLOR *bg,
                               MT_COLOR *fg, int a);
static void draw_arrow(MT_WINDOW *win, MT_GC *gc,
                       int x, int y, int width, int height, int type);


static void draw_bitmap(MT_WINDOW *win, MT_GC *gc,
                        int x, int y, int w, int h,
                        unsigned char *bits, int inv);

static void draw_radio(MT_WINDOW *win, int x, int y,
                       int state, MT_COLOR *col_bg);

static void render_tab(MT_WINDOW *win, int x, int y, int width, int height,
                       int mouseOver, int selected, int bottom,
                       int pos, int triangular);


enum TabPosition {
   First = 0,
   Middle,
   Last,
   Single /* only one tab! */
};

/************************************************************/

void metatheme_init(MT_ENGINE *engine)
{
    int i;

    /* initialization: */

   engine->data = (void *)calloc(1, sizeof(ENGINE_DATA));

   /* palette and colors stuff: */
   mt_color_set(DATA(col_black),     0,   0,   0);
   mt_color_set(DATA(col_dark1),    64,  64,  64);
   mt_color_set(DATA(col_dark2),   128, 128, 128);
   mt_color_set(DATA(col_white),   255, 255, 255);
   mt_color_set(DATA(col_yellow),  245, 219, 149);
   mt_color_set(DATA(col_tooltip), 255, 255, 225);

   DATA(col_grey)     = engine->palette[MT_BACKGROUND];
   DATA(col_selected) = engine->palette[MT_SELECTED_BACKGROUND];

   engine->palette[MT_BUTTON_LIGHT]    = DATA(col_white);
   engine->palette[MT_BUTTON_MIDLIGHT] = DATA(col_grey);
   engine->palette[MT_BUTTON_DARK]     = DATA(col_dark1);
   engine->palette[MT_BUTTON_MID]      = DATA(col_dark2);
   engine->palette[MT_BUTTON_SHADOW]   = DATA(col_black);


    mt_mother_theme = engine->toolkit->load_engine(engine->library, MT_MOTHER_THEME);

    if (!mt_mother_theme)
    {
        fprintf(stderr, "fatal error!\n");
        exit(1);
    }

    for (i=0; i<MT_COLOR_MAX; i++)
        mt_mother_theme->palette[i] = engine->palette[i];

    if ( 1 == mt_mother_theme->refcount )
    {
        mt_mother_theme->init(mt_mother_theme);
    }

    /* size metrics: */
    mt_set_metric_size(engine, MT_BUTTON_BORDER,             5, 5);
    mt_set_metric_size(engine, MT_BUTTON_TEXT_OFFSET,        1, 1);
    mt_set_metric_size(engine, MT_BUTTON_TOGGLE_BORDER,      5, 5);
    mt_set_metric_size(engine, MT_BUTTON_TOGGLE_TEXT_OFFSET, 1, 1);
    mt_set_metric_size(engine, MT_TOOLBAR_ITEM_BORDER,       3, 3);
    mt_set_metric_size(engine, MT_TOOLBAR_ITEM_TEXT_OFFSET,  1, 1);
    mt_set_metric_size(engine, MT_MENUBAR_ITEM_TEXT_OFFSET,  0, 0);
    mt_set_metric_size(engine, MT_MENU_BORDER,               2, 2);
    mt_set_metric_size(engine, MT_MENUBAR_ITEM_BORDER,       5, 5);
    mt_set_metric_size(engine, MT_MENU_ITEM_BORDER,  4, 2);
    mt_set_metric_size(engine, MT_MENU_BORDER,       3, 3);
    mt_set_metric_size(engine, MT_MENUBAR_BORDER,    1, 1);
    mt_set_metric_size(engine, MT_PROGRESSBAR_BORDER,        2, 2);

    /* metrics: */
    mt_set_metric(engine, MT_SCALE_SLIDER_SIZE,              11);
    mt_set_metric(engine, MT_STATICBOX_TEXT_OFFSET,          5); /* TODO */
    mt_set_metric(engine, MT_MENUBAR_ITEM_SPACING,           3);
    mt_set_metric(engine, MT_MENU_SEPARATOR_HEIGHT,          9);
    mt_set_metric(engine, MT_CHOICE_BUTTON_WIDTH,            16);
    mt_set_metric(engine, MT_TOOLBAR_ITEM_DROPDOWN_WIDTH,    13);
    mt_set_metric(engine, MT_MENU_TEAROFF_HEIGHT,            5);
    mt_set_metric(engine, MT_DETACHED_MENU_TEAROFF_HEIGHT,   10);

    mt_set_metric (engine, MT_SCALE_SLIDER_SIZE,  31);
    /* TODO: is it possible to set the height of the slider? Should be 15 px */

    mt_set_metric (engine, MT_CHECKBOX_SIZE, CHECKMARK_BOX_SIZE +1);
    mt_set_metric (engine, MT_SCROLLBAR_SIZE, 15 );
    mt_set_metric (engine, MT_SPLITTER_WIDTH, 6 );
    mt_set_metric (engine, MT_BORDER, 4 );

    mt_set_metric(engine, MT_NOTEBOOK_OVERLAP,               2);
    mt_set_metric(engine, MT_NOTEBOOK_TAB_OVERLAP,           1);
    mt_set_metric(engine, MT_NOTEBOOK_TEXT_OFFSET,           2);
    mt_set_metric(engine, MT_NOTEBOOK_ARROW_WIDTH,           12);
    mt_set_metric(engine, MT_NOTEBOOK_IS_FILLED,             0);

} /* metatheme_init */

/************************************************************/


void metatheme_realize(MT_ENGINE *engine)
{
  if ( 1 == mt_mother_theme->refcount )
  {
      mt_mother_theme->realize(mt_mother_theme);
  }

  if ( 1 == engine->refcount )
  {
      DATA(gc_dark1)    = mt_gc_new_with_foreground(&DATA(col_dark1));
      DATA(gc_dark2)    = mt_gc_new_with_foreground(&DATA(col_dark2));
      DATA(gc_white)    = mt_gc_new_with_foreground(&DATA(col_white));
      DATA(gc_black)    = mt_gc_new_with_foreground(&DATA(col_black));
      DATA(gc_grey)     = mt_gc_new_with_foreground(&DATA(col_grey));
      DATA(gc_selected) = mt_gc_new_with_foreground(&DATA(col_selected));
      DATA(gc_yellow)   = mt_gc_new_with_foreground(&DATA(col_yellow));
      DATA(gc_tooltip)  = mt_gc_new_with_foreground(&DATA(col_tooltip));
  }

} /* metatheme_realize */

/************************************************************/

void metatheme_exit(MT_ENGINE *engine)
{

    if (mt_mother_theme->refcount == 1)
    {
        mt_mother_theme->exit(mt_mother_theme);

        engine->toolkit->unload_engine(mt_mother_theme);
    }

    mt_gc_destroy(DATA(gc_dark1));
    mt_gc_destroy(DATA(gc_dark2));
    mt_gc_destroy(DATA(gc_white));
    mt_gc_destroy(DATA(gc_black));
    mt_gc_destroy(DATA(gc_grey));
    mt_gc_destroy(DATA(gc_selected));
    mt_gc_destroy(DATA(gc_tooltip));


} /* metatheme_exit */

/************************************************************/




void metatheme_draw_widget(MT_ENGINE *_engine,
                           MT_WINDOW *win,
                           MT_RECTANGLE *area,
                           int type, int state,
                           int x, int y,
                           int width, int height,
                           MT_WIDGET_DATA *data)
{
    MT_COLOR col_bg, col_fg;
    int flags = 0;

    engine = _engine;

    switch (type)
    {

    case MT_BASE:
    case MT_WINDOW_BASE:
    {
        break;
    } /* MT_BASE ; MT_WINDOW_BASE ******/

    case MT_HLINE:
    {
        MT_GC *gc;

        alpha_blend_colors(&col_fg, &DATA(col_grey), &DATA(col_black), 160);

        gc = mt_gc_new_with_foreground(&col_fg);

        mt_draw_line(win, gc, x, y, x+width, y);

        mt_gc_destroy (gc);

        break;
    } /* MT_HLINE *********************/

    case MT_VLINE:
    {
        MT_GC *gc;

        alpha_blend_colors(&col_fg, &DATA(col_grey), &DATA(col_black), 160);

        gc = mt_gc_new_with_foreground(&col_fg);

        mt_draw_line(win, gc, x, y, x, y+height);

        mt_gc_destroy (gc);

        break;
    } /* MT_VLINE *********************/


    case MT_ARROW_UP:
    case MT_ARROW_DOWN:
    case MT_ARROW_LEFT:
    case MT_ARROW_RIGHT:
    {
        if (state & MT_DISABLED) {
            draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, type);
            draw_arrow(win, DATA(gc_dark2), x, y, width, height, type);
        }
        else {
            draw_arrow(win, DATA(gc_black), x, y, width, height, type);
        }
        break;
    } /* MT_ARROW_* ***********************/

    case MT_BUTTON:
    case MT_BUTTON_DEFAULT:
    case MT_BUTTON_TOGGLE:
    case MT_BUTTON_HEADER:
    {
        col_fg = DATA(col_black);

        flags  =  BORDER_COMPLETE | GRADIENT_3D;

        if ( MT_BUTTON_HEADER != type )
            flags |= BORDER_ROUND | GRADIENT_BOTTOM | GRADIENT_RIGHT;

        if ( state & MT_DISABLED)
        {
            /* make bg color darker */
            mt_color_dark (&col_bg, &DATA(col_grey), 113);
            flags |= MEDIUM_3D;

        }
        else if  (state & MT_ACTIVE)
        {
            /* make bg color darker */
            mt_color_dark (&col_bg, &DATA(col_grey), 113);
            flags |=  GRADIENT_TOP | GRADIENT_LEFT | CONTROL_INSERT;

        } else if ( state & MT_HOVER)
        {
            /* highlight bg */
            mt_color_light (&col_bg, &DATA(col_grey), 110); /* light bg-color */
            flags |=  CONTROL_RAISED;
        } else
        {
            col_bg = DATA(col_grey);
            flags |= CONTROL_RAISED;
        }

        if (state & MT_SELECTED )
            flags |= SELECTION_BORDER;

        draw_box (win, data,  &col_fg, &col_bg,
                  x, y, width, height, flags, state);
        break;
    }  /* MT_BUTTON_* *********************/

    case MT_SCROLLBAR:
    case MT_SCALE:
    case MT_PROGRESSBAR:
    {
        flags  =  BORDER_COMPLETE;

        if (MT_SCALE != type) flags |= BORDER_ROUND ;

        mt_color_dark (&col_bg, &DATA(col_grey), 112);

        mt_color_dark (&col_fg, &DATA(col_grey), 134);

        draw_box (win, data, &col_fg, &col_bg,
                  x, y, width, height, flags, state);
        break;
    } /* MT_SCROLLBAR; MT_SCALE; MT_PROGRESSBAR  *********************/

    case MT_SCROLLBAR_ARROW_UP:
    case MT_SCROLLBAR_ARROW_DOWN:
    case MT_SCROLLBAR_ARROW_LEFT:
    case MT_SCROLLBAR_ARROW_RIGHT:
    case MT_SCROLLBAR_HANDLE:
    case MT_SCALE_HANDLE:
    case MT_PROGRESSBAR_SLIDER:

    {
        int w, h;

        col_fg = DATA(col_white);
        mt_color_light (&col_fg, &DATA(col_grey), 19);

        flags  =  BORDER_COMPLETE | BORDER_ROUND | BORDER_ROUND_FILL_CORNER;

        if ( (MT_SCALE_HANDLE != type) && ( MT_PROGRESSBAR_SLIDER != type) )
            flags |= BORDER_ROUND ;

        if ( (state & MT_DISABLED) || ( (state & MT_ACTIVE) &&
                                        (MT_SCROLLBAR_HANDLE != type) &&
                                        (MT_SCALE_HANDLE != type) ) )
        {
            /* make bg color darker */
            mt_color_dark (&col_bg, &DATA(col_grey), 113);
            flags |= MEDIUM_3D;
        } else if ( ( state & MT_HOVER ) || ( MT_PROGRESSBAR_SLIDER == type )  )
        {
            /* highlight bg */

            mt_color_light (&col_bg, &DATA(col_grey), 110);
        } else
        {
            col_bg = DATA(col_grey);
        }

        draw_box (win, data,  &col_fg, &col_bg,
                  x, y, width, height, flags, state);

        if ( (MT_SCROLLBAR_HANDLE != type) && (MT_SCALE_HANDLE != type)
             && (MT_PROGRESSBAR_SLIDER != type))
        {
            w = width/2;
            h = height/2;
            x += (width-w)/2;
            y += (height-h)/2;

            if (BIT_IS_SET(state, MT_ACTIVE))
            {
                x += 1;
                y += 1;
            }

            type =  type - MT_SCROLLBAR_ARROW_UP + MT_ARROW_UP;

            metatheme_draw_widget(engine, win, area, type,
                                  state, x, y, w, h, NULL);
        } /* type != ...BAR_HANDLE */

        break;
    } /* MT_SCROLLBAR_HANDLE ; MT_SCROLLBAR_ARROW_*;
         MT_SCALE_HANDLE; MT_PROGRESSBAR_SLIDER *********************/

    case MT_MENU:
    case MT_BORDER_IN:
    case MT_BORDER_OUT:
    case MT_SCROLLED_WINDOW:
    {
        flags = BORDER_COMPLETE;

        if (MT_MENU == type) flags |= MEDIUM_3D;

        draw_box (win, data,  &DATA(col_black), &DATA(col_grey),
                  x, y, width, height, flags, state);
        break;
    } /* MT_BORDER_IN; MT_BORDER_OUT; MT_SCROLLED_WINDOW *********************/

    case MT_TOOLBAR_HANDLE:
    case MT_TOOLBAR:
    {
        mt_color_light (&col_fg, &DATA(col_grey), 44); /* rgb 194 with no flags */

        flags = BORDER_LIGHT_CORNER_LESS;

#ifdef ALTERNATE_TOOLBAR
        if (MT_TOOLBAR_HANDLE == type)
        {
            flags |= BORDER_COMPLETE;
        }

        draw_box (win, data,  &col_fg, &DATA(col_grey),
                  x, y, width, height, flags, state);

#else

        if (MT_TOOLBAR == type)
        {
            flags |= BORDER_COMPLETE;
        }
        else
        {
            /* TODO:  check if doable

               it seems as if this can't be done as in gtk+-industrial
               as the HANDLE is on the toolbar and not next to it on the
               left or the top side
            */
            if (MT_HORIZONTAL == data->orientation)
                flags |= BORDER_LEFT | BORDER_RIGHT ;
            else
                flags |= BORDER_TOP | BORDER_BOTTOM;
        }

        draw_box (win, data,  &col_fg, &DATA(col_grey),
                  x, y, width, height, flags, state);
#endif
        /* TODO: draw dots onto the handle */
        break;
    } /* MT_TOOLBAR ; MT_TOOLBAR_HANDLE *********************/

    case MT_MENUBAR:
    {
        mt_color_light (&col_fg, &DATA(col_grey), 74); /* rgb 194 with no flags */
        draw_box (win, data,  &col_fg, &DATA(col_grey),
                  x, y, width, height, BORDER_COMPLETE, state);
        break;
    } /* MT_MENUBAR *********************/

    case MT_ENTRY:
    {
        /* TODO: find out if we need to draw something here,
                 as everything we need is drawn in
                 MT_ENTRY_BORDER
        */
        break;
    } /* MT_ENTRY *********************/

    case MT_SPINBUTTON:
    case MT_ENTRY_BORDER:
    case MT_CHOICE:
    {
        draw_box (win, data,  &DATA(col_black), &DATA(col_white),
                  x, y, width, height, BORDER_COMPLETE, state);
        break;
    } /* MT_ENTRY_BORDER ; MT_CHOICE *********************/

    case MT_CHOICE_BUTTON:
    {
        int w, h;

        col_fg = DATA(col_white);
        mt_color_light (&col_fg, &DATA(col_grey), 19);

        flags  =  BORDER_COMPLETE | BORDER_ROUND;

        if ( (state & MT_DISABLED) || ( (state & MT_ACTIVE) &&
                                        (MT_SCROLLBAR_HANDLE != type)) )
        {
            /* make bg color darker */
            mt_color_dark (&col_bg, &DATA(col_grey), 113);
            flags |= MEDIUM_3D;
        } else if ( state & MT_HOVER)
        {
            /* highlight bg */

            mt_color_light (&col_bg, &DATA(col_grey), 110);
        } else
        {
            col_bg = DATA(col_grey);
        }

        draw_box (win, data,  &col_fg, &col_bg,
                  x, y, width, height, flags, state);

        w = (int) width/2.5; /* TODO: find good values here so that the arrow*/
        h = (int) height/2.5;/*       isn't too big */
        x += (width-w)/2;
        y += (height-h)/2;

        if (BIT_IS_SET(state, MT_ACTIVE))
        {
            x += 1;
            y += 1;
        }

        metatheme_draw_widget(engine, win, area, MT_ARROW_DOWN,
                              state, x, y, w, h, NULL);


        break;
    } /* MT_CHOICE_BUTTON *********************/

    case MT_SPINBUTTON_UP:
    case MT_SPINBUTTON_DOWN:
    {
        int w, h;

        col_fg = DATA(col_white);
        mt_color_light (&col_fg, &DATA(col_grey), 19);

        if ( MT_SPINBUTTON_UP == type )
        {
            flags = BORDER_LEFT  | BORDER_TOP | BORDER_RIGHT;
        }
        else
        {
             flags =  BORDER_LEFT | BORDER_RIGHT | BORDER_BOTTOM ;
        }


        if ( (state & MT_DISABLED) || ( state & MT_ACTIVE) )
        {
            /* make bg color darker */
            mt_color_dark (&col_bg, &DATA(col_grey), 113);
            flags |= MEDIUM_3D;
        } else if ( state & MT_HOVER)
        {
            /* highlight bg */

            mt_color_light (&col_bg, &DATA(col_grey), 110);
        } else
        {
            col_bg = DATA(col_grey);
        }

        draw_box (win, data,  &col_fg, &col_bg,
                  x, y, width, height+1, flags, state);

        w = (int)width/2.1;
        h = (int)height/2.1;
        x += (width-w)/2;
        y += (height-h)/2;

        if  (MT_SPINBUTTON_UP == type)
            y +=2;
        else
            y --;


        if (BIT_IS_SET(state, MT_ACTIVE))
        {
            x += 1;
            y += 1;
        }

        type = type - MT_SPINBUTTON_UP + MT_ARROW_UP;

        metatheme_draw_widget(engine, win, area, type,
                              state, x, y, w, h, NULL);

        break;
    } /* MT_SPINBUTTON_UP; MT_SPINBUTTON_DOWN *********************/


    case MT_MENUBAR_ITEM:
    {
        if (state & MT_ACTIVE )
        {
            col_bg = DATA(col_selected);
            flags  = BORDER_COMPLETE;

            draw_box (win, data,  &DATA(col_black), &col_bg,
                  x, y, width, height, flags, state);

        }


        break;
    } /* MT_MENUBAR_ITEM */

    case MT_MENU_ITEM:
    {
        if (state & MT_SELECTED )
        {
            col_bg = DATA(col_selected);
            flags  = BORDER_COMPLETE;
        }
        else
        {
            col_bg = DATA(col_grey);
            flags = 0;
        }
        draw_box (win, data,  &DATA(col_black), &col_bg,
                  x, y, width, height, flags, state);

        break;
    } /* MT_MENU_ITEM */



/* basing uppon plastik-theme code */
    case MT_RADIO_BOX:
    case MT_RADIO_CELL:
    case MT_MENU_ITEM_RADIO:
    {

        if (type == MT_RADIO_BOX && (data->flags & MT_DRAW_MASK))
        {
            draw_bitmap(win, DATA(gc_black), x, y, RADIOMARK_BOX_SIZE,
                        RADIOMARK_BOX_SIZE,
                        radiobutton_mask_bits, 0);
            return;
        }

        if ( MT_MENU_ITEM_RADIO == type )
        {
            /* workaround to display it properly in mt_gtk2
               no idea if it works with mt_qt, as I don't know
               a mt_qt app, which has a radio item in menu
            */

            x-=3;
            y-=2;

            /* background setting */
            if ( state & MT_SELECTED )
                col_bg = DATA(col_selected);
            else
                col_bg = DATA(col_grey);
        }
        else
        {
            col_bg = DATA(col_grey);
        }

        draw_radio(win, x, y, state, &col_bg);
        break;
    } /* MT_RADIO_BOX ; MT_RADIO_CELL; MT_MENU_ITEM_RADIO  *********************/

/* end of from plastik-theme */

    case MT_RADIO_BUTTON:
    case MT_CHECK_BUTTON:
    {
        flags = 0;

        if ( state & MT_HOVER)
            mt_color_light (&col_bg, &DATA(col_grey), 110);  /* highlight bg */
        else
            col_bg = DATA(col_grey);

        /*
           TODO

           mt_qt:
             check if highlighting of the background can also be done.

           mt_gtk2:
             highlight a bigger area (probably using different METRICS

        */

        draw_box (win, data,  &DATA(col_black), &col_bg,
                  x, y, width, height, flags, state);

        break;
    } /* MT_RADIO_BUTTON; MT_CHECK_BUTTON *********************/

    case MT_CHECK_BOX:
    case MT_CHECK_CELL:
    case MT_MENU_ITEM_CHECK:
    {
        flags = BORDER_COMPLETE | BORDER_LIGHT_CORNER_LESS | BORDER_3D
            | CONTROL_INSERT;

        if (state & (MT_DISABLED | MT_MOUSE_ACTIVE))
            alpha_blend_colors (&col_bg, &DATA(col_dark2),
                                &DATA(col_white), 70);
        else
            col_bg =  DATA(col_grey);

#ifdef ALTERNATE_MENU_CHECKMARK
        /* checkmark is displayed in mt_qt for every
           menu item, which doesn't have an icon
        */
        if (MT_MENU_ITEM_CHECK != type)
        {
            draw_box (win, data,  &DATA(col_black), &col_bg,
                      x, y, CHECKMARK_BOX_SIZE, CHECKMARK_BOX_SIZE, flags, state);
        } /* if */
#else
        draw_box (win, data,  &DATA(col_black), &col_bg,
                  x, y, CHECKMARK_BOX_SIZE, CHECKMARK_BOX_SIZE, flags, state);
#endif


        if (state & MT_ACTIVE)
        {
            /* draw the checkmark if activated */
            MT_GC *gc;
            col_fg = DATA(col_black);

            gc = mt_gc_new_with_foreground (&col_fg);

            draw_bitmap(win, gc, x, y, CHECKMARK_BOX_SIZE, CHECKMARK_BOX_SIZE,
                        checkmark_base_bits, 0);

            alpha_blend_colors(&col_fg, &DATA(col_black), &col_bg, 180);
            mt_gc_set_foreground(gc, &col_fg);

            draw_bitmap(win, gc, x, y, CHECKMARK_BOX_SIZE, CHECKMARK_BOX_SIZE,
                        checkmark_alpha1_bits, 0);

            alpha_blend_colors(&col_fg, &col_fg, &col_bg, 190);
            mt_gc_set_foreground(gc, &col_fg);

            draw_bitmap(win, gc, x, y, CHECKMARK_BOX_SIZE, CHECKMARK_BOX_SIZE,
                        checkmark_alpha2_bits, 0);


            alpha_blend_colors(&col_fg, &col_fg, &col_bg, 120);
            mt_gc_set_foreground(gc, &col_fg);

            draw_bitmap(win, gc, x, y, CHECKMARK_BOX_SIZE, CHECKMARK_BOX_SIZE,
                        checkmark_alpha3_bits, 0);

            mt_gc_destroy(gc);
        } /* if (state & MT_ACTIVE) */
        break;
    } /* MT_CHECK_BOX; MT_CHECK_CELL *********************/


    case MT_TOOLBAR_ITEM:
    case MT_TOOLBAR_ITEM_TOGGLE:
    {
        if (state & (MT_ACTIVE | MT_HOVER))
        {

            mt_color_light (&col_bg, &DATA(col_grey), 110);

            flags  =  BORDER_COMPLETE | GRADIENT_3D | BORDER_ROUND |
                GRADIENT_BOTTOM | GRADIENT_RIGHT | CONTROL_RAISED;


            draw_box (win, data,  &DATA(col_black), &col_bg,
                      x, y, width, height, flags, state);
        }

        break;
    } /* MT_TOOLBAR_ITEM; MT_TOOLBAR_ITEM_TOGGLE ********************/

    case MT_TOOLBAR_ITEM_DROPDOWN:
    {
        int oldstate = state, mode=0, w, h;
        int TB_DROPDOWN_WIDTH = 26;

        if ((state & MT_ACTIVE) && (data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE))
        {
            state &= ~MT_ACTIVE;
            state |= MT_HOVER;
        }


        if ( width > 2*TB_DROPDOWN_WIDTH )
        {
            TB_DROPDOWN_WIDTH /= 2;

            metatheme_draw_widget(engine, win, area, MT_TOOLBAR_ITEM, state, x, y, width-TB_DROPDOWN_WIDTH, height, data);

            if ( oldstate & (MT_HOVER | MT_ACTIVE))
            {

                mt_color_light (&col_bg, &DATA(col_grey), 110);
                flags  =  BORDER_COMPLETE | GRADIENT_3D | BORDER_ROUND |
                    GRADIENT_BOTTOM | GRADIENT_RIGHT | CONTROL_RAISED;


                draw_box (win, data,  &DATA(col_black), &col_bg,
                          x+width-TB_DROPDOWN_WIDTH, y, TB_DROPDOWN_WIDTH, height, flags, oldstate);

            }
            x += width-TB_DROPDOWN_WIDTH;
        }
        else
        {
            /* widget is too small, draw alternate dropdown button */
            MT_GC *gc;

            metatheme_draw_widget(engine, win, area, MT_TOOLBAR_ITEM, state, x, y, width, height, data);

            TB_DROPDOWN_WIDTH/=2;
            x += width-TB_DROPDOWN_WIDTH;

            if ( oldstate & (MT_HOVER | MT_ACTIVE))
            {
                mt_color_light (&col_bg, &DATA(col_grey), 110);
                alpha_blend_colors(&col_fg, &col_bg, &DATA(col_black), 160);

                gc = mt_gc_new_with_foreground(&col_fg);
                mt_draw_line(win, gc, x, y+1, x, y+height-1);
                mt_gc_destroy(gc);

            }

        }

        state = oldstate;

        /* and now draw the arrow */

        w = (int)TB_DROPDOWN_WIDTH/2;
        h = (int)height/2;
        x += (TB_DROPDOWN_WIDTH-w)/2;
        y += (height-h)/2;

        if (BIT_IS_SET(state, MT_ACTIVE))
        {
            x += 1;
            y += 1;
        }

        type = MT_ARROW_DOWN;

        metatheme_draw_widget(engine, win, area, type,
                              state, x, y, w, h, NULL);



        break;
    } /* MT_TOOLBAR_ITEM_DROPDOWN ********************/


    case MT_TOOLBAR_SEPARATOR:
    {
        /* TODO: test it; i don't know a program with MT_TOOLBAR_SEPERATOR */

        if ( MT_HORIZONTAL == data->orientation )
        {
            type = MT_HLINE;
            x += width/4;
            width /= 2;
        }
        else
        {
            type = MT_VLINE;
            y += height/4;
            height /= 2;
        }

        metatheme_draw_widget(engine, win, area, type, state, x, y, width, height, NULL);

        break;
    } /* MT_TOOLBAR_SEPARATOR *********************/


    case MT_NOTEBOOK:
    {
        draw_box (win, data,  &DATA(col_black), &DATA(col_grey),
                  x, y, width, height, BORDER_BOTTOM | BORDER_TOP |
                  BORDER_LEFT | BORDER_RIGHT, state);

        break;
    } /* MT_NOTEBOOK *********************/

    case MT_NOTEBOOK_TAB:
    {
        /* mostly from plastik */
        int pos = Middle;

        if (data->flags & MT_NOTEBOOK_FIRST_VISIBLE_TAB)
            data->flags |= MT_NOTEBOOK_FIRST_TAB;

        if (data->flags & MT_NOTEBOOK_FIRST_TAB) pos = First;
        if (data->flags & MT_NOTEBOOK_LAST_TAB)
            pos = (data->flags & MT_NOTEBOOK_FIRST_TAB)? Single : Last;

        switch (data->gap_position)
        {
        case MT_POSITION_TOP:
            render_tab(win, x, y, width, height, state & MT_HOVER,
                       state & MT_SELECTED, 0, pos, 1);
            break;

        case MT_POSITION_BOTTOM:
            render_tab(win, x, y, width, height, state & MT_HOVER,
                       state & MT_SELECTED, 1, pos, 1);
            break;
        }

        break;

    } /* MT_NOTEBOOK_TAB *********************/


    case MT_NOTEBOOK_ARROW_LEFT:
    case MT_NOTEBOOK_ARROW_RIGHT:
    {
        /* TODO: check if possible: left arrow on the left side, right arrow on the right side. */
        x += width/2.5;
        width -= width/2.5;

        metatheme_draw_widget(engine, win, area, (type == MT_NOTEBOOK_ARROW_LEFT)? MT_ARROW_LEFT : MT_ARROW_RIGHT,
                              state, x, y, width, height, data);
        break;
    } /* MT_NOTEBOOK_ARROW_LEFT; MT_NOTEBOOK_ARROW_RIGHT *********************/

    /* from plastik theme engine */

    case MT_MENU_ITEM_ARROW:
    {
        y += (height-7)/2;
        height = 7;

        if (state & MT_DISABLED)
        {
            draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, MT_ARROW_RIGHT);
            draw_arrow(win, DATA(gc_dark2), x, y, width, height, MT_ARROW_RIGHT);
        }
        else
        {
            draw_arrow(win, (state & MT_SELECTED)? DATA(gc_white) : DATA(gc_black), x, y, width, height, MT_ARROW_RIGHT);
        }
        break;
    } /* MT_MENU_ITEM_ARROW *********************/

    case MT_MENU_ITEM_SEPARATOR:
    {
        MT_GC *gc;
        const int dx = 2;

        alpha_blend_colors(&col_fg, &DATA(col_grey), &DATA(col_black), 160);

        gc = mt_gc_new_with_foreground(&col_fg);

        mt_draw_line(win, gc, x+dx, y, x+width-2*dx, y);

        mt_gc_destroy (gc);

        break;
    } /* MT_MENU_ITEM_SEPARATOR  *********************/


    case MT_TOOLTIP:
    {
        mt_draw_rectangle(win, DATA(gc_black), 0,
                          x, y, x+width-1, y+height-1);
        mt_draw_rectangle(win, DATA(gc_tooltip), 1,
                          x+1, y+1, x+width-2, y+height-2);

        break;
    } /* MT_TOOLTIP *********************/

    case MT_STATICBOX:
    {
        flags = BORDER_COMPLETE;

        draw_box (win, data,  &DATA(col_black), &DATA(col_grey),
                  x, y, width, height, flags, state);

        if (data->gap_width > 0)
        {
            /* undraw border line */
            if (MT_POSITION_TOP == data->gap_position)
                mt_draw_line(win, DATA(gc_grey), x+data->gap_x, y,
                             x+data->gap_x+data->gap_width-1, y);

        }

        break;
    } /* MT_STATICBOX ********************/

    case MT_SPLITTER:
    {
        if ( flags & MT_HOVER )
            mt_color_light (&col_bg, &DATA(col_grey), 110);  /* highlight bg */
        else
            col_bg = DATA(col_grey);

        draw_box (win, data,  &DATA(col_black), &col_bg,
                  x, y, width, height, flags, state);

        /* TODO:
            - draw dots onto the splitter

            - splitter background is not highlighted

        */

        break;
    } /* MT_SPLITTER *********************/


    case MT_MENU_TEAROFF:
    {
        /* I've no idea what this widget looks like, so I've
           copied the code from the plastik theme and
           changed some parts to make it fit into
           this theme.
        */

        MT_COLOR col_tmp;
        MT_GC *gc1, *gc2;
        int i,max;

        metatheme_draw_widget(engine, win, area, MT_MENU_ITEM, state, x, y, width, height, data);

        x += 1;
        width -= 2;

        if (data->flags & MT_MENU_DETACHED)
        {
            draw_arrow(win, (state & MT_SELECTED)? DATA(gc_white) : DATA(gc_black),
                       x, y+1, height, height-2, MT_ARROW_LEFT);
            y += 4;
            i = 10;
         }
        else
         {
             y += 1;
             i = 0;
         }

        mt_color_dark (&col_tmp, &DATA(col_grey), 160);
        gc1 = mt_gc_new_with_foreground(&col_tmp);

        mt_color_dark (&col_tmp, &DATA(col_grey), 115);
        gc2 = mt_gc_new_with_foreground(&col_tmp);

        while (i < width)
        {
            max = i+5;
            if (max >= width) max = width;

            mt_draw_line(win, gc1, x+i, y, x+max, y);
            mt_draw_line(win, gc2, x+i, y+1, x+max, y+1);

            i += 10;
        }

        mt_gc_destroy(gc1);
        mt_gc_destroy(gc2);
        break;
    } /* MT_MENU_TEAROFF *********************/


    case MT_RESIZE_GRIP:
    {
        /*
          no idea what this could be; seems to theme some parts
          of a notebook - but as other themes leave this blank
          it seems not to be used
        */
        break;
    }

    /* this widget types are still missing: ***/

    case MT_FOCUS_TAB:
    case MT_FOCUS_CHECKBOX:
    case MT_STATUSBAR_SECTION: /* looks like in redmond theme */


    default:

        mt_mother_theme->draw_widget(mt_mother_theme,
                             win, area,
                             type, state,
                             x, y,
                             width, height,
                             data);
    } /* switch */
} /* metatheme_draw_widget */

/************************************************************/

void metatheme_draw_string(MT_ENGINE *engine,
                           MT_WINDOW *win,
                           int type, int state,
                           MT_STRING *str)
{
    if (state & MT_DISABLED) {
        mt_draw_string(win, str, 1, 1, &DATA(col_white));
    }

    mt_draw_string(win, str, 0, 0, NULL);

} /* metatheme_draw_string */

/************************************************************/


void draw_box (MT_WINDOW *win, MT_WIDGET_DATA *data,
               MT_COLOR *fg, MT_COLOR *bg,
               int x, int y,
               int width, int height,
               int flags, int state)
{
    MT_COLOR bd_col_medium, bd_col_br, bd_col_tl, bd_col_br2, bd_col_tl2;
    MT_GC *bd_gc_br = NULL, *bd_gc_br2 = NULL,  *bd_gc_med = NULL,
        *bd_gc_tl = NULL, *bd_gc_tl2 = NULL;

    MT_COLOR gr_col_tl, gr_col_br;
    MT_GC *gr_gc_tl = NULL, *gr_gc_br = NULL, *gc_bg;
    int posx, posy, curw, curh;

    const int GRAD_THICKNESS = 4;
    const int GRAD_STEPS = GRAD_THICKNESS / 2;

    /* br == bottom, right side
       tl == top, left side

       br2(,tl2) == br (,tl) (darker or lighter variant when 3D-Border is used)
    */


    int xw = x+width-1;
    int yh = y+height-1;

    if ( 0 == flags)
    {
        /* draw nothing but empty background */

        gc_bg = mt_gc_new_with_foreground (bg);
        mt_draw_rectangle(win, gc_bg, 1, x, y, x+width-1, y+height-1);

        mt_gc_destroy (gc_bg);
        return;
    } /* 0 == flags */


    /* select base drawing color */
    if (flags & MEDIUM_3D)
        alpha_blend_colors(&bd_col_medium, bg, fg, 212);
    else
        alpha_blend_colors(&bd_col_medium, bg, fg, 160);


    /* select tl and br color */

    if ( !(flags & BORDER_ROUND) && (flags & BORDER_3D) )
    {
        if ( (flags & CONTROL_FLAT) == CONTROL_FLAT )
        {
            bd_col_tl = bd_col_br =  *bg;
        }
        else if (flags & CONTROL_RAISED)
        {
            mt_color_light (&bd_col_tl, bg, 110);
            mt_color_dark (&bd_col_br, bg, 110);
        }
        else if (flags & CONTROL_INSERT)
        {
            mt_color_light (&bd_col_br, bg, 110);
            mt_color_dark (&bd_col_tl, bg,110);
        }
    } /* if BORDER_3D */
    else
    {
        bd_col_tl = bd_col_br = *bg;
    }

    /* create alternate tl and br color */
    mt_color_light (&bd_col_tl2, &bd_col_medium, 130);
    mt_color_light (&bd_col_br2, &bd_col_medium, 130);

    /* create gcs */
    bd_gc_br = mt_gc_new_with_foreground(&bd_col_br);
    bd_gc_tl = mt_gc_new_with_foreground(&bd_col_tl);
    bd_gc_med = mt_gc_new_with_foreground(&bd_col_medium);

    bd_gc_br2 = mt_gc_new_with_foreground(&bd_col_br2);
    bd_gc_tl2 = mt_gc_new_with_foreground(&bd_col_tl2);

    gc_bg = mt_gc_new_with_foreground (bg);


    height -= 1;
    width  -= 1;

    /* reset the background */
    mt_draw_rectangle(win, gc_bg, 1, x, y, x+width, y+height);


    /* for the gradient */
    posx = x+1  ;
    posy = y+1  ;
    curw = width-2 -1;
    curh = height-2 -1;

    /* first of all, draw the gradient */

    if (flags & GRADIENT_COMPLETE) /* draw anything of the gradient? */
    {
        int i;
        MT_COLOR temp1, temp2;
        MT_GC *gc_mean = NULL;

        /* set start colors */
        if (flags & CONTROL_INSERT)
        {
            mt_color_light (&gr_col_tl, &bd_col_medium, 129);
            mt_color_light (&gr_col_br, &bd_col_medium, 158);
        }
        else
        {
            if (flags & MEDIUM_3D)
            {
                mt_color_light (&gr_col_tl, bg, 102);
                mt_color_dark (&gr_col_br, bg, 102);
            } else
            {
                mt_color_light (&gr_col_tl, bg, 104);
                mt_color_dark (&gr_col_br, bg, 104);
            }
            /* 120 looks also very nice ;-) */
        }

        gr_gc_tl = mt_gc_new_with_foreground(&gr_col_tl);
        gr_gc_br = mt_gc_new_with_foreground(&gr_col_br);
        gc_mean  = mt_gc_new_with_foreground(&gr_col_br);

        for (i=0; i < GRAD_THICKNESS; i++)
        {

            /* calculate new color */
            if ( flags & (GRADIENT_LEFT | GRADIENT_TOP))
            {
                /* only half the thickness */
                if ( i < GRAD_THICKNESS/2)
                    alpha_blend_colors (&temp1, bg, &gr_col_tl,
                                        (i*2*255)/GRAD_THICKNESS );
                else
                    temp1 = *bg;

                mt_gc_set_foreground(gr_gc_tl, &temp1);
            }

            if (flags & (GRADIENT_BOTTOM | GRADIENT_RIGHT) )
            {
                alpha_blend_colors (&temp2, bg, &gr_col_br,
                                    (i*255)/GRAD_THICKNESS);
                mt_gc_set_foreground(gr_gc_br, &temp2);
            }

            if (  (flags & GRADIENT_LEFT) && (flags & GRADIENT_BOTTOM) ||
                  (flags & GRADIENT_TOP) && (flags & GRADIENT_RIGHT) )
            {
                alpha_blend_colors (&temp2, &temp1, &temp2, 125);
                mt_gc_set_foreground(gc_mean, &temp2);
            }

            /* do the drawing */

            if (flags & GRADIENT_TOP)
                mt_draw_line(win, gr_gc_tl, posx, posy, posx+curw, posy);

            if (flags & GRADIENT_RIGHT)
                mt_draw_line(win, gr_gc_br, posx+curw, posy,
                             posx+curw, posy+curh);

            if (flags & GRADIENT_BOTTOM)
                mt_draw_line(win, gr_gc_br, posx+curw,
                             posy+curh, posx, posy+curh);

            if (flags & GRADIENT_LEFT)
                mt_draw_line(win, gr_gc_tl, posx, posy, posx, posy+curh);

                if ( (flags & GRADIENT_LEFT) && (flags & GRADIENT_BOTTOM))
                mt_draw_point (win, gc_mean, posx, posy+curh);

            if ( (flags & GRADIENT_TOP) && (flags & GRADIENT_RIGHT) )
                mt_draw_point (win, gc_mean, posx+curw, posy);

            posx++;
            posy++;
            curw -= 2;
            curh -= 2;

        } /* for */

        /* destroy drawing context */
        if (gr_gc_br) mt_gc_destroy (gr_gc_br);

        if (gr_gc_tl) mt_gc_destroy (gr_gc_tl);

        if (gc_mean) mt_gc_destroy (gc_mean);

    } /* draw gradient */


    /* draw border */

    if (flags & BORDER_ROUND)
    {
        MT_COLOR temp;
        MT_GC *gc = NULL;
        int dty=0, dby=0;

        if (flags & BORDER_HIGHLIGHT_COMPLETE)
        {
            /* sall be col rgb 232 with default colors */

            /* don't use MEDIUM_3D here */

            mt_color_light (&temp, &bd_col_medium, 172);
            gc = mt_gc_new_with_foreground (&temp);
        }


        /* draw the corners a bit rounded */

        /* first of all the shortend lines */
        if (flags & BORDER_TOP)
        {
            dty = 2; /* leave out 2 px */
            mt_draw_line(win, bd_gc_med, x+2, y, xw-2, y);
        }

        if (flags & BORDER_BOTTOM)
        {
            dby = 2; /* also leave out 2 px */
            mt_draw_line(win, bd_gc_med, xw-2, yh, x+2, yh);
        }

        if (flags & BORDER_RIGHT)
            mt_draw_line(win, bd_gc_med, xw, y+dty, xw, yh-dby);


        if (flags & BORDER_LEFT)
            mt_draw_line(win, bd_gc_med, x, y+dty, x, yh-dby);

        if (flags & BORDER_HIGHLIGHT_LEFT)
            mt_draw_line(win, gc, x+1, y+2, x+1, yh-2);

        if (flags & BORDER_HIGHLIGHT_TOP)
            mt_draw_line(win, gc, x+2, y+1, xw-2, y+1);


        if (gc) mt_gc_destroy (gc);

        /* and then the diagonals */
        if ((flags & (BORDER_TOP | BORDER_LEFT) )==(BORDER_TOP | BORDER_LEFT))
        {
            mt_draw_line (win, bd_gc_tl2, x, y+1, x+1, y);
            mt_draw_point(win, bd_gc_med, x+1, y+1);
        }

        if ((flags & (BORDER_TOP | BORDER_RIGHT ))
            ==(BORDER_TOP | BORDER_RIGHT))
        {
            mt_draw_line (win, bd_gc_br2, xw-1, y, xw, y+1);
            mt_draw_point(win, bd_gc_med, xw-1, y+1);
        }

        if ((flags & (BORDER_BOTTOM | BORDER_RIGHT) )==
            (BORDER_BOTTOM | BORDER_RIGHT))
        {
            mt_draw_line (win, bd_gc_br2, xw, yh-1, xw-1, yh);
            mt_draw_point(win, bd_gc_med, xw-1, yh-1);
        }

        if ((flags & (BORDER_BOTTOM | BORDER_LEFT) )==
            (BORDER_BOTTOM | BORDER_LEFT))
        {
            mt_draw_line (win, bd_gc_tl2, x+1, yh, x, yh-1);
            mt_draw_point(win, bd_gc_med, x+1, yh-1);
        }

        if ( flags & BORDER_ROUND_FILL_CORNER)
        {
            /* draw darker bg in the corners */
            MT_COLOR bd_corner;
            MT_GC *gc_corner;

            mt_color_dark (&bd_corner, &bd_col_tl2, 104);

            gc_corner = mt_gc_new_with_foreground(&bd_corner);

            mt_draw_point(win, gc_corner, x, y);
            mt_draw_point(win, gc_corner, xw, y);
            mt_draw_point(win, gc_corner, xw, yh);
            mt_draw_point(win, gc_corner, x, yh);

            mt_gc_destroy (gc_corner);
        }
    }
    else
    {
        MT_COLOR bd_corner;
        MT_GC *gc_corner;

        /* sharper corners */
        if (flags & BORDER_TOP)
        {
            mt_draw_line(win, bd_gc_med, x, y, xw, y);
            if (flags & BORDER_3D)
                mt_draw_line(win, bd_gc_tl, x+1, y+1, xw-2, y+1);
        }

        if (flags & BORDER_RIGHT)
        {
            mt_draw_line(win, bd_gc_med, xw, y, xw, yh);
            if (flags & BORDER_3D)
                mt_draw_line(win, bd_gc_br, xw-1, y+2, xw-1, yh-1);
        }

        if (flags & BORDER_BOTTOM)
        {
            mt_draw_line(win, bd_gc_med, xw, yh, x, yh);
            if (flags & BORDER_3D)
                mt_draw_line(win, bd_gc_br, xw-1, yh-1, x+1, yh-1);
        }

        if (flags & BORDER_LEFT)
        {
            mt_draw_line(win, bd_gc_med, x, y, x, yh);
            if (flags & BORDER_3D)
                mt_draw_line(win, bd_gc_tl, x+1, y+1, x+1, yh-2);
        }

        /* select corner color */
        if (flags & BORDER_LIGHT_CORNER_LESS)
            mt_color_light (&bd_corner, &bd_col_tl2, 87);
        else
            mt_color_light (&bd_corner, &bd_col_tl2, 96); /* ca rgb 200 with
                                                          fg black, bg white */

        if (flags & BORDER_COMPLETE)             /* draw any border line ? */
        {

            gc_corner = mt_gc_new_with_foreground(&bd_corner);

            mt_draw_point(win, gc_corner, x, y);
            mt_draw_point(win, gc_corner, xw, y);
            mt_draw_point(win, gc_corner, xw, yh);
            mt_draw_point(win, gc_corner, x, yh);

            mt_gc_destroy (gc_corner);
        } /* BORDER_COMPLETE ? */


    } /* border round */

    if (flags & SELECTION_BORDER)
    {
        MT_GC *gc;
        int t1, t2, s1, s2, i;
        /* draw a dotted border into the widget */

        width = (width%2)   ? width+1 : width;
        height = (height%2) ? height+1: height;

        t1 = y + 3;
        t2 = y - 3 + height;

        s1 = x + 3;
        s2 = x - 3 + width;

        gc = mt_gc_new_with_foreground(fg);

        for (i=s1; i <= s2; i+=2)
        {
            mt_draw_point(win, gc, i, t1);
            mt_draw_point(win, gc, i, t2);
        }
        for (i=t1; i <= t2; i+=2)
        {
            mt_draw_point (win, gc, s1, i);
            mt_draw_point (win, gc, s2, i);
        }

        mt_gc_destroy(gc);

    } /* flags & SELECTION_BORDER */


    if (gc_bg) mt_gc_destroy (gc_bg);
    if (bd_gc_tl) mt_gc_destroy(bd_gc_tl);
    if (bd_gc_med) mt_gc_destroy(bd_gc_med);
    if (bd_gc_tl2) mt_gc_destroy(bd_gc_tl2);
    if (bd_gc_br) mt_gc_destroy(bd_gc_br);
    if (bd_gc_br2) mt_gc_destroy(bd_gc_br2);

} /* draw_box */


/************************************************************/
/* from the plastik-theme */
/*                        */
/**************************/

static void draw_arrow(MT_WINDOW *win, MT_GC *gc, int x, int y, int width, int height, int type)
{
   int w, h;
   MT_POINT points[3];

   if (type == MT_ARROW_UP || type == MT_ARROW_DOWN) {
      w = (width & 1)? width:width-1;
      h = w/2+1;
      if (h > height) {
         w = w * ((float)height/(float)h);
         w = (w & 1)? w:w-1;
         h = w/2+1;
         x += (width-w)/2;
      }
      y += (height-h)/2;
   }
   else {
      h = (height & 1)? height:height-1;
      w = h/2+1;
      if (w > width) {
         h = h * ((float)width/(float)w);
         h = (h & 1)? h:h-1;
         w = h/2+1;
         y += (height-h)/2;
      }
      x += (width-w)/2;
   }

   switch (type) {
      case MT_ARROW_DOWN:
         points[0].x = x;       points[0].y = y;
         points[1].x = x+w;     points[1].y = y;
         points[2].x = x+w/2;   points[2].y = y+h;
         break;

      case MT_ARROW_UP:
         points[0].x = x-1;     points[0].y = y+h;
         points[1].x = x+w/2;   points[1].y = y-1;
         points[2].x = x+w;     points[2].y = y+h;
         break;

      case MT_ARROW_LEFT:
         points[0].x = x+w;     points[0].y = y-1;
         points[1].x = x+w;     points[1].y = y+h;
         points[2].x = x;       points[2].y = y+h/2;
         break;

      case MT_ARROW_RIGHT:
         points[0].x = x;       points[0].y = y-1;
         points[1].x = x+w;     points[1].y = y+h/2;
         points[2].x = x;       points[2].y = y+h;
         break;
   }

   mt_draw_polygon(win, gc, 1, points, 3);
} /* draw_arrow */

/************************************************************
 *                        *
 * from the plastik theme *
 **************************/

static void alpha_blend_colors(MT_COLOR *result, MT_COLOR *bg,
                               MT_COLOR *fg, int a)
{
   int alpha = a, inv_alpha;

   if (alpha > 255) alpha = 255;
   if (alpha < 0) alpha = 0;
   inv_alpha = 255 - alpha;

   result->r = fg->r * inv_alpha/255 + bg->r*alpha/255;
   result->g = fg->g * inv_alpha/255 + bg->g*alpha/255;
   result->b = fg->b * inv_alpha/255 + bg->b*alpha/255;
} /* alpha_blend_colors */

/************************************************************/

static void draw_bitmap(MT_WINDOW *win, MT_GC *gc,
                        int x, int y, int w, int h,
                        unsigned char *bits, int inv)
{
    int i,j,offset;
    unsigned char m;

    offset = 0;
    for (i=0; i<h; i++)
    {
        m = 0x01;
        for (j=0; j<w; j++)
        {
            if ((!inv && (bits[offset] & m)) || (inv && !(bits[offset] & m)))
            {
                mt_draw_point(win, gc, x+j, y+i);
            }
            m <<= 1;
            if (m == 0)
            {
                offset++;
                m = 0x01;
            }
        } /* for j */
        if (m != 0x01) offset++;
    } /* for i */
} /* draw_bitmap */
/************************************************************/


static void draw_radio(MT_WINDOW *win, int x, int y,
                       int state, MT_COLOR *col_bg)
{
    /* basing uppon plastik-code */
    MT_COLOR col_contour, col_content, col_tmp, col_checkmark, col_outside;
    MT_GC *gc;
    int alpha[3];
    int width = CHECKMARK_BOX_SIZE, height = CHECKMARK_BOX_SIZE;

    alpha_blend_colors (&col_contour,  col_bg, &DATA(col_black), 160);

    if (state & (MT_DISABLED | MT_MOUSE_ACTIVE))

        alpha_blend_colors (&col_content, &DATA(col_dark2), col_bg, 70);
    else
        col_content =  DATA(col_white);


    draw_box (win, NULL, &DATA(col_black), &col_content,
              x, y, width, height, 0, state);

    gc = mt_gc_new_with_foreground(col_bg);
    draw_bitmap(win, gc, x, y, RADIOMARK_BOX_SIZE, RADIOMARK_BOX_SIZE, radiobutton_mask_bits, 1);

    /* contour: */

    alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_contour, 50);
    mt_gc_set_foreground(gc, &col_tmp);
    draw_bitmap(win, gc, x, y, RADIOMARK_BOX_SIZE, RADIOMARK_BOX_SIZE,
                radiobutton_contour_bits, 0);

    /* anti-alias outside: */

    alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_contour, 150);
    mt_gc_set_foreground(gc, &col_tmp);
    draw_bitmap(win, gc, x, y, RADIOMARK_BOX_SIZE, RADIOMARK_BOX_SIZE,
                radiobutton_aa_outside_bits, 0);

    /* pressing the button: */

    if (state & ( MT_MOUSE_ACTIVE))
    {
        alpha_blend_colors(&col_tmp, &col_content, &DATA(col_grey), 100);
        mt_gc_set_foreground(gc, &col_content);
        draw_bitmap(win, gc, x, y, RADIOMARK_BOX_SIZE, RADIOMARK_BOX_SIZE, radiobutton_highlight1_bits, 0);

        alpha_blend_colors(&col_tmp, &col_content, &DATA(col_grey), 170);
        mt_gc_set_foreground(gc, &col_tmp);
        draw_bitmap(win, gc, x, y, RADIOMARK_BOX_SIZE, RADIOMARK_BOX_SIZE, radiobutton_highlight2_bits, 0);
    }

   /* anti-alias inside  */
   alpha_blend_colors(&col_tmp, &col_content, &col_contour, 180);


   mt_gc_set_foreground(gc, &col_tmp);
   draw_bitmap(win, gc, x, y, RADIOMARK_BOX_SIZE, RADIOMARK_BOX_SIZE, radiobutton_aa_inside_bits, 0);

   /* radiomark: */

   mt_color_light (&col_tmp, &DATA(col_black), 180);

   col_content = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_white);
   col_checkmark = (state & MT_DISABLED)? DATA(col_grey) : col_tmp;

   if (state & MT_ACTIVE)
   {

       alpha[0] = 80;
       alpha[1] = 140;
       alpha[2] = 160;


       x += (RADIOMARK_BOX_SIZE-RADIOMARK_SIZE+1)/2;
       y += (RADIOMARK_BOX_SIZE-RADIOMARK_SIZE+1)/2;


       mt_color_dark(&col_tmp, &col_checkmark, 150);
       alpha_blend_colors(&col_tmp, &col_content, &col_tmp, alpha[0]);
       mt_gc_set_foreground(gc, &col_tmp);
       draw_bitmap(win, gc, x, y, RADIOMARK_SIZE, RADIOMARK_SIZE,
                   radiomark_dark_bits, 0);

       mt_color_dark(&col_tmp, &col_checkmark, 125);
       alpha_blend_colors(&col_tmp, &col_content, &col_tmp, alpha[1]);
       mt_gc_set_foreground(gc, &col_tmp);
       draw_bitmap(win, gc, x, y, RADIOMARK_SIZE, RADIOMARK_SIZE,
                   radiomark_light_bits, 0);

       mt_color_dark(&col_tmp, &col_checkmark, 150);
       alpha_blend_colors(&col_tmp, &col_content, &col_tmp, alpha[2]);
       mt_gc_set_foreground(gc, &col_tmp);
       draw_bitmap(win, gc, x, y, RADIOMARK_SIZE, RADIOMARK_SIZE,
                   radiomark_aa_bits, 0);
   }

   mt_gc_destroy(gc);
} /* draw_radio */

/************************************************************/

/* also basing uppon plastik-code */
static void render_tab(MT_WINDOW *win, int x, int y, int width, int height,
                       int mouseOver, int selected, int bottom,
                       int pos, int triangular)
/* default values: 0, 0, 0, Middle, 0 */
{
   int reverseLayout = 0;
   int isFirst = (pos == First) || (pos == Single);
   int isLast = (pos == Last);
   int isSingle = (pos == Single);
   MT_RECTANGLE Rc, Rs, Rb;
   MT_COLOR col_tmp, col_tmp2;
   MT_GC *gc;
   int flags, surfaceFlags;
   int x2 = x+width-1;
   int y2 = y+height-1;

   /* TODO: */
   mouseOver = 0;

   gc = mt_gc_new_with_foreground(&DATA(col_grey));

   /* MetaTheme specific: */

   if (selected)
   {
       if (!bottom)
       {
           mt_draw_rectangle(win, gc, 1, x, y2-2, x2, y2);
           mt_rectangle_set(Rc, x, y, width, height-1);
       }
       else
       {
           mt_draw_rectangle(win, gc, 1, x, y, x2, y+2);
           mt_rectangle_set(Rc, x, y+1, width, height-1);
       }


       /* the resulting surface: */
       mt_rectangle_set(Rs, Rc.x+1, bottom? Rc.y:Rc.y+1, Rc.width-2, Rc.height-1);

       /* the area where the fake border shoudl appear: */
       mt_rectangle_set(Rb, x, bottom? y:(Rc.y+Rc.height-1)+1, width, height-Rc.height);

       flags = BORDER_LEFT | BORDER_RIGHT | BORDER_ROUND ;
       if (!bottom)
       {
           flags |= BORDER_TOP;
       }
       else
       {
           flags |= BORDER_BOTTOM;
       }

       draw_box (win, NULL, &DATA(col_black), &DATA(col_grey),
                 Rc.x, Rc.y, Rc.width, Rc.height, flags, 0);

      if (isFirst)
      {
          alpha_blend_colors(&col_tmp, &DATA(col_grey), &DATA(col_black), 160);
          mt_gc_set_foreground(gc, &col_tmp);

          if (!bottom)
          {
              mt_draw_point (win, gc, x, y+height-1);
          } /* !bottom */
          else
          {
              mt_draw_point (win, gc, x, y); /* TODO: find out if this is the right place */
          }
      } /* isFirst */

   } /* if selected */
   else
   {
       /* inactive tabs: */
       MT_COLOR col_bg;

       /* the top part of the tab which is nearly the same for all positions: */
       if (isFirst&&reverseLayout)
       {
           mt_rectangle_set(Rc, x+1, (bottom? y+2:(triangular? y+2:y+3)), width-2, (triangular? height-4:height-5));
       }
       else
       {
           mt_rectangle_set(Rc, x, (bottom? y+2:(triangular? y+2:y+3)), width, (triangular? height-4:height-5));
       }

       if ((isFirst && !reverseLayout) || (isLast && reverseLayout)) {
           mt_rectangle_set(Rs, Rc.x+1, bottom? Rc.y:Rc.y+1, Rc.width-2, Rc.height-1);
       }
       else
       {
           mt_rectangle_set(Rs, Rc.x, bottom? Rc.y:Rc.y+1, Rc.width-1, Rc.height-1);
       }

       /* the area where the fake border shoudl appear: */
       mt_rectangle_set(Rb, x, bottom? y:(Rc.y+Rc.height-1)+1, width, 2);

       if (!bottom)
       {
           if ((isFirst && !reverseLayout) || (isLast && reverseLayout))
           {
               flags =  BORDER_HIGHLIGHT_TOP | BORDER_HIGHLIGHT_LEFT | BORDER_LEFT |
                   BORDER_RIGHT | BORDER_TOP | BORDER_ROUND;
           }
           else if ((isLast && !reverseLayout) || (isFirst && reverseLayout))
           {
               flags =  BORDER_HIGHLIGHT_TOP | BORDER_RIGHT | BORDER_TOP| BORDER_ROUND;
           }
           else
           {
               flags =  BORDER_HIGHLIGHT_TOP | BORDER_RIGHT | BORDER_TOP| BORDER_ROUND;
           }
       }
       else
       { /* bottom */
         if ((isFirst && !reverseLayout) || (isLast && reverseLayout))
         {
             flags =  BORDER_HIGHLIGHT_TOP | BORDER_HIGHLIGHT_LEFT | BORDER_LEFT |
                 BORDER_RIGHT | BORDER_BOTTOM | BORDER_ROUND;
         }
         else if ((isLast && !reverseLayout) || (isFirst && reverseLayout))
         {
             flags = BORDER_HIGHLIGHT_TOP | BORDER_RIGHT | BORDER_BOTTOM | BORDER_ROUND;
         }
         else
         {
             flags =  BORDER_HIGHLIGHT_TOP | BORDER_RIGHT | BORDER_BOTTOM;
         }
       } /* if !bottom .. else */


       mt_color_dark (&col_bg, &DATA(col_grey), 113);

       draw_box (win, NULL, &DATA(col_black), &col_bg,
                Rc.x, Rc.y, Rc.width, Rc.height, flags, 0);

      /* some "position specific" paintings... */

      /* bottom or top side of the tab */
       alpha_blend_colors(&col_tmp, &DATA(col_grey), &DATA(col_black), 160);
       mt_gc_set_foreground(gc, &col_tmp);

       if(!bottom)
       {
           mt_draw_line(win, gc, Rb.x, Rb.y, (isLast && !reverseLayout)? (Rb.x+Rb.width-1):(Rb.x+Rb.width-1)-1, Rb.y);
       }

       else
       {
           mt_draw_line(win, gc, Rb.x, Rb.y+Rb.height-1,
                        (isLast && !reverseLayout)? (Rb.x+Rb.width-1):(Rb.x+Rb.width-1)-1, Rb.y+Rb.height-1);
       }

       /* connect the left side of the tab with the left neighbor */
       if ( !((isFirst && !reverseLayout) || (isLast && reverseLayout)) )
       {

           alpha_blend_colors(&col_tmp, &DATA(col_grey), &DATA(col_black), 142);
           mt_gc_set_foreground(gc, &col_tmp);

           mt_draw_point (win, gc, Rc.x+1, (bottom) ? Rc.y + Rc.height -1 : Rc.y );

           mt_color_dark (&col_tmp, &DATA(col_grey), 105);
           mt_gc_set_foreground(gc, &col_tmp);

           mt_draw_point (win, gc, Rc.x+1, (bottom) ? Rc.y + Rc.height : Rc.y + 1 );

       } /* of if */



   } /* of if selected */

   mt_gc_destroy(gc);
} /* render_tab */

/************************************************************/
