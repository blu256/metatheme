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

#ifndef METATHEMEQT_H
#define METATHEMEQT_H

namespace metathemeqt {

class QPointArray;

#define MT_EXT metathemeqt

#define MT_EXT_COLOR \
public: \
   const Color& color() const { return *((const Color *)this); } \
   bool operator== (const Color& b) const { return (col.r == b.col.r && col.g == b.col.g && col.b == b.col.b); }

#define MT_EXT_RECT \
public: \
   void rect(int *x, int *y, int *w, int *h) const;

#define MT_EXT_GC \
public: \
   void setPen(const Color &color); \
   void setBrush(const Color &color); \
   void fillRect(int x, int y, int width, int height, const Color &color); \
   void fillRect(const Rect &rect, const Color &color); \
   void drawRect(int x, int y, int width, int height); \
   void drawRect(const Rect &rect); \
   void drawLineSegments(const QPointArray& a); \
   Color pen() const { return Color((MT_COLOR *)&col); } \
   Color brush() const { return Color((MT_COLOR *)&col); }

#define MT_EXT_THEMEENGINE \
   friend class QColorGroup;

#include "metathemecpp.h"

/********************************/
/*** QT COMPATIBILITY CLASSES ***/
/********************************/

#define QPainter GC
#define QRect Rect
#define QPen Color
#define QBrush Color
#define QColor Color

#define QCOORD signed int

enum {
   Style_Horizontal = 0x0001,
   Style_Down       = 0x0002,
   Style_On         = 0x0004,
   Style_Raised     = 0x0008,
   Style_Sunken     = 0x0010,
   Style_MouseOver  = 0x0020,
   Style_Enabled    = 0x0040
};

class QColorGroup
{
public:
   enum ColorRole {
      Foreground, Button, Light, Midlight, Dark, Mid,
      Text, BrightText, ButtonText, Base, Background, Shadow,
      Highlight, HighlightedText, Link, LinkVisited,
      NColorRoles
   };

public:
   QColorGroup(ThemeEngine *engine);

   Color shadow() const;
   Color light() const;
   Color mid() const;
   Color midlight() const;
   Color dark() const;
   Color button() const;
   Color buttonText() const;
   Color base() const;
   Color text() const;
   Color background() const;
   Color highlight() const;
   Color highlightedText() const;

   const Color &brush(ColorRole role);

private:
   MT_ENGINE *engine;
   Color brushes[NColorRoles];
};

class QPointArray
{
public:
   QPointArray();
   QPointArray(int nPoints);
   QPointArray(int nPoints, const QCOORD* points);
   virtual ~QPointArray();

   void setPoint(int i, int x, int y);
   void setPoints(int nPoints, const QCOORD* points);
   void translate(int dx, int dy);

private:
   int np;
   MT_POINT *p;

   friend class GC;
};

/********************************/
/*** END OF INTERFACE SECTION ***/
/********************************/

inline void Rect::rect(int *x, int *y, int *w, int *h) const { *x=r.x; *y=r.y; *w=r.width; *h=r.height; }

inline void GC::setPen(const Color &color) { setColor(color); }
inline void GC::setBrush(const Color &color) { setColor(color); }
inline void GC::fillRect(int x, int y, int width, int height, const Color &color) { setColor(color); drawRectangle(true, x, y, x+width-1, y+height-1); }
inline void GC::fillRect(const Rect &rect, const Color &color) { setColor(color); drawRectangle(true, rect.x(), rect.y(), rect.right(), rect.bottom()); }
inline void GC::drawRect(int x, int y, int width, int height) { drawRectangle(false, x, y, x+width-1, y+height-1); }
inline void GC::drawRect(const Rect &rect) { drawRectangle(false, rect.x(), rect.y(), rect.right(), rect.bottom()); }
inline void GC::drawLineSegments(const QPointArray& a) { drawPolygon(false, a.p, a.np); }

inline QColorGroup::QColorGroup(ThemeEngine *engine) { this->engine = engine->engine; }
inline Color QColorGroup::shadow() const { return Color(&engine->palette[MT_BUTTON_SHADOW]); }
inline Color QColorGroup::light() const { return Color(&engine->palette[MT_BUTTON_LIGHT]); }
inline Color QColorGroup::mid() const { return Color(&engine->palette[MT_BUTTON_MID]); }
inline Color QColorGroup::midlight() const { return Color(&engine->palette[MT_BUTTON_MIDLIGHT]); }
inline Color QColorGroup::dark() const { return Color(&engine->palette[MT_BUTTON_DARK]); }
inline Color QColorGroup::button() const { return Color(&engine->palette[MT_BUTTON_BACKGROUND]); }
inline Color QColorGroup::buttonText() const { return Color(&engine->palette[MT_BUTTON_FOREGROUND]); }
inline Color QColorGroup::base() const { return Color(&engine->palette[MT_WINDOW_BACKGROUND]); }
inline Color QColorGroup::text() const { return Color(&engine->palette[MT_WINDOW_FOREGROUND]); }
inline Color QColorGroup::background() const { return Color(&engine->palette[MT_BACKGROUND]); }
inline Color QColorGroup::highlight() const { return Color(&engine->palette[MT_SELECTED_BACKGROUND]); }
inline Color QColorGroup::highlightedText() const { return Color(&engine->palette[MT_SELECTED_FOREGROUND]); }

inline const Color &QColorGroup::brush(ColorRole role)
{
   switch (role) {
      case Mid: brushes[role] = mid(); break;
      case Button: brushes[role] = button(); break;
      case Midlight: brushes[role] = midlight(); break;
      case Highlight: brushes[role] = highlight(); break;
   }
   return brushes[role];
}

inline QPointArray::QPointArray(): np(0), p(NULL) { }
inline QPointArray::QPointArray(int nPoints): np(nPoints) { p = (MT_POINT *)malloc(sizeof(MT_POINT) * nPoints); }
inline QPointArray::QPointArray(int nPoints, const QCOORD* points): np(0), p(NULL) { setPoints(nPoints, points); }
inline QPointArray::~QPointArray() { if (p) free(p); }
inline void QPointArray::setPoint(int i, int x, int y) { p[i].x = x; p[i].y = y; }

inline void QPointArray::setPoints(int nPoints, const QCOORD* points)
{
   int i;

   if (p) free(p);

   np = nPoints;
   p = (MT_POINT *)malloc(sizeof(MT_POINT) * np);
   for (i=0; i<np; i++) {
      p[i].x = points[i*2];
      p[i].y = points[i*2+1];
   }
}

inline void QPointArray::translate(int dx, int dy)
{
   int i;

   for (i=0; i<np; i++) {
      p[i].x += dx;
      p[i].y += dy;
   }
}

} /* end of namespace metathemeqt */

using namespace metathemeqt;

#endif /* METATHEMEQT_H */
