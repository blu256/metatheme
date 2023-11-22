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
 * Based on code from HighColor style from KDE and some QT drawing functions.
 *
 * KDE3 HighColor Style (version 1.0)
 * Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
 *           (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org> 
 *
 * Drawing routines adapted from the KDE2 HCStyle,
 * Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *           (C) 2000 Dirk Mueller          <mueller@kde.org>
 *           (C) 2001 Martijn Klingens      <klingens@kde.org>
 *
 * Qt GUI Toolkit
 * Copyright (C) 1999-2003 Trolltech AS.  All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include "highcolor.h"
#include "bitmaps.h"

#define PE_ArrowUp MT_ARROW_UP
#define PE_ArrowDown MT_ARROW_DOWN
#define PE_ArrowLeft MT_ARROW_LEFT
#define PE_ArrowRight MT_ARROW_RIGHT


THEME_ENGINE(HighColorEngine)


HighColorEngine::HighColorEngine(MT_ENGINE *engine): ThemeEngine(engine)
{
   /* palette and colors stuff: */

   blackColor.set(0, 0, 0);
   whiteColor.set(255, 255, 255);
   yellowColor.set(245, 219, 149);
   tooltipColor.set(255, 255, 225);

   greyColor = palette(MT_BACKGROUND);
   selectedColor = palette(MT_SELECTED_BACKGROUND);

   Color button = palette(MT_BUTTON_BACKGROUND);
   Color midlight(
      (whiteColor.r() + button.r()) / 2,
      (whiteColor.g() + button.g()) / 2,
      (whiteColor.b() + button.b()) / 2
   );

   darkColor = greyColor.dark(280);
   midColor = greyColor.dark(120);

   setPalette(MT_BUTTON_LIGHT,    whiteColor);
   setPalette(MT_BUTTON_MIDLIGHT, midlight);
   setPalette(MT_BUTTON_DARK,     darkColor);
   setPalette(MT_BUTTON_MID,      midColor);
   setPalette(MT_BUTTON_SHADOW,   blackColor);

   /* size metrics: */
   
   setMetric(MT_BUTTON_BORDER,                  3, 3);
   setMetric(MT_BUTTON_TEXT_OFFSET,             1, 1);
   setMetric(MT_BUTTON_TOGGLE_BORDER,           3, 3);
   setMetric(MT_BUTTON_TOGGLE_TEXT_OFFSET,      1, 1);
   setMetric(MT_TOOLBAR_ITEM_BORDER,            3, 3);
   setMetric(MT_TOOLBAR_ITEM_TEXT_OFFSET,       1, 1);
   setMetric(MT_MENUBAR_BORDER,                 3, 3); /* TODO */
   setMetric(MT_MENUBAR_ITEM_BORDER,            2, 2);
   setMetric(MT_MENUBAR_ITEM_TEXT_OFFSET,       0, 0);
   setMetric(MT_MENU_BORDER,                    2, 2);
   setMetric(MT_MENU_ITEM_BORDER,               2, 2);
   setMetric(MT_PROGRESSBAR_BORDER,             2, 2);

   /* metrics: */

   setMetric(MT_BORDER,                         2);
   setMetric(MT_SCROLLBAR_SIZE,                 16);
   setMetric(MT_CHECKBOX_SIZE,                  13);
   setMetric(MT_SCALE_SLIDER_SIZE,              19);
   setMetric(MT_STATICBOX_TEXT_OFFSET,          5); /* TODO */
   setMetric(MT_MENUBAR_ITEM_SPACING,           3);
   setMetric(MT_MENU_SEPARATOR_HEIGHT,          2);
   setMetric(MT_NOTEBOOK_OVERLAP,               2);
   setMetric(MT_NOTEBOOK_TAB_OVERLAP,           0);
   setMetric(MT_NOTEBOOK_TEXT_OFFSET,           1);
   setMetric(MT_NOTEBOOK_ARROW_WIDTH,           16);
   setMetric(MT_NOTEBOOK_IS_FILLED,             0);
   setMetric(MT_CHOICE_BUTTON_WIDTH,            16+2);
   setMetric(MT_TOOLBAR_ITEM_DROPDOWN_WIDTH,    7);
   setMetric(MT_SPLITTER_WIDTH,                 3);
   setMetric(MT_MENU_TEAROFF_HEIGHT,            5);
   setMetric(MT_DETACHED_MENU_TEAROFF_HEIGHT,   12);

   Config cfg = getConfig();
   const char *tmp = cfg.get("b3", "settings");
   styleType = (tmp && !strcmp(tmp, "1"))? B3 : HighColor;
}


HighColorEngine::~HighColorEngine()
{
}


void HighColorEngine::realize()
{
}


void HighColorEngine::drawWidget(const Window &win, const Rect &area, int widgetType, int state, const Rect &rect, MT_WIDGET_DATA *data)
{
   bool highcolor = true;
   GC gc;
   QPainter *p = &gc;
   QColorGroup cg(this);
   int flags = 0;
   int type = styleType;
   Rect r(rect);

   if (data && data->orientation == MT_HORIZONTAL) flags |= Style_Horizontal;
   if (state & MT_ACTIVE) flags |= Style_Down | Style_On;
   if (state & MT_HOVER) flags |= Style_MouseOver;
   if (!(state & MT_DISABLED)) flags |= Style_Enabled;
   flags |= Style_Raised;

   bool down   = flags & Style_Down;
   bool on     = flags & Style_On;

   gc.begin(win);

   switch (widgetType) {
      case MT_BORDER_IN:
      case MT_BORDER_OUT:
      case MT_SCROLLED_WINDOW:
      case MT_ENTRY_BORDER:
      case MT_MENU:
      case MT_PROGRESSBAR:
      case MT_NOTEBOOK:
      {
         bool sunken = (widgetType == MT_BORDER_OUT || widgetType == MT_MENU || widgetType == MT_NOTEBOOK)? false : true;
         //QPen oldPen = p->pen();
         int x,y,w,h;
         r.rect(&x, &y, &w, &h);
         int x2 = x+w-1;
         int y2 = y+h-1;
         p->setPen(sunken ? cg.light() : cg.dark());
         p->drawLine(x, y2, x2, y2);
         p->drawLine(x2, y, x2, y2);
         p->setPen(sunken ? cg.mid() : cg.light());
         p->drawLine(x, y, x2, y);
         p->drawLine(x, y, x, y2);
         p->setPen(sunken ? cg.midlight() : cg.mid());
         p->drawLine(x+1, y2-1, x2-1, y2-1);
         p->drawLine(x2-1, y+1, x2-1, y2-1);
         p->setPen(sunken ? cg.dark() : cg.midlight());
         p->drawLine(x+1, y+1, x2-1, y+1);
         p->drawLine(x+1, y+1, x+1, y2-1);
         //p->setPen(oldPen);

         if (widgetType == MT_PROGRESSBAR) {
            gc.setColor(greyColor);
            gc.drawRectangle(true, x+2, y+2, x2-2, y2-2);
         }
         break;
      }

      case MT_HLINE:
      {
         gc.setColor(midColor);
         gc.drawLine(r.x(), r.y(), r.right(), r.y());
         gc.setColor(whiteColor);
         gc.drawLine(r.x(), r.y()+1, r.right(), r.y()+1);
         break;
      }

      case MT_VLINE:
      {
         gc.setColor(midColor);
         gc.drawLine(r.x(), r.y(), r.x(), r.bottom());
         gc.setColor(whiteColor);
         gc.drawLine(r.x()+1, r.y(), r.x()+1, r.bottom());
         break;
      }

      case MT_ARROW_UP:
      case MT_ARROW_DOWN:
      case MT_ARROW_LEFT:
      case MT_ARROW_RIGHT:
      {
         QPointArray a;
         
         if ( type != B3 ) {
            // HighColor & Default arrows
            switch(widgetType) {
               case PE_ArrowUp:
                  a.setPoints(QCOORDARRLEN(u_arrow), u_arrow);
                  break;

               case PE_ArrowDown:
                  a.setPoints(QCOORDARRLEN(d_arrow), d_arrow);
                  break;

               case PE_ArrowLeft:
                  a.setPoints(QCOORDARRLEN(l_arrow), l_arrow);
                  break;

               default:
                  a.setPoints(QCOORDARRLEN(r_arrow), r_arrow);
            }
         } else {
            // B3 arrows
            switch(widgetType) {
               case PE_ArrowUp:
                  a.setPoints(QCOORDARRLEN(B3::u_arrow), B3::u_arrow);
                  break;

               case PE_ArrowDown:
                  a.setPoints(QCOORDARRLEN(B3::d_arrow), B3::d_arrow);
                  break;

               case PE_ArrowLeft:
                  a.setPoints(QCOORDARRLEN(B3::l_arrow), B3::l_arrow);
                  break;

               default:
                  a.setPoints(QCOORDARRLEN(B3::r_arrow), B3::r_arrow);
            }
         }
            
         //p->save();
         /*
         if ( flags & Style_Down )
            p->translate( pixelMetric( PM_ButtonShiftHorizontal ),
                       pixelMetric( PM_ButtonShiftVertical ) );
         */
         if ( flags & Style_Down )
            r.translate(engine->metric_size[MT_BUTTON_TEXT_OFFSET].x, engine->metric_size[MT_BUTTON_TEXT_OFFSET].y);

         if ( flags & Style_Enabled ) {
            a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
            p->setPen( cg.buttonText() );
            p->drawLineSegments( a );
         } else {
            a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
            p->setPen( cg.light() );
            p->drawLineSegments( a );
            a.translate( -1, -1 );
            p->setPen( cg.mid() );
            p->drawLineSegments( a );
         }
         //p->restore();

         break;
      }

      case MT_SCROLLBAR:
      {
         int x, y, w, h;
         r.rect(&x, &y, &w, &h);
         int x2 = x+w-1;
         int y2 = y+h-1;

         if ( type != B3 ) {
            // HighColor & Default scrollbar
            
            p->setPen(cg.shadow());
            
            if (flags & Style_Horizontal) {
               p->drawLine(x, y, x2, y);
               p->drawLine(x, y2, x2, y2);
               renderGradient(p, QRect(x, y+1, w, h-2),
                        cg.mid(), false);
            } else {
               p->drawLine(x, y, x, y2);
               p->drawLine(x2, y, x2, y2);
               renderGradient(p, QRect(x+1, y, w-2, h),
                        cg.mid(), true);
            }   
         } else {
            // B3 scrollbar
            
            p->setPen( cg.mid() );
            
            if (flags & Style_Horizontal) {
               p->drawLine(x, y, x2, y);
               p->drawLine(x, y2, x2, y2);
               p->fillRect( QRect(x, y+1, w, h-2), 
                     flags & Style_Down ? cg.button() : cg.midlight() );
            } else {
               p->drawLine(x, y, x, y2);
               p->drawLine(x2, y, x2, y2);
               p->fillRect( QRect(x+1, y, w-2, h), 
                     flags & Style_Down ? cg.button() : cg.midlight() );
            }
         }
         break;
      }

      case MT_SCROLLBAR_HANDLE:
      {
         drawBevel(p, r, cg, flags);

         flags ^= Style_Horizontal;

         // Draw a scrollbar riffle (note direction after above changes)
         if ( type != B3 ) {
            
            // HighColor & Default scrollbar
            if (flags & Style_Horizontal) {
               if (r.height() >= 15) {
                  int x = r.x()+3;
                  int y = r.y() + (r.height()-7)/2;
                  int x2 = r.right()-3;
                  p->setPen(cg.light());
                  p->drawLine(x, y, x2, y);
                  p->drawLine(x, y+3, x2, y+3);
                  p->drawLine(x, y+6, x2, y+6);

                  p->setPen(cg.mid());
                  p->drawLine(x, y+1, x2, y+1);
                  p->drawLine(x, y+4, x2, y+4);
                  p->drawLine(x, y+7, x2, y+7);
               }
            } else {
               if (r.width() >= 15) {
                  int y = r.y()+3;
                  int x = r.x() + (r.width()-7)/2;
                  int y2 = r.bottom()-3;
                  p->setPen(cg.light());
                  p->drawLine(x, y, x, y2);
                  p->drawLine(x+3, y, x+3, y2);
                  p->drawLine(x+6, y, x+6, y2);

                  p->setPen(cg.mid());
                  p->drawLine(x+1, y, x+1, y2);
                  p->drawLine(x+4, y, x+4, y2);
                  p->drawLine(x+7, y, x+7, y2);
               }
            }
         } else {
            
            // B3 scrollbar
            if (flags & Style_Horizontal) {
               int buttons = 0;
               
               if (r.height() >= 36) buttons = 3;
               else if (r.height() >=24) buttons = 2;
               else if (r.height() >=16) buttons = 1;
               
               int x = r.x() + (r.width()-7) / 2;
               int y = r.y() + (r.height() - (buttons * 5) -
                     (buttons-1)) / 2;
               int x2 = x + 7;
               
               for ( int i=0; i<buttons; i++, y+=6 )
               {
                  p->setPen( cg.mid() );
                  p->drawLine( x+1, y, x2-1, y );
                  p->drawLine( x, y+1, x, y+3 );
                  p->setPen( cg.light() );
                  p->drawLine( x+1, y+1, x2-1, y+1 );
                  p->drawLine( x+1, y+1, x+1, y+3 );
                  p->setPen( cg.dark() );
                  p->drawLine( x+1, y+4, x2-1, y+4 );
                  p->drawLine( x2, y+1, x2, y+3 );
               }
            } else {
               int buttons = 0;
               
               if (r.width() >= 36) buttons = 3;
               else if (r.width() >=24) buttons = 2;
               else if (r.width() >=16) buttons = 1;
               
               int x = r.x() + (r.width() - (buttons * 5) -
                     (buttons-1)) / 2;
               int y = r.y() + (r.height()-7) / 2;
               int y2 = y + 7;
               
               for ( int i=0; i<buttons; i++, x+=6 )
               {
                  p->setPen( cg.mid() );
                  p->drawLine( x+1, y, x+3, y );
                  p->drawLine( x, y+1, x, y2-1 );
                  p->setPen( cg.light() );
                  p->drawLine( x+1, y+1, x+3, y+1 );
                  p->drawLine( x+1, y+1, x+1, y2-1 );
                  p->setPen( cg.dark() );
                  p->drawLine( x+1, y2, x+3, y2 );
                  p->drawLine( x+4, y+1, x+4, y2-1 );
               }
            }
         }
         break;
      }

      case MT_SCROLLBAR_ARROW_UP:
      case MT_SCROLLBAR_ARROW_DOWN:
      case MT_SCROLLBAR_ARROW_LEFT:
      case MT_SCROLLBAR_ARROW_RIGHT:
      {
         drawBevel(p, r, cg, flags);
         drawWidget(win, area, widgetType - MT_SCROLLBAR_ARROW_UP + MT_ARROW_UP, state, r, NULL);
         break;
      }

      case MT_BUTTON:
      case MT_BUTTON_DEFAULT:
      case MT_BUTTON_TOGGLE:
      {
         bool sunken = on || down;
         bool flat = !(flags & (Style_Raised | Style_Sunken));
         int  x, y, w, h;
         r.rect(&x, &y, &w, &h);
         int x2 = x+w-1;
         int y2 = y+h-1;

         gc.setColor(greyColor);
         gc.drawPoint(x, y);
         gc.drawPoint(x2, y);
         gc.drawPoint(x, y2);
         gc.drawPoint(x2, y2);

         if ( sunken )
            kDrawBeButton( p, x, y, w, h, cg, true,
                  &cg.brush(QColorGroup::Mid) );

         else if ( flags & Style_MouseOver && !flat ) {
            QBrush brush(cg.button().light(110));
            kDrawBeButton( p, x, y, w, h, cg, false, &brush );
         }

         // "Flat" button
         else if ( flat ) {
            if ( flags & Style_MouseOver )
               p->fillRect(r, cg.button().light(110));
            else
               renderGradient(p, QRect(x, y, w-1, h-1),
                           cg.button(), false);

            p->setPen(cg.button().light(75));
            p->drawLine(x, y, x2, y);
            p->drawLine(x, y, x, y2);
            p->drawLine(x, y2, x2, y2);
            p->drawLine(x2, y, x2, y2);
         }      

         else if( highcolor )
         {
            int x2 = x+w-1;
            int y2 = y+h-1;
            p->setPen(cg.shadow());
            p->drawLine(x+1, y, x2-1, y);
            p->drawLine(x+1, y2, x2-1, y2);
            p->drawLine(x, y+1, x, y2-1);
            p->drawLine(x2, y+1, x2, y2-1);

            p->setPen(cg.light());
            p->drawLine(x+2, y+2, x2-1, y+2);
            p->drawLine(x+2, y+3, x2-2, y+3);
            p->drawLine(x+2, y+4, x+2, y2-1);
            p->drawLine(x+3, y+4, x+3, y2-2);

            p->setPen(cg.mid());
            p->drawLine(x2-1, y+2, x2-1, y2-1);
            p->drawLine(x+2, y2-1, x2-1, y2-1);

            p->drawLine(x+1, y+1, x2-1, y+1);
            p->drawLine(x+1, y+2, x+1, y2-1);
            p->drawLine(x2-2, y+3, x2-2, y2-2);

            renderGradient(p, QRect(x+4, y+4, w-6, h-6),
                        cg.button(), false);
         } else
            kDrawBeButton(p, x, y, w, h, cg, false,
                       &cg.brush(QColorGroup::Button));

         if (!sunken) {
            gc.setColor(cg.light());
            gc.drawPoint(r.right() - 2, r.top() + 3);
         }
         break;
      }

      case MT_BUTTON_HEADER:
      {
         bool horizontal = true;
         int x,y,w,h;
         r.rect(&x, &y, &w, &h);
         bool sunken = on || down;
         int x2 = x+w-1;
         int y2 = y+h-1;
         //QPen oldPen = p->pen();

         // Bevel
         p->setPen(sunken ? cg.mid() : cg.light());
         p->drawLine(x, y, x2-1, y);
         p->drawLine(x, y, x, y2-1);
         p->setPen(sunken ? cg.light() : cg.mid());
         p->drawLine(x+1, y2-1, x2-1, y2-1);
         p->drawLine(x2-1, y+1, x2-1, y2-1);
         p->setPen(cg.shadow());
         p->drawLine(x, y2, x2, y2);
         p->drawLine(x2, y, x2, y2);

         if (sunken)
            p->fillRect(x+1, y+1, w-3, h-3, cg.button());
         else
            renderGradient( p, QRect(x+1, y+1, w-3, h-3),
                         cg.button(), !horizontal );
         //p->setPen( oldPen );
         break;
      }

      case MT_MENU_ITEM:
      case MT_MENUBAR_ITEM:
      {
         bool active = state & MT_SELECTED;

         if (widgetType == MT_MENUBAR_ITEM) {
            active = state & (MT_ACTIVE | MT_HOVER);
         }

         if ( active )
            qDrawShadePanel( p, r.x(), r.y(), r.width(), r.height(), cg, true, 1,
                             &cg.brush(QColorGroup::Midlight) );
         // Draw a solid background
         else
            p->fillRect( r, cg.button() );

         break;
      }
      
      case MT_MENU_ITEM_ARROW:
      {
         r.setX(r.x() - 2);
         drawWidget(win, area, MT_ARROW_RIGHT, state, r, data);
         break;
      }
      
      case MT_MENU_ITEM_SEPARATOR:
      {
         int  x, y, w, h;
         r.rect(&x, &y, &w, &h);

         p->setPen( cg.dark() );
         p->drawLine( x, y, x+w, y );
         p->setPen( cg.light() );
         p->drawLine( x, y+1, x+w, y+1 );
         break;
      }

      case MT_MENU_TEAROFF:
      {
         int i, max, x, y, width, height;
         r.rect(&x, &y, &width, &height);

         drawWidget(win, area, MT_MENU_ITEM, state, rect, data);

         x += 2;
         width -= 4;

         if (data->flags & MT_MENU_DETACHED) {
            drawWidget(win, area, MT_ARROW_LEFT, state, Rect(x-2, y+1, height, height-2), data);
            y += 5;
            i = 10;
         }
         else {
            y += 2;
            i = 0;
         }

         while (i < width) {
            max = i+5;
            if (max >= width) max = width;
            
            gc.setColor(midColor);
            gc.drawLine(x+i, y, x+max, y);
            gc.setColor(whiteColor);
            gc.drawLine(x+i, y+1, x+max, y+1);

            i += 10;
         }
         break;
      }

      case MT_MENUBAR:
      case MT_TOOLBAR:
      {
         //p->fillRect(r, cg.dark());
         int x2 = r.x()+r.width()-1;
         int y2 = r.y()+r.height()-1;
         //int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth) : opt.lineWidth();
         int lw = 2;
         
         if (lw)
         {
            p->setPen(cg.light());
            p->drawLine(r.x(), r.y(), x2-1,  r.y());
            p->drawLine(r.x(), r.y(), r.x(), y2-1);
            p->setPen(cg.dark());
            p->drawLine(r.x(), y2, x2, y2);
            p->drawLine(x2, r.y(), x2, y2);

            // ### Qt should specify Style_Horizontal where appropriate
            renderGradient( p, QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2),
                        cg.button(), 
                        (r.width() < r.height()) && (widgetType != MT_MENUBAR) );
         }
         else
            renderGradient( p, r, cg.button(), 
                        (r.width() < r.height()) && (widgetType != MT_MENUBAR) );
         break;
      }

      case MT_CHECK_BOX:
      case MT_CHECK_CELL:
      {
         bool enabled  = flags & Style_Enabled;
         bool nochange = false /*flags & Style_NoChange*/;
         
         /*
         if (xBmp.isNull()) {
            xBmp = QBitmap(7, 7, x_bits, true);
            xBmp.setMask(xBmp);
         }
         */
         down = (state & MT_MOUSE_ACTIVE);

         int x,y,w,h;
         x=r.x(); y=r.y(); w=r.width(); h=r.height();
         int x2 = x+w-1;
         int y2 = y+h-1;

         if (widgetType == MT_CHECK_BOX && (data->flags & MT_DRAW_MASK)) {
            gc.setColor(blackColor);
            gc.drawRectangle(true, x, y, x2, y2);
            return;
         }

         p->setPen(cg.mid());
         p->drawLine(x, y, x2, y);
         p->drawLine(x, y, x, y2);

         p->setPen(cg.light());
         p->drawLine(x2, y+1, x2, y2);
         p->drawLine(x+1, y2, x2, y2);

         p->setPen(cg.shadow());
         p->drawLine(x+1, y+1, x2-1, y+1);
         p->drawLine(x+1, y+1, x+1, y2-1);

         p->setPen(cg.midlight());
         p->drawLine(x2-1, y+2, x2-1, y2-1);
         p->drawLine(x+2, y2-1, x2-1, y2-1);

         if ( enabled )
            p->fillRect(x+2, y+2, w-4, h-4, 
                  down ? cg.button(): cg.base());
         else
            p->fillRect(x+2, y+2, w-4, h-4, cg.background());
         
         //if (!(flags & Style_Off)) {
            if (on) {
               p->setPen(nochange ? cg.dark() : cg.text());
               //p->drawPixmap(x+3, y+3, xBmp);
               drawBitmap(p, x+3, y+3, 7, 7, x_bits);
            }
            /*
            else {
               p->setPen(cg.shadow());
               p->drawRect(x+2, y+2, w-4, h-4);
               p->setPen(nochange ? cg.text() : cg.dark());
               p->drawLine(x+3, (y+h)/2-2, x+w-4, (y+h)/2-2);
               p->drawLine(x+3, (y+h)/2, x+w-4, (y+h)/2);
               p->drawLine(x+3, (y+h)/2+2, x+w-4, (y+h)/2+2);
            }
            */
         //}

         break;
      }

      case MT_RADIO_BOX:
      case MT_RADIO_CELL:
      {
         if (widgetType == MT_RADIO_BOX && (data->flags & MT_DRAW_MASK)) {
            gc.setColor(blackColor);
            drawBitmap(p, r.x(), r.y(), 13, 13, radiomask_bits);
            return;
         }

         down = (state & MT_MOUSE_ACTIVE);

         /*
         if (lightBmp.isNull()) {
            lightBmp  = QBitmap(13, 13, radiooff_light_bits,  true);
            grayBmp   = QBitmap(13, 13, radiooff_gray_bits,   true);
            dgrayBmp  = QBitmap(13, 13, radiooff_dgray_bits,  true);
            centerBmp = QBitmap(13, 13, radiooff_center_bits, true);
            centerBmp.setMask( centerBmp );
         }

         // Bevel
         kColorBitmaps(p, cg, r.x(), r.y(), &lightBmp , &grayBmp,
                    NULL, &dgrayBmp);
         */
         p->setPen(cg.light());
         drawBitmap(p, r.x(), r.y(), 13, 13, radiooff_light_bits);
         p->setPen(cg.mid());
         drawBitmap(p, r.x(), r.y(), 13, 13, radiooff_gray_bits);
         p->setPen(cg.dark());
         drawBitmap(p, r.x(), r.y(), 13, 13, radiooff_dgray_bits);
         
         // The center fill of the indicator (grayed out when disabled)
         if ( flags & Style_Enabled )
            p->setPen( down ? cg.button() : cg.base() );
         else
            p->setPen( cg.background() );
         //p->drawPixmap( r.x(), r.y(), centerBmp );
         drawBitmap(p, r.x(), r.y(), 13, 13, radiooff_center_bits);

         // Indicator "dot"
         if ( on ) {
            int x=r.x(), y=r.y();
            QColor color = /*flags & Style_NoChange*/false ?
               cg.dark() : cg.text();
            
            p->setPen(color);
            p->drawLine(x+5, y+4, x+7, y+4);
            p->drawLine(x+4, y+5, x+4, y+7);
            p->drawLine(x+5, y+8, x+7, y+8);
            p->drawLine(x+8, y+5, x+8, y+7);
            p->fillRect(x+5, y+5, 3, 3, color);
         }

         break;
      }

      case MT_PROGRESSBAR_SLIDER:
      {
         gc.drawGradient(rect, selectedColor.dark(150), selectedColor.light(150), MT_HORIZONTAL);
         break;
      }

      case MT_SCALE:
      {
         bool horizontal = (data->orientation == MT_HORIZONTAL);
         int gcenter = (horizontal ? r.height() : r.width()) / 2;

         QRect gr;
         if (horizontal)
            gr = QRect(r.x(), r.y()+gcenter-3, r.width(), 7);
         else
            gr = QRect(r.x()+gcenter-3, r.y(), 7, r.height());

         int x,y,w,h;
         gr.rect(&x, &y, &w, &h);
         int x2=x+w-1;
         int y2=y+h-1;

         // Draw the slider groove.
         p->setPen(cg.dark());
         p->drawLine(x+2, y, x2-2, y);
         p->drawLine(x, y+2, x, y2-2);
         p->fillRect(x+2,y+2,w-4, h-4, 
            !(state & MT_DISABLED) ? cg.dark() : cg.mid());
         p->setPen(cg.shadow());
         p->drawRect(x+1, y+1, w-2, h-2);
         p->setPen(cg.light());
         p->drawPoint(x+1,y2-1);
         p->drawPoint(x2-1,y2-1);
         p->drawLine(x2, y+2, x2, y2-2);
         p->drawLine(x+2, y2, x2-2, y2);
         break;
      }

      case MT_SCALE_HANDLE:
      {
         bool horizontal = (data->orientation == MT_HORIZONTAL);
         int x,y,w,h;
         r.rect(&x, &y, &w, &h);
         int x2 = x+w-1;
         int y2 = y+h-1;
            
         p->setPen(cg.mid());
         p->drawLine(x+1, y, x2-1, y);
         p->drawLine(x, y+1, x, y2-1);
         p->setPen(cg.shadow());
         p->drawLine(x+1, y2, x2-1, y2);
         p->drawLine(x2, y+1, x2, y2-1);

         p->setPen(cg.light());
         p->drawLine(x+1, y+1, x2-1, y+1);
         p->drawLine(x+1, y+1, x+1,  y2-1);
         p->setPen(cg.dark());
         p->drawLine(x+2, y2-1, x2-1, y2-1);
         p->drawLine(x2-1, y+2, x2-1, y2-1);
         p->setPen(cg.midlight());
         p->drawLine(x+2, y+2, x2-2, y+2);
         p->drawLine(x+2, y+2, x+2, y2-2);
         p->setPen(cg.mid());
         p->drawLine(x+3, y2-2, x2-2, y2-2);
         p->drawLine(x2-2, y+3, x2-2, y2-2);
         renderGradient(p, QRect(x+3, y+3, w-6, h-6), 
                     cg.button(), !horizontal);

         // Paint riffles
         if (horizontal) {
            p->setPen(cg.light());
            p->drawLine(x+5, y+4, x+5, y2-4);
            p->drawLine(x+8, y+4, x+8, y2-4);
            p->drawLine(x+11,y+4, x+11, y2-4);
            p->setPen(!(state & MT_DISABLED) ? cg.shadow(): cg.mid());
            p->drawLine(x+6, y+4, x+6, y2-4);
            p->drawLine(x+9, y+4, x+9, y2-4);
            p->drawLine(x+12,y+4, x+12, y2-4);
         } else {
            p->setPen(cg.light());
            p->drawLine(x+4, y+5, x2-4, y+5);
            p->drawLine(x+4, y+8, x2-4, y+8);
            p->drawLine(x+4, y+11, x2-4, y+11);
            p->setPen(!(state & MT_DISABLED) ? cg.shadow() : cg.mid());
            p->drawLine(x+4, y+6, x2-4, y+6);
            p->drawLine(x+4, y+9, x2-4, y+9);
            p->drawLine(x+4, y+12, x2-4, y+12);
         }
         break;
      }

      case MT_TOOLTIP:
      {
         gc.setColor(blackColor);
         gc.drawRectangle(false, rect);
         break;
      }

      case MT_NOTEBOOK_TAB:
      {
         bool selected = (state & MT_SELECTED);
         /*
         if (!selected && data->gap_position == MT_POSITION_TOP) {
            p->setPen(cg.midlight());
            p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
            
            if (data->flags & MT_NOTEBOOK_FIRST_VISIBLE_TAB) {
               p->setPen(cg.light());
               p->drawPoint(r.x(), r.bottom());
            }

            r.setHeight(r.height() - 1);
         }
         */

         int x = r.x(), y=r.y(), bottom=r.bottom(), right=r.right();

         if (selected) {
            gc.setColor(greyColor);
            if (data->gap_position == MT_POSITION_TOP) {
               gc.drawRectangle(true, x, bottom-2, right, bottom);
            }
            else {
               gc.drawRectangle(true, x, y, right, y+2);
            }
         }

         switch (data->gap_position) {
            case MT_POSITION_TOP:
               if (!selected) {
                  y += 1;
                  r.setHeight(r.height() - 1);
               }

               p->setPen(selected ? cg.light() : cg.shadow());
               p->drawLine(x, y+4, x, bottom);
               p->drawLine(x, y+4, x+4, y);
               p->drawLine(x+4, y, right-1, y);
               if (selected)
                  p->setPen(cg.shadow());
               p->drawLine(right, y+1, right, bottom);

               p->setPen(cg.midlight());
               p->drawLine(x+1, y+4, x+1, bottom);
               p->drawLine(x+1, y+4, x+4, y+1);
               p->drawLine(x+5, y+1, right-2, y+1);

               if (selected) {
                  p->setPen(cg.mid());
                  p->drawLine(right-1, y+1, right-1, bottom);
               } else {
                  p->setPen(cg.mid());
                  p->drawPoint(right-1, y+1);
                  p->drawLine(x+4, y+2, right-1, y+2);
                  p->drawLine(x+3, y+3, right-1, y+3);
                  p->fillRect(x+2, y+4, r.width()-3, r.height()-5, cg.mid());

                  p->setPen(cg.light());
                  p->drawLine(x, bottom/*-1*/, right, bottom/*-1*/);
               }
               break;

            case MT_POSITION_BOTTOM:
               if (!selected) {
                  y += 1;
                  r.setHeight(r.height() + 1);
               }
               
               p->setPen(selected ? cg.light() : cg.shadow());
               p->drawLine(x, bottom-4, x, y);
               if (selected)
                  p->setPen(cg.mid());
               p->drawLine(x, bottom-4, x+4, bottom);
               if (selected)
                  p->setPen(cg.shadow());
               p->drawLine(x+4, bottom, right-1, bottom);
               p->drawLine(right, bottom-1, right, y);

               p->setPen(cg.midlight());
               p->drawLine(x+1, bottom-4, x+1, y);
               p->drawLine(x+1, bottom-4, x+4, bottom-1);
               p->setPen(cg.shadow());
               p->drawLine(x+5, bottom-1, right-2, bottom-1);

               if (selected) {
                  p->setPen(cg.mid());
                  p->drawLine(right-1, y, right-1, bottom-1);
               } else {
                  p->setPen(cg.mid());
                  p->drawPoint(right-1, bottom-1);
                  p->drawLine(x+4, bottom-2, right-1, bottom-2);
                  p->drawLine(x+3, bottom-3, right-1, bottom-3);
                  p->fillRect(x+2, y+2, r.width()-3, r.height()-6, cg.mid());

                  p->setPen(cg.dark());
                  p->drawLine(x, y, right, y);
               }
               break;
         }
         break;
      }

      case MT_NOTEBOOK_ARROW_LEFT:
      case MT_NOTEBOOK_ARROW_RIGHT:
      {
         state |= MT_HOVER;
         drawWidget(win, area, MT_TOOLBAR_ITEM, state, r, data);
         drawWidget(win, area, (widgetType == MT_NOTEBOOK_ARROW_LEFT)? MT_ARROW_LEFT : MT_ARROW_RIGHT, state, r, data);
         break;
      }

      case MT_TOOLBAR_ITEM:
      case MT_TOOLBAR_ITEM_TOGGLE:
      {
         if (!(state & (MT_HOVER | MT_ACTIVE))) break;

         bool sunken = on || down;
         int  x,y,w,h;
         r.rect(&x, &y, &w, &h);
         int x2 = x+w-1;
         int y2 = y+h-1;
         //QPen oldPen = p->pen();

         // Outer frame (round style)
         p->setPen(cg.shadow());
         p->drawLine(x+1,y,x2-1,y);
         p->drawLine(x,y+1,x,y2-1);
         p->drawLine(x+1,y2,x2-1,y2);
         p->drawLine(x2,y+1,x2,y2-1);

         // Bevel
         p->setPen(sunken ? cg.mid() : cg.light());
         p->drawLine(x+1, y+1, x2-1, y+1);
         p->drawLine(x+1, y+1, x+1, y2-1);
         p->setPen(sunken ? cg.light() : cg.mid());
         p->drawLine(x+2, y2-1, x2-1, y2-1);
         p->drawLine(x2-1, y+2, x2-1, y2-1);

         p->fillRect(x+2, y+2, w-4, h-4, cg.button());

         //p->setPen( oldPen );
         break;
      }
      
      case MT_TOOLBAR_ITEM_DROPDOWN:
      {
         /*
         gc.setColor(greyColor);
         gc.drawRectangle(true, r);
         */

         drawWidget(win, area, MT_TOOLBAR_ITEM, state, r, data);

         /*
         if (data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE) {
            state &= ~MT_ACTIVE;
            state |= MT_HOVER;
         }
         drawWidget(win, area, MT_TOOLBAR_ITEM, state, r, data);

         if ((state & MT_ACTIVE) || (data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE)) {
            Rect r2(r);
            r2.setX(r.x() + r.width() - 12);
            r2.setWidth(12);
            drawWidget(win, area, MT_TOOLBAR_ITEM, MT_ACTIVE, r2, NULL);
         }
         else if (state & (MT_HOVER)) {
            int x = r.right() - 11;
            gc.setColor(midColor);
            gc.drawLine(x, r.y()+2, x, r.bottom()-3);
         }
         */

         r.setX(r.right() - 8);
         r.setY(r.y() + r.height() / 2 - 1);
         //drawWidget(win, area, MT_ARROW_DOWN, state, r, NULL);
         
         if ((state & MT_ACTIVE) && !(data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE)) {
            r.translate(1, 1);
         }

         gc.setColor(blackColor);
         gc.drawLine(r.x(), r.y(), r.x()+4, r.y());
         gc.drawLine(r.x()+1, r.y()+1, r.x()+3, r.y()+1);
         gc.drawPoint(r.x()+2, r.y()+2);
         break;
      }

      case MT_TOOLBAR_HANDLE:
      {
         int x = r.x(); int y = r.y();
         int x2 = r.x() + r.width()-1;
         int y2 = r.y() + r.height()-1;

         if (flags & Style_Horizontal) {

            renderGradient( p, r, cg.button(), false);
            p->setPen(cg.light());
            p->drawLine(x+1, y+4, x+1, y2-4);
            p->drawLine(x+3, y+4, x+3, y2-4);
            p->drawLine(x+5, y+4, x+5, y2-4);

            p->setPen(cg.mid());
            p->drawLine(x+2, y+4, x+2, y2-4);
            p->drawLine(x+4, y+4, x+4, y2-4);
            p->drawLine(x+6, y+4, x+6, y2-4);

         } else {
            
            renderGradient( p, r, cg.button(), true);
            p->setPen(cg.light());
            p->drawLine(x+4, y+1, x2-4, y+1);
            p->drawLine(x+4, y+3, x2-4, y+3);
            p->drawLine(x+4, y+5, x2-4, y+5);

            p->setPen(cg.mid());
            p->drawLine(x+4, y+2, x2-4, y+2);
            p->drawLine(x+4, y+4, x2-4, y+4);
            p->drawLine(x+4, y+6, x2-4, y+6);

         }
         break;
      }

      case MT_STATICBOX:
      {
         gc.setColor(midColor);
         gc.drawRectangle(false, r.left(), r.top(), r.right()-1, r.bottom()-1);

         gc.setColor(whiteColor);
         gc.drawLine(r.right(), r.top(), r.right(), r.bottom());
         gc.drawLine(r.left(), r.bottom(), r.right(), r.bottom());
         gc.drawLine(r.left()+1, r.top()+1, r.left()+1, r.bottom()-2);
         gc.drawLine(r.left()+1, r.top()+1, r.right()-2, r.top()+1);

         if (data->gap_width > 0) {
            gc.setColor(greyColor);

            switch (data->gap_position) {
               case MT_POSITION_TOP:
                  gc.drawRectangle(true, r.x()+data->gap_x, r.y(), r.x()+data->gap_x+data->gap_width-1, r.y()+1);
                  break;
            }
         }

         break;
      }

      case MT_SPINBUTTON:
      {
         drawWidget(win, area, MT_ENTRY_BORDER, state, r, NULL);
         break;
      }

      case MT_SPINBUTTON_UP:
      case MT_SPINBUTTON_DOWN:
      {
         r.setLeft(r.left() + 1);

         state |= MT_HOVER;
         drawWidget(win, area, MT_TOOLBAR_ITEM, state, r, NULL);
         
         int x = r.x() + (r.width() - 5) / 2;
         int y = r.y() + (r.height() - 3) / 2;
         gc.setColor((state & MT_DISABLED)? midColor : blackColor);
         if (state & MT_ACTIVE) {
            x++;
            y++;
         }
         
         if (widgetType == MT_SPINBUTTON_UP) {
            gc.drawPoint(x+2, y+0);
            gc.drawLine(x+1, y+1, x+3, y+1);
            gc.drawLine(x+0, y+2, x+4, y+2);
         }
         else {
            gc.drawLine(x+0, y+0, x+4, y+0);
            gc.drawLine(x+1, y+1, x+3, y+1);
            gc.drawPoint(x+2, y+2);
         }

         r.setLeft(r.left() - 1);
         r.setWidth(1);
         gc.setColor(greyColor);
         gc.drawLine(r.x(), r.y(), r.x(), r.bottom());
         break;
      }

      case MT_CHOICE:
      case MT_CHOICE_BUTTON:
      {
         if (widgetType == MT_CHOICE) {
            r.setRight(r.right() + 18);
         }
         else {
            r.setLeft(r.left() - 4);
         }
         
         //bool sunken = (active == SC_ComboBoxArrow);
         bool sunken = false;

         // Draw the combo
         int x,y,w,h;
         r.rect(&x, &y, &w, &h);
         int x2 = x+w-1;
         int y2 = y+h-1;

         p->setPen(cg.shadow());
         p->drawLine(x+1, y, x2-1, y);
         p->drawLine(x+1, y2, x2-1, y2);
         p->drawLine(x, y+1, x, y2-1);
         p->drawLine(x2, y+1, x2, y2-1);

         // Ensure the edge notches are properly colored
         p->setPen(cg.button());
         p->drawPoint(x,y);
         p->drawPoint(x,y2);
         p->drawPoint(x2,y);
         p->drawPoint(x2,y2);

         renderGradient( p, QRect(x+2, y+2, w-4, h-4),
                     cg.button(), false);

         p->setPen(sunken ? cg.light() : cg.mid());
         p->drawLine(x2-1, y+2, x2-1, y2-1);
         p->drawLine(x+1, y2-1, x2-1, y2-1);

         p->setPen(sunken ? cg.mid() : cg.light());
         p->drawLine(x+1, y+1, x2-1, y+1);
         p->drawLine(x+1, y+2, x+1, y2-2);

         // Get the button bounding box
         /*QRect ar = QStyle::visualRect(
            querySubControlMetrics(CC_ComboBox, widget, SC_ComboBoxArrow),
            widget );*/
         QRect ar(r.right() - 18, r.y() + 2, 18, r.height() - 4);

         /*
         // Are we enabled?
         if ( widget->isEnabled() )
            flags |= Style_Enabled;

         // Are we "pushed" ?
         if ( active & Style_Sunken )
            flags |= Style_Sunken;
         */

         //drawPrimitive(PE_ArrowDown, p, ar, cg, flags);
         drawWidget(win, area, MT_ARROW_DOWN, state & ~MT_ACTIVE, ar, NULL);

         /*QRect re = QStyle::visualRect(
            querySubControlMetrics( CC_ComboBox, widget,
                                   SC_ComboBoxEditField), widget );*/
         QRect re(r.x() + 3, r.y() + 3, r.width() - 6 - 18, r.height() - 6);
         gc.setColor(whiteColor);
         gc.drawRectangle(true, re);

         // Draw the indent
         if (true /*cb->editable()*/) {
            p->setPen( cg.dark() );
            p->drawLine( re.x(), re.y()-1, re.x()+re.width(), re.y()-1 );
            p->drawLine( re.x()-1, re.y(), re.x()-1, re.y()+re.height() );
         }

         /*
         if ( cb->hasFocus() ) {
            p->setPen( cg.highlightedText() );
            p->setBackgroundColor( cg.highlight() );
         } else {
            p->setPen( cg.text() );
            p->setBackgroundColor( cg.button() );
         }

         if ( cb->hasFocus() && !cb->editable() ) {
            // Draw the contents
            p->fillRect( re.x(), re.y(), re.width(), re.height(),
                      cg.brush( QColorGroup::Highlight ) );

            //QRect re = QStyle::visualRect( 
            //         subRect(SR_ComboBoxFocusRect, cb), widget);

            //drawPrimitive( PE_FocusRect, p, re, cg,
            //            Style_FocusAtBorder, QStyleOption(cg.highlight()));
         }
         */
         break;
      }

   }

   gc.end();
}


