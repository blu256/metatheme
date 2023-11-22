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

#ifndef HIGHCOLOR_H
#define HIGHCOLOR_H

#include "metathemeqt.h"


class HighColorEngine: public ThemeEngine
{
public:
   HighColorEngine(MT_ENGINE *engine);
   virtual ~HighColorEngine();

   virtual void realize();
   virtual void drawWidget(const Window &win, const Rect &area, int type, int state, const Rect &rect, MT_WIDGET_DATA *data);
   virtual void drawString(const Window &win, int type, int state, MT_STRING *str);

private:
   void kDrawBeButton(QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool sunken, const QBrush *fill);
   void qDrawShadePanel(QPainter *p, int x, int y, int w, int h, const QColorGroup &g, bool sunken, int lineWidth, const QBrush *fill);
   void renderGradient(QPainter* p, const QRect& r, QColor clr, bool horizontal, int px=0, int py=0, int pwidth=-1, int pheight=-1);
   void drawBevel(QPainter* p, const Rect& r, const QColorGroup& cg, int flags);
   void drawBitmap(QPainter *p, int x, int y, int w, int h, const unsigned char *bits, bool inv=false);

private:
   enum StyleType { HighColor = 0, Default, B3 };

   StyleType styleType;
   Color darkColor, midColor, whiteColor, blackColor, greyColor, selectedColor, yellowColor, tooltipColor;
};

#endif /* HIGHCOLOR_H */
