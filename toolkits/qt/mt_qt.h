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

#ifndef MT_QT_H
#define MT_QT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_KDE
#include <kstyle.h>
#else
#include <qwindowsstyle.h>
#endif

#include <qbitmap.h>
#include <qptrdict.h>

extern "C" {
#include "metathemeInt.h"
};

#ifndef HAVE_KDE
#define KStyle QWindowsStyle
#endif


class MetaThemeStyle: public KStyle
{
   Q_OBJECT

public:
   MetaThemeStyle();
   virtual ~MetaThemeStyle();

   int retrieveState(SFlags flags) const;
   void retrieveColors(const QWidget* widget, SFlags flags, const QColorGroup &cg, MT_WIDGET_DATA *data, PrimitiveElement pe = PE_CustomBase) const;
   
   virtual void polish(QApplication *app);
   virtual void polish(QPalette &);
   virtual void polish(QWidget *widget);
   virtual void unPolish(QWidget *widget);

#ifdef HAVE_KDE
   void drawKStylePrimitive(KStylePrimitive kpe,
      QPainter *p,
      const QWidget *widget,
      const QRect &r,
      const QColorGroup &cg,
      SFlags flags = Style_Default,
      const QStyleOption& = QStyleOption::Default) const;
#endif

   void drawPrimitive(PrimitiveElement pe,
      QPainter *p,
      const QRect &r,
      const QColorGroup &cg,
      SFlags flags = Style_Default,
      const QStyleOption &opt = QStyleOption::Default) const;

   void drawControl(ControlElement element,
      QPainter *p,
      const QWidget *widget,
      const QRect &r,
      const QColorGroup &cg,
      SFlags flags = Style_Default,
      const QStyleOption& = QStyleOption::Default) const;

   void drawControlMask(ControlElement element,
      QPainter *p,
      const QWidget *widget,
      const QRect &r,
      const QStyleOption& = QStyleOption::Default) const;

   void drawComplexControl(ComplexControl control,
      QPainter *p,
      const QWidget *widget,
      const QRect &r,
      const QColorGroup &cg,
      SFlags flags = Style_Default,
      SCFlags controls = SC_All,
      SCFlags active = SC_None,
      const QStyleOption& = QStyleOption::Default) const;

   void drawItem(QPainter *p,
      const QRect &r,
      int flags,
      const QColorGroup &g,
      bool enabled,
      const QPixmap *pixmap,
      const QString& text,
      int len = -1,
      const QColor *penColor = 0) const;

   int pixelMetric(PixelMetric m,
      const QWidget *widget = 0) const;

   QRect subRect(SubRect r,
      const QWidget *widget) const;

   QRect querySubControlMetrics(ComplexControl control,
      const QWidget *widget,
      SubControl subcontrol,
      const QStyleOption &opt = QStyleOption::Default) const;

   QSize sizeFromContents(QStyle::ContentsType t,
                          const QWidget *w,
                          const QSize &s,
                          const QStyleOption &o) const;

   int styleHint(StyleHint stylehint, const QWidget *widget = 0, const QStyleOption &opt = QStyleOption::Default, QStyleHintReturn *returnData = 0) const;
   bool eventFilter(QObject *, QEvent *);
   void setColorPalette(MT_COLOR_PALETTE *pal);
   void setFont();

   QWidget *hoverWidget;
   int hoverPart;
   QWidget *toolButtonDropDownActiveWidget;

protected slots:
   void updateFont();

private:
   // disable copy constructor and = operator
   MetaThemeStyle(const MetaThemeStyle &);
   MetaThemeStyle &operator =(const MetaThemeStyle &);
   QStyle *winstyle;

   bool pseudo3D, useTextShadows, roundedCorners, reverseLayout, kickerMode;

private:
   MT_ENGINE *mt_engine;
   QPalette qpalette;
   QFont qfont;
   bool qtonly;
   QPtrDict<QWidget> tabWidgets;
};

class MetaThemeHelper: public QObject
{
   Q_OBJECT
   
public slots:
   void widgetDestroyedForData(QObject *obj);
   void widgetDestroyedForRef(QObject *obj);

};

#endif /* MT_QT_H */