void HighColorEngine::drawString(const Window &win, int type, int state, MT_STRING *str)
{
   bool drawBold = false;
   Color *col1, *col2;

   if (type == MT_MENU_ITEM && !(state & MT_DISABLED)) {
      win.drawString(str, 0, 0, &blackColor);
      return;
   }

   if ((type == MT_BUTTON || type == MT_BUTTON_DEFAULT || type == MT_BUTTON_TOGGLE) && (state & MT_ACTIVE)) {
      drawBold = true;
      col1 = &whiteColor;
      col2 = &blackColor;
   }
   else if (type == MT_BUTTON_DEFAULT && !(state & MT_DISABLED)) {
      drawBold = true;
      col1 = &blackColor;
      col2 = &midColor;
   }

   if (drawBold) {
      win.drawString(str, 2, 1, col2);
      win.drawString(str, 0, 0, col1);
      win.drawString(str, 1, 0, col1);
      return;
   }

   if (state & MT_DISABLED) {
      win.drawString(str, 1, 1, &whiteColor);
   }

   win.drawString(str, 0, 0, NULL);
}


void HighColorEngine::kDrawBeButton(QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool sunken, const QBrush *fill)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(g.dark());
    p->drawLine(x+1, y, x2-1, y);
    p->drawLine(x, y+1, x, y2-1);
    p->drawLine(x+1, y2, x2-1, y2);
    p->drawLine(x2, y+1, x2, y2-1);


    if(!sunken){
        p->setPen(g.light());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }
    else{
        p->setPen(g.mid());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }


    p->setPen(sunken? g.light() : g.mid());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+2, y2-1, x2-1, y2-1);

    p->setPen(g.mid());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+2, x+1, y2-1);
    p->drawLine(x2-2, y+3, x2-2, y2-2);

    if(fill)
        p->fillRect(x+4, y+4, w-6, h-6, *fill);
}


