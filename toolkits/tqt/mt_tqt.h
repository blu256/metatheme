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

#ifndef MT_TQT_H
#define MT_TQT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tdestyle.h>

#include <tqbitmap.h>
#include <tqptrdict.h>

extern "C" {
#include "metathemeInt.h"
};

class MetaThemeStyle: public TDEStyle
{
   TQ_OBJECT

public:
   MetaThemeStyle();
   virtual ~MetaThemeStyle();

   int retrieveState(SFlags flags) const;
   void retrieveColors(const TQWidget* widget, SFlags flags, const TQColorGroup &cg, MT_WIDGET_DATA *data, PrimitiveElement pe = PE_CustomBase) const;

   virtual void polish(TQApplication *app);
   virtual void polish(TQPalette &);
   virtual void polish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr);
   virtual void unPolish(const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr);

   void drawTDEStylePrimitive(TDEStylePrimitive kpe,
      TQPainter *p,
      const TQWidget *widget,
      const TQRect &r,
      const TQColorGroup &cg,
      SFlags flags = Style_Default,
      const TQStyleOption& = TQStyleOption::Default) const;

   void drawPrimitive(PrimitiveElement pe,
      TQPainter *p,
      const TQRect &r,
      const TQColorGroup &cg,
      SFlags flags = Style_Default,
      const TQStyleOption &opt = TQStyleOption::Default) const;

   void drawControl(ControlElement element,
      TQPainter *p,
      const TQStyleControlElementData &ceData,
      ControlElementFlags elementFlags,
      const TQRect &r,
      const TQColorGroup &cg,
      SFlags flags = Style_Default,
      const TQStyleOption& = TQStyleOption::Default,
      const TQWidget *widget = 0) const;

   void drawControlMask(ControlElement element,
      TQPainter *p,
      const TQStyleControlElementData &ceData,
      ControlElementFlags elementFlags,
      const TQRect &r,
      const TQStyleOption& = TQStyleOption::Default,
      const TQWidget *widget = 0) const;

   void drawComplexControl(ComplexControl control,
      TQPainter *p,
      const TQStyleControlElementData &ceData,
      ControlElementFlags elementFlags,
      const TQRect &r,
      const TQColorGroup &cg,
      SFlags flags = Style_Default,
      SCFlags controls = SC_All,
      SCFlags active = SC_None,
      const TQStyleOption& = TQStyleOption::Default,
      const TQWidget *widget = 0) const;

   void drawItem(TQPainter *p,
      const TQRect &r,
      int flags,
      const TQColorGroup &g,
      bool enabled,
      const TQPixmap *pixmap,
      const TQString& text,
      int len = -1,
      const TQColor *penColor = 0) const;

   int pixelMetric(PixelMetric m, const TQStyleControlElementData &ceData,
                   ControlElementFlags elementFlags, const TQWidget *widget = 0) const;

   TQRect subRect(SubRect r, const TQStyleControlElementData &ceData,
                  ControlElementFlags elementFlags, const TQWidget *widget) const;

   TQRect querySubControlMetrics(ComplexControl control,
      const TQStyleControlElementData &ceData,
      ControlElementFlags elementFlags,
      const TQWidget *widget,
      SubControl subcontrol,
      const TQStyleOption &opt = TQStyleOption::Default) const;

   TQSize sizeFromContents(TQStyle::ContentsType t,
                           const TQStyleControlElementData &ceData,
                           ControlElementFlags elementFlags,
                           const TQSize &s,
                           const TQStyleOption &o,
                           const TQWidget *w) const;

   int styleHint(StyleHint stylehint,
                 const TQStyleControlElementData &ceData,
                 ControlElementFlags elementFlags,
                 const TQWidget *widget = 0,
                 const TQStyleOption &opt = TQStyleOption::Default,
                 TQStyleHintReturn *returnData = 0) const;
   bool objectEventHandler(const TQStyleControlElementData &ceData,
                           ControlElementFlags elementFlags, void* source,
                           TQEvent *ev );
   void setColorPalette(MT_COLOR_PALETTE *pal);
   void setFont();

   TQWidget *hoverWidget;
   int hoverPart;
   TQWidget *toolButtonDropDownActiveWidget;

protected slots:
   void updateFont();

private:
   // disable copy constructor and = operator
   MetaThemeStyle(const MetaThemeStyle &);
   MetaThemeStyle &operator =(const MetaThemeStyle &);
   TQStyle *winstyle;

   bool pseudo3D, useTextShadows, roundedCorners, reverseLayout, kickerMode;

private:
   MT_ENGINE *mt_engine;
   TQPalette qpalette;
   TQFont qfont;
   bool qtonly;
   TQPtrDict<TQWidget> tabWidgets;
};

class MetaThemeHelper: public TQObject
{
   TQ_OBJECT

public slots:
   void widgetDestroyedForData(TQObject *obj);
   void widgetDestroyedForRef(TQObject *obj);

};

#endif /* MT_TQT_H */