void HighColorEngine::qDrawShadePanel( QPainter *p, int x, int y, int w, int h,
            const QColorGroup &g, bool sunken,
            int lineWidth, const QBrush *fill )
{
    if ( w == 0 || h == 0 )
   return;
    if ( !( w > 0 && h > 0 && lineWidth >= 0 ) ) {
#if defined(QT_CHECK_RANGE)
       qWarning( "qDrawShadePanel() Invalid parameters." );
#endif
    }
    QColor shade = g.dark();
    QColor light = g.light();
    if ( fill ) {
   if ( fill->color() == shade )
       shade = g.shadow();
   if ( fill->color() == light )
       light = g.midlight();
    }
    QPen oldPen = p->pen();         // save pen
    QPointArray a( 4*lineWidth );
    if ( sunken )
   p->setPen( shade );
    else
   p->setPen( light );
    int x1, y1, x2, y2;
    int i;
    int n = 0;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for ( i=0; i<lineWidth; i++ ) {      // top shadow
   a.setPoint( n++, x1, y1++ );
   a.setPoint( n++, x2--, y2++ );
    }
    x2 = x1;
    y1 = y+h-2;
    for ( i=0; i<lineWidth; i++ ) {      // left shadow
   a.setPoint( n++, x1++, y1 );
   a.setPoint( n++, x2++, y2-- );
    }
    p->drawLineSegments( a );
    n = 0;
    if ( sunken )
   p->setPen( light );
    else
   p->setPen( shade );
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for ( i=0; i<lineWidth; i++ ) {      // bottom shadow
   a.setPoint( n++, x1++, y1-- );
   a.setPoint( n++, x2, y2-- );
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lineWidth-1;
    for ( i=0; i<lineWidth; i++ ) {      // right shadow
   a.setPoint( n++, x1--, y1++ );
   a.setPoint( n++, x2--, y2 );
    }
    p->drawLineSegments( a );
    if ( fill ) {            // fill with fill color
   QBrush oldBrush = p->brush();
   //p->setPen( Qt::NoPen );
   //p->setBrush( *fill );
   p->fillRect( x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2, *fill );
   p->setBrush( oldBrush );
    }
    p->setPen( oldPen );         // restore pen
}


// TODO
void HighColorEngine::renderGradient(QPainter* p, const QRect& r, QColor clr, bool horizontal, int px, int py, int pwidth, int pheight)
{
   if (styleType != HighColor) {
      p->fillRect(r, clr);
      return;
   }

   p->drawGradient(r, clr.light(110), clr, horizontal? MT_HORIZONTAL:MT_VERTICAL);
}


void HighColorEngine::drawBevel(QPainter* p, const Rect& r, const QColorGroup& cg, int flags)
{
   bool down = flags & Style_Down;
   bool on   = flags & Style_On;

   int x,y,w,h;
   r.rect(&x, &y, &w, &h);
   bool sunken = on || down;
   int x2 = x+w-1;
   int y2 = y+h-1;

   // Outer frame
   p->setPen(cg.shadow());
   p->drawRect(r);

   // Bevel
   p->setPen(sunken ? cg.mid() : cg.light());
   p->drawLine(x+1, y+1, x2-1, y+1);
   p->drawLine(x+1, y+1, x+1, y2-1);
   p->setPen(sunken ? cg.light() : cg.mid());
   p->drawLine(x+2, y2-1, x2-1, y2-1);
   p->drawLine(x2-1, y+2, x2-1, y2-1);

   if (w > 4 && h > 4) {
      if (sunken)
         p->fillRect(x+2, y+2, w-4, h-4, cg.button());
      else
         renderGradient( p, QRect(x+2, y+2, w-4, h-4),
                      cg.button(), (flags & Style_Horizontal)? MT_HORIZONTAL : MT_VERTICAL );
   }
}


void HighColorEngine::drawBitmap(QPainter *p, int x, int y, int w, int h, const unsigned char *bits, bool inv)
{
   int i,j,offset;
   unsigned char m;

   offset = 0;
   for (i=0; i<h; i++) {
      m = 0x01;
      for (j=0; j<w; j++) {
         if ((!inv && (bits[offset] & m)) || (inv && !(bits[offset] & m))) {
            p->drawPoint(x+j, y+i);
         }
         m <<= 1;
         if (m == 0) {
            offset++;
            m = 0x01;
         }
      }
      if (m != 0x01) offset++;
   }
}
