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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <tqstyleplugin.h>
#include <tqstylefactory.h>
#include <tqpointarray.h>
#include <tqpainter.h>
#include <tqtabbar.h>
#include <tqprogressbar.h>
#include <tqcombobox.h>
#include <tqlistbox.h>
#include <tqscrollbar.h>
#include <tqpushbutton.h>
#include <tqtoolbutton.h>
#include <tqtoolbar.h>
#include <tqmenubar.h>
#include <tqpopupmenu.h>
#include <tqdrawutil.h>
#include <tqapplication.h>
#include <tqvariant.h>
#include <tqpixmapcache.h>
#include <tqslider.h>
#include <tqsettings.h>
#include <tqimage.h>
#include <tqstylesheet.h>
#include <tqlineedit.h>
#include <tqobjectlist.h>
#ifdef HAVE_KDE
#include <kapplication.h>
#include <kpixmap.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#endif

#include <malloc.h>
#include <stdlib.h>

#include "mt_ntqt.h"
#include "mt_qt.moc"

#define MT_STRING_SET_TYPE(type, state) { stringType = (type); stringState = (state); }

extern "C" {

typedef struct {
   MT_COLOR col;
} MT_QT_GC;

typedef struct {
   const MetaThemeStyle *style;
   const TQRect &r;
   int flags;
   const TQColorGroup &g;
   const TQString& text;
   int len;
   const TQColor *penColor;
} MT_QT_STRING;

static bool toolButtonPopup = false;
static bool toggleButton = false;
static bool defaultButton = false;
static bool tabBarButton = false;
static bool drawMask = false;
static int stringType = 0;
static int stringState = 0;
static TQPtrDict<void *> widgetRefs;
static TQPtrDict<void> widgetData;
static TQPtrDict<void> widgetDataDispose;
static MetaThemeHelper helper;


void MetaThemeHelper::widgetDestroyedForData(TQObject *obj)
{
   void *data = widgetData[obj];
   if (!data) return;

   mt_destroy_func dispose_func = (mt_destroy_func)widgetDataDispose[obj];
   dispose_func(data);
   widgetData.remove(obj);
   widgetDataDispose.remove(obj);
   TQObject::disconnect(obj, SIGNAL(destroyed(TQObject *)), this, SLOT(widgetDestroyedForData(TQObject *)));
}


void MetaThemeHelper::widgetDestroyedForRef(TQObject *obj)
{
   TQPtrDictIterator<void *> it(widgetRefs);
   for( ; it.current(); ++it ) {
      MT_WIDGET *widget = (MT_WIDGET *)it.currentKey();
      if (widget && *widget == obj) {
         widgetRefs.remove(widget);
         break;
      }
   }

   TQObject::disconnect(obj, SIGNAL(destroyed(TQObject *)), this, SLOT(widgetDestroyedForRef(TQObject *)));
}


static MT_GC *_mt_gc_new_with_foreground(MT_COLOR *col)
{
   MT_QT_GC *gc = (MT_QT_GC *)malloc(sizeof(MT_QT_GC));
   gc->col = *col;
   return (MT_GC *)gc;
}


static void _mt_gc_destroy(MT_GC *gc)
{
   free(gc);
}


static void _mt_gc_set_clip_rectangle(MT_RECTANGLE *rect, ...)
{
}


static MT_RECTANGLE *_mt_gc_get_clip_rectangle(MT_GC *gc)
{
   return NULL;
}


static void _mt_gc_set_foreground(MT_GC *gc, MT_COLOR *col)
{
   MT_QT_GC *qtgc = (MT_QT_GC *)gc;
   qtgc->col = *col;
}


static MT_PIXMAP *_mt_pixmap_new_from_xpm(char **xpm_data)
{
   return (MT_PIXMAP *)new TQPixmap(xpm_data);
}


static MT_PIXMAP *_mt_pixmap_new_from_image(MT_IMAGE *img)
{
   TQImage *i = (TQImage *)img;
   i->setAlphaBuffer(false);
   TQPixmap *p = new TQPixmap(*i);
   i->setAlphaBuffer(true);
   return (MT_PIXMAP *)p;
}


static void _mt_pixmap_destroy(MT_PIXMAP *pixmap)
{
   delete (TQPixmap *)pixmap;
}


static MT_IMAGE *_mt_image_new(int width, int height)
{
   TQImage *img = new TQImage(width, height, 32);
   img->setAlphaBuffer(true);
   return (MT_IMAGE *)img;
}


static MT_IMAGE *_mt_image_new_from_xpm(char **xpm_data)
{
   TQImage *img = new TQImage(xpm_data);

   if (img->depth() != 32) {
      TQImage newimg = img->convertDepth(32);
      delete img;

      if (newimg.isNull()) {
         return NULL;
      }

      return (MT_IMAGE *)new TQImage(newimg);
   }

   return (MT_IMAGE *)img;
}


static MT_IMAGE *_mt_image_copy(MT_IMAGE *img)
{
   TQImage *qimg = new TQImage(*((const TQImage *)img));
   qimg->detach();
   return (MT_IMAGE *)qimg;
}


static void _mt_image_destroy(MT_IMAGE *image)
{
   delete (TQImage *)image;
}


static void _mt_image_replace_color(MT_IMAGE *img, int r, int g, int b, MT_COLOR *col)
{
   TQImage *qi = (TQImage *)img;
   uint i,j,col1,col2;

   col1 = (r << 16) + (g << 8) + b;
   col2 = (col->a << 24) + (col->r << 16) + (col->g << 8) + col->b;

   for (i=0; i<(uint)qi->height(); i++) {
      uint *p = (uint *)qi->scanLine(i);
      for (j=0; j<(uint)qi->width(); j++, p++) {
         if ((*p & 0xFFFFFF) == col1) {
            *p = col2;
         }
      }
   }
}


static void _mt_image_set_pixel(MT_IMAGE *img, int x, int y, MT_COLOR *col)
{
   TQImage *i = (TQImage *)img;
   i->setPixel(x, y, (col->a << 24) + (col->r << 16) + (col->g << 8) + col->b);
}


static void _mt_image_get_pixel(MT_IMAGE *img, int x, int y, MT_COLOR *col)
{
   TQRgb rgba = ((TQImage *)img)->pixel(x, y);
   col->a = tqAlpha(rgba);
   col->r = tqRed(rgba);
   col->g = tqGreen(rgba);
   col->b = tqBlue(rgba);
}


static void _mt_image_fill(MT_IMAGE *img, MT_COLOR *col)
{
   TQImage *i = (TQImage *)img;
   i->fill((255 << 24) + (col->r << 16) + (col->g << 8) + col->b);
}


static void _mt_image_get_size(MT_IMAGE *img, int *width, int *height)
{
   TQImage *qi = (TQImage *)img;
   *width = qi->width();
   *height = qi->height();
}


static void _mt_draw_point(MT_WINDOW *win, MT_GC *gc, int x, int y)
{
   TQPainter *p = (TQPainter *)win;
   MT_QT_GC *qtgc = (MT_QT_GC *)gc;

   p->setPen(TQColor(qtgc->col.r, qtgc->col.g, qtgc->col.b));
   p->drawPoint(x, y);
}


static void _mt_draw_line(MT_WINDOW *win, MT_GC *gc, int x1, int y1, int x2, int y2)
{
   TQPainter *p = (TQPainter *)win;
   MT_QT_GC *qtgc = (MT_QT_GC *)gc;

   p->setPen(TQColor(qtgc->col.r, qtgc->col.g, qtgc->col.b));
   p->drawLine(x1, y1, x2, y2);
}


static void _mt_draw_rectangle(MT_WINDOW *win, MT_GC *gc, int filled, int x1, int y1, int x2, int y2)
{
   TQPainter *p = (TQPainter *)win;
   MT_QT_GC *qtgc = (MT_QT_GC *)gc;

   p->setPen(TQColor(qtgc->col.r, qtgc->col.g, qtgc->col.b));
   if (filled) p->setBrush(TQColor(qtgc->col.r, qtgc->col.g, qtgc->col.b));
   p->drawRect(x1, y1, x2-x1+1, y2-y1+1);
}


static void _mt_draw_polygon(MT_WINDOW *win, MT_GC *gc, int filled, MT_POINT *points, int npoints)
{
   TQPainter *p = (TQPainter *)win;
   MT_QT_GC *qtgc = (MT_QT_GC *)gc;

   TQPointArray a(npoints);
   for (int i=0; i<npoints; i++) {
      a.setPoint(i, points[i].x, points[i].y);
   }

   if (filled) {
      p->setPen(TQt::NoPen);
      p->setBrush(TQColor(qtgc->col.r, qtgc->col.g, qtgc->col.b));
      p->drawPolygon(a, false);
      p->setPen(TQt::SolidLine);
   }
   else {
      p->setPen(TQColor(qtgc->col.r, qtgc->col.g, qtgc->col.b));
      p->drawPolyline(a);
   }
}


static void _mt_draw_string(MT_WINDOW *win, MT_STRING *str, int x_offset, int y_offset, MT_COLOR *color)
{
   TQPainter *p = (TQPainter *)win;
   MT_QT_STRING *s = (MT_QT_STRING *)str;
   TQRect r(s->r);
   const TQColor *penColor = s->penColor;
   TQColor tmpColor;
   
   r.moveBy(x_offset, y_offset);

   if (color) {
      tmpColor.setRgb(color->r, color->g, color->b);
      penColor = &tmpColor;
   }

   s->style->drawItem(p, r, s->flags, s->g, true, NULL, s->text, s->len, penColor);
}


static void _mt_draw_pixmap(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int xsrc, int ysrc, int xdest, int ydest, int width, int height)
{
   TQPainter *p = (TQPainter *)win;

   p->drawPixmap(xdest, ydest, *((const TQPixmap *)pixmap), xsrc, ysrc, width, height);
}


static void _mt_draw_pixmap_tiled(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int srcwidth, int srcheight, int xdest, int ydest, int width, int height)
{
   TQPainter *p = (TQPainter *)win;

   p->drawTiledPixmap(xdest, ydest, width, height, *((const TQPixmap *)pixmap));
}


static void _mt_draw_image(MT_WINDOW *win, MT_GC *gc, MT_IMAGE *img, int xsrc, int ysrc, int wsrc, int hsrc, int xdest, int ydest, int width, int height)
{
   TQPainter *p = (TQPainter *)win;
   TQImage *i = (TQImage *)img;

   p->drawImage(xdest, ydest, i->copy(xsrc, ysrc, wsrc, hsrc).scale(width, height));
}


static MT_WIDGET *_mt_widget_ref(MT_WIDGET *widget)
{
   MT_WIDGET *result = (MT_WIDGET *)malloc(sizeof(MT_WIDGET));
   *result = *widget;

   // mark the reference mapping:
   widgetRefs.insert(result, result);
   
   TQWidget *w = (TQWidget *)*widget;
   TQObject::connect(w, SIGNAL(destroyed(TQObject *)), &helper, SLOT(widgetDestroyedForRef(TQObject *)));

   return result;
}


static void _mt_widget_unref(MT_WIDGET *widget)
{
   MT_WIDGET *handle = widgetRefs[widget];

   // if it is a handle then check if the widget is still available:
   if (handle && *handle == NULL) return;

   widgetRefs.remove(widget);
   TQObject::disconnect((TQWidget *)*widget, SIGNAL(destroyed(TQObject *)), &helper, SLOT(widgetDestroyedForRef(TQObject *)));
   free(widget);
}


static MT_WIDGET *_mt_widget_get_parent(MT_WIDGET *widget, MT_WIDGET *result)
{
   MT_WIDGET *handle = widgetRefs[widget];

   // if it is a handle then check if the widget is still available:
   if (handle && *handle == NULL) return NULL;

   if (!result) result = (MT_WIDGET *)malloc(sizeof(MT_WIDGET));

   *result = ((TQWidget *)*widget)->parentWidget(TRUE);
   return result;
}


static void _mt_widget_set_data(MT_WIDGET *widget, void *data, mt_destroy_func data_dispose)
{
   MT_WIDGET *handle = widgetRefs[widget];

   // if it is a handle then check if the widget is still available:
   if (handle && *handle == NULL) return;

   // obtain the real address of TQWidget:
   TQWidget *w = (TQWidget *)*widget;

   // check if it already contains data and free them:
   if (widgetData[w]) {
      mt_destroy_func destroy_func = (mt_destroy_func)widgetDataDispose[w];
      
      if (destroy_func) {
         destroy_func(widgetData[w]);
         widgetDataDispose.remove(w);
      }
   
      TQObject::disconnect(w, SIGNAL(destroyed(TQObject *)), &helper, SLOT(widgetDestroyedForData(TQObject *)));
   }

   if (!data) return;

   widgetData.replace(w, data);
   if (data_dispose) {
      widgetDataDispose.replace(w, (void *)data_dispose);
      TQObject::connect(w, SIGNAL(destroyed(TQObject *)), &helper, SLOT(widgetDestroyedForData(TQObject *)));
   }
}


static void *_mt_widget_get_data(MT_WIDGET *widget)
{
   MT_WIDGET *handle = widgetRefs[widget];

   // if it is a handle then check if the widget is still available:
   if (handle && *handle == NULL) return NULL;

   return widgetData[*widget];
}


static MT_TOOLKIT mt = {
   _mt_gc_new_with_foreground,
   _mt_gc_destroy,
   _mt_gc_set_clip_rectangle,
   _mt_gc_get_clip_rectangle,
   _mt_gc_set_foreground,

   _mt_pixmap_new_from_xpm,
   _mt_pixmap_new_from_image,
   _mt_pixmap_destroy,

   _mt_image_new,
   _mt_image_new_from_xpm,
   _mt_image_copy,
   _mt_image_destroy,
   _mt_image_replace_color,
   _mt_image_set_pixel,
   _mt_image_get_pixel,
   _mt_image_fill,
   _mt_image_get_size,

   _mt_draw_point,
   _mt_draw_line,
   _mt_draw_rectangle,
   _mt_draw_polygon,
   _mt_draw_string,
   _mt_draw_pixmap,
   _mt_draw_pixmap_tiled,
   _mt_draw_image,

   NULL,
   NULL,
   
   _mt_widget_ref,
   _mt_widget_unref,
   _mt_widget_get_parent,
   _mt_widget_set_data,
   _mt_widget_get_data
};


} /* extern "C" */


class MetaThemeStylePlugin: public TQStylePlugin
{
public:
   MetaThemeStylePlugin() {}
   ~MetaThemeStylePlugin() {}

   TQStringList keys() const
   {
      return TQStringList() << "MetaTheme";
   }

   TQStyle* create(const TQString& key)
   {
      if (key == "metatheme") return new MetaThemeStyle;
      return 0;
   }
};

Q_EXPORT_PLUGIN(MetaThemeStylePlugin)


MetaThemeStyle::MetaThemeStyle()
#ifdef HAVE_KDE
   :KStyle(AllowMenuTransparency), kickerMode(false)
#endif
{
   TQSettings settings;
   pseudo3D = settings.readBoolEntry("/KStyle/Settings/Pseudo3D", true);
   roundedCorners = settings.readBoolEntry("/KStyle/Settings/RoundedCorners", true);
   useTextShadows = settings.readBoolEntry("/KStyle/Settings/UseTextShadows", false);
   reverseLayout = TQApplication::reverseLayout();

   metatheme_toolkit_init(&mt);

   MT_LIBRARY *mt_library = metatheme_library_init(&mt);
   if (!mt_library) {
      exit(1);
   }

   mt_engine = metatheme_load_engine(mt_library, NULL);
   if (!mt_engine) {
      metatheme_library_exit(mt_library);
      exit(1);
   }

   metatheme_load_default_settings(mt_engine);
   mt_engine->init(mt_engine);
   mt_engine->realize(mt_engine);

   setColorPalette(&(mt_engine->palette));
   setFont();

   hoverWidget = 0;
   hoverPart = 0;
   toolButtonPopup = false;
   toolButtonDropDownActiveWidget = NULL;

#ifdef HAVE_KDE
   qtonly = (tqApp && tqApp->inherits("KApplication"))? false : true;
#else
   qtonly = true;
#endif

#ifdef HAVE_KDE
   if (!qtonly) {
      connect(kapp, SIGNAL(kdisplayFontChanged()), this, SLOT(updateFont()));
   }
#endif
}


MetaThemeStyle::~MetaThemeStyle()
{
#ifdef HAVE_KDE
   if (!qtonly && kapp) {
      disconnect(kapp, SIGNAL(kdisplayFontChanged()), this, SLOT(updateFont()));
   }
#endif

   if (mt_engine) {
      MT_LIBRARY *mt_library = mt_engine->library;
      mt_engine->exit(mt_engine);
      metatheme_unload_engine(mt_engine);
      metatheme_library_exit(mt_library);
   }
}


void MetaThemeStyle::polish(TQApplication *app)
{
   if (!qstrcmp(app->argv()[0], "kicker")) kickerMode = true;
}


void MetaThemeStyle::polish(TQPalette &pal)
{
   pal = qpalette;
}


void MetaThemeStyle::polish(TQWidget *widget)
{
   if (widget->inherits("TQPushButton") || widget->inherits("TQComboBox") ||
       widget->inherits("TQSpinWidget") || widget->inherits("TQSlider") ||
       widget->inherits("TQCheckBox") || widget->inherits("TQRadioButton") ||
       widget->inherits("KToolBarButton") || widget->inherits("TQLineEdit") ||
       widget->inherits("TQComboBox") || widget->inherits("TQFrame")) {
      widget->installEventFilter(this);
   }

   if (widget->inherits("TQScrollBar")) {
      widget->installEventFilter(this);
      widget->setMouseTracking(true);
   }

#ifdef HAVE_KDE
   if (widget->inherits("KToolBarButton")) {
      static_cast<KToolBarButton*>(widget)->modeChange();
   }
#endif

   if (widget->inherits("TQTipLabel")) {
      TQPalette pal(widget->palette());
      pal.setColor(TQPalette::Active, TQColorGroup::Background, TQColor(255, 255, 225));
      widget->setPalette(pal);
   }

   if (widget->inherits("TQToolBar") || widget->inherits("TQToolButton") || widget->inherits("TQMenuBar")) {
      if (widget->backgroundMode() == TQt::PaletteButton) {
         widget->setBackgroundMode(TQt::PaletteBackground);
      }
   }

   if (mt_engine->metric[MT_NOTEBOOK_IS_FILLED]) {
      if (!qstrcmp(widget->name(), "tab pages")) {
         widget->installEventFilter(this);
      }
      else if (widget->backgroundMode() == TQt::PaletteBackground && !widget->ownPalette() && !tabWidgets.find(widget)) {
         TQWidget *w = widget;
         while (w) {
            if (w->inherits("TQScrollView") || w->inherits("TQWorkspace")) {
               break;
            }

            if (!qstrcmp(w->name(), "tab pages")) {
               tabWidgets.replace(widget, w);
               widget->installEventFilter(this);
               widget->setBackgroundMode(TQt::NoBackground);
               break;
            }

            w = w->parentWidget(true);
         }
      }
   }

   KStyle::polish(widget);
}


void MetaThemeStyle::unPolish(TQWidget *widget)
{
   if (widget->inherits("TQPushButton") || widget->inherits("TQComboBox") ||
       widget->inherits("TQSpinWidget") || widget->inherits("TQSlider") ||
       widget->inherits("TQCheckBox") || widget->inherits("TQRadioButton") ||
       widget->inherits("KToolBarButton") || widget->inherits("TQLineEdit") ||
       widget->inherits("TQComboBox") || widget->inherits("TQFrame")) {
      widget->removeEventFilter(this);
   }

   if (widget->inherits("TQScrollBar")) {
      widget->removeEventFilter(this);
      widget->setMouseTracking(false);
   }

   if (widget->inherits("TQLineEdit") && widget->backgroundPixmap()) {
      widget->setBackgroundMode(TQt::PaletteBase);
   }

   if (mt_engine->metric[MT_NOTEBOOK_IS_FILLED]) {
      if (!qstrcmp(widget->name(), "tab pages")) {
         widget->removeEventFilter(this);

         // unpolish all registered widgets with this tab pages widget:
         TQPtrDictIterator<TQWidget> it(tabWidgets);
         while (it.current()) {
            if (it.current() == widget) {
               unPolish((TQWidget *)it.currentKey());
            }
            ++it;
         }
      }

      if (tabWidgets.find(widget)) {
         widget->removeEventFilter(this);
         widget->setBackgroundMode(TQt::PaletteBackground);

         tabWidgets.remove(widget);
      }
   }

   KStyle::unPolish(widget);
}


#ifdef HAVE_KDE
void MetaThemeStyle::drawKStylePrimitive(KStylePrimitive kpe,
                                      TQPainter *p,
                                      const TQWidget *widget,
                                      const TQRect &r,
                                      const TQColorGroup &cg,
                                      SFlags flags,
                                      const TQStyleOption& opt) const
{
   switch (kpe) {
      default:
         KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);
   }
}
#endif


int MetaThemeStyle::retrieveState(SFlags flags) const
{
   int state = MT_NORMAL;
   if (flags & Style_MouseOver) state |= MT_HOVER;
   if (flags & Style_Down) state |= MT_ACTIVE;
   if (!(flags & Style_Enabled)) state |= MT_DISABLED;
   if (flags & Style_Selected) state |= MT_SELECTED;
   if (flags & Style_Active) state |= MT_MOUSE_ACTIVE;
   return state;
}


void MetaThemeStyle::retrieveColors(const TQWidget *widget, SFlags flags, const TQColorGroup &cg, MT_WIDGET_DATA *data, PrimitiveElement pe) const
{
   TQColor bg = cg.background();
   switch (pe) {
      case PE_ButtonCommand:
      case PE_ButtonDefault:
      case PE_ButtonBevel:
      case PE_ButtonTool:
      case PE_ButtonDropDown:
         bg = cg.button();
         break;

      default:
         break;
   }
   mt_color_set(data->background_color, bg.red(), bg.green(), bg.blue());
}


static void debug_flags(int flags)
{
   if (flags & TQStyle::Style_Default) printf("Default ");
   if (flags & TQStyle::Style_Enabled) printf("Enabled ");
   if (flags & TQStyle::Style_Raised) printf("Raised ");
   if (flags & TQStyle::Style_Sunken) printf("Sunken ");
   if (flags & TQStyle::Style_Off) printf("Off ");
   if (flags & TQStyle::Style_NoChange) printf("NoChange ");
   if (flags & TQStyle::Style_On) printf("On ");
   if (flags & TQStyle::Style_Down) printf("Down ");
   if (flags & TQStyle::Style_Horizontal) printf("Horizontal ");
   if (flags & TQStyle::Style_HasFocus) printf("HasFocus ");
   if (flags & TQStyle::Style_Top) printf("Top ");
   if (flags & TQStyle::Style_Bottom) printf("Bottom ");
   if (flags & TQStyle::Style_FocusAtBorder) printf("FocusAtBorder ");
   if (flags & TQStyle::Style_AutoRaise) printf("AutoRaise ");
   if (flags & TQStyle::Style_MouseOver) printf("MouseOver ");
   if (flags & TQStyle::Style_Up) printf("Up ");
   if (flags & TQStyle::Style_Selected) printf("Selected ");
   if (flags & TQStyle::Style_HasFocus) printf("HasFocus ");
   if (flags & TQStyle::Style_Active) printf("Active ");
   if (flags & TQStyle::Style_ButtonDefault ) printf("ButtonDefault  ");
   printf("\n");
}


void MetaThemeStyle::drawPrimitive(PrimitiveElement pe,
                                TQPainter *p,
                                const TQRect &r,
                                const TQColorGroup &cg,
                                SFlags flags,
                                const TQStyleOption &opt ) const
{
   MT_WIDGET_DATA data;

   int x, x2, y, y2, w, h;
   r.rect(&x, &y, &w, &h);
   r.coords(&x, &y, &x2, &y2);
   TQRect rect(r);

   int type=0, state=retrieveState(flags);
   retrieveColors((const TQWidget *)NULL, flags, cg, &data, pe);
   data.widget = NULL;

   // hack:
   if (pe == PE_ArrowDown && toolButtonPopup) {
      toolButtonPopup = false;
      return;
   }

   // hack:
   if (tabBarButton && (pe == PE_ArrowLeft || pe == PE_ArrowRight)) {
      tabBarButton = false;
      return;
   }

   switch (pe) {
      case PE_ButtonBevel:
      case PE_ButtonDropDown:
      case PE_ButtonCommand:
      {
         if (flags & Style_HasFocus) state |= MT_SELECTED;

         p->setPen(cg.button());
         p->setBrush(cg.button());
         p->drawRect(r.x(), r.y(), r.width(), r.height());

         if (toggleButton) {
            state &= ~MT_ACTIVE;
            if (flags & (Style_Down | Style_On)) state |= MT_ACTIVE;
         }

         type = MT_BUTTON;
         // TODO: if (defaultButton) type = MT_BUTTON_DEFAULT;
         if (toggleButton) type = MT_BUTTON_TOGGLE;

         MT_STRING_SET_TYPE(type, state);
         goto draw;
      }

      case PE_ButtonDefault: return;

      case PE_HeaderSection:
      {
         if (kickerMode) {
            type = MT_BUTTON_TOGGLE;
            state &= ~MT_DISABLED;
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, type, state, rect.x(), rect.y(), rect.width(), rect.height(), &data);

            p->setPen(TQColor(mt_engine->palette[MT_BUTTON_FOREGROUND].r, mt_engine->palette[MT_BUTTON_FOREGROUND].g, mt_engine->palette[MT_BUTTON_FOREGROUND].b));
            break;
         }

         type = MT_BUTTON_HEADER;
         p->setClipRect(rect);
         goto draw;
      }

      case PE_ButtonTool:
      {
         if ((flags & Style_Raised) && !(flags & Style_Down)) state |= MT_HOVER;
         type = MT_TOOLBAR_ITEM; goto draw;
      }

      case PE_ArrowUp: type = MT_ARROW_UP; goto draw;
      case PE_ArrowDown: type = MT_ARROW_DOWN; goto draw;
      case PE_ArrowLeft: type = MT_ARROW_LEFT; goto draw;
      case PE_ArrowRight: type = MT_ARROW_RIGHT; goto draw;

      //case PE_CheckMark: // TODO: test
      case PE_Indicator:
      case PE_ExclusiveIndicator:
      case PE_CheckListIndicator:
      case PE_CheckListExclusiveIndicator:
      {
         data.flags = 0;
         if (drawMask) data.flags |= MT_DRAW_MASK;

         state &= ~(MT_ACTIVE | MT_MOUSE_ACTIVE);
         if (flags & Style_On) state |= MT_ACTIVE;
         if (flags & Style_Down) state |= MT_MOUSE_ACTIVE;

         if (pe == PE_CheckListIndicator || pe == PE_CheckListExclusiveIndicator) {
            rect.moveBy(-((mt_engine->metric[MT_CHECKBOX_SIZE] - rect.width()) / 2), -((mt_engine->metric[MT_CHECKBOX_SIZE] - rect.height()) / 2));
            rect.setWidth(mt_engine->metric[MT_CHECKBOX_SIZE]);
            rect.setHeight(mt_engine->metric[MT_CHECKBOX_SIZE]);
         }

         switch (pe) {
            case PE_Indicator: type = MT_CHECK_BOX; break;
            case PE_ExclusiveIndicator: type = MT_RADIO_BOX; break;
            case PE_CheckListIndicator: type = MT_CHECK_CELL; break;
            case PE_CheckListExclusiveIndicator: type = MT_RADIO_CELL; break;
            default: return;
         }

         goto draw;
      }

      case PE_Panel:
      {
         type = (flags & Style_Sunken)? MT_BORDER_IN : MT_BORDER_OUT;
         goto draw;
      }

      case PE_PanelPopup: type = MT_MENU; goto draw;
      case PE_PanelMenuBar: type = MT_MENUBAR; goto draw;
      case PE_PanelDockWindow: type = MT_TOOLBAR; goto draw;
      case PE_PanelLineEdit: type = MT_ENTRY_BORDER; goto draw;
      case PE_StatusBarSection: type = MT_STATUSBAR_SECTION; goto draw; // TODO

      case PE_PanelGroupBox:
      case PE_GroupBoxFrame:
      {
         data.gap_x = 0;
         data.gap_width = 0;
         type = MT_STATICBOX;
         goto draw;
      }

      case PE_Separator:
      {
         if (!(flags & Style_Horizontal)) {
            rect.moveBy(0, -1);
            type = MT_HLINE;
         }
         else {
            rect.moveBy(-1, 0);
            type = MT_VLINE;
         }
         goto draw;
      }

      case PE_Splitter:
      {
         data.orientation = (rect.width() > rect.height())? MT_HORIZONTAL : MT_VERTICAL;
         type = MT_SPLITTER;
         goto draw;
      }

      case PE_DockWindowHandle:
      {
         data.orientation = (flags & Style_Horizontal)? MT_HORIZONTAL : MT_VERTICAL;
         type = MT_TOOLBAR_HANDLE;
         goto draw;
      }

      case PE_SpinWidgetUp:
      case PE_SpinWidgetDown:
      case PE_SpinWidgetMinus:
      case PE_SpinWidgetPlus:
      {
         data.flags = 0;
         if (flags & Style_On) state |= MT_ACTIVE;
         if (flags & Style_MouseOver) {
            state &= ~MT_HOVER;
            data.flags |= MT_SPINBUTTON_HOVER;
         }
         type = (pe == PE_SpinWidgetUp || pe == PE_SpinWidgetMinus)? MT_SPINBUTTON_UP : MT_SPINBUTTON_DOWN;
         goto draw;
      }

      case PE_TabBarBase:
      case PE_PanelTabWidget:
      {
         data.gap_x = 0;
         data.gap_width = 0;
         data.gap_position = 0;

         type = MT_NOTEBOOK;
         goto draw;
      }

      case PE_FocusRect:
      {
         /* TODO */
         return;
      }

#if TQT_VERSION >= 0x030300
      case PE_RubberBand:
      {
         p->drawWinFocusRect(r);
         return;
      }
#endif

      default:
         break;
   }

   return;

draw:
   mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, type, state, rect.x(), rect.y(), rect.width(), rect.height(), &data);
}


void MetaThemeStyle::drawControl(ControlElement element,
                              TQPainter *p,
                              const TQWidget *widget,
                              const TQRect &r,
                              const TQColorGroup &cg,
                              SFlags flags,
                              const TQStyleOption& opt) const
{
   if (widget == hoverWidget) flags |= Style_MouseOver;

   int state = retrieveState(flags);
   MT_WIDGET_DATA data;

   retrieveColors(widget, flags, cg, &data);
   data.widget = (MT_WIDGET)widget;

   switch (element) {
      case CE_CheckBox: drawPrimitive(PE_Indicator, p, r, cg, flags); break;
      case CE_RadioButton: drawPrimitive(PE_ExclusiveIndicator, p, r, cg, flags); break;

      case CE_MenuBarItem:
      {
         if (flags & Style_Active) state = (state & ~MT_MOUSE_ACTIVE) | MT_HOVER;
         if ((flags & Style_Down) && !(flags & Style_Active)) state &= ~MT_ACTIVE;

         mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_MENUBAR_ITEM, state, r.x(), r.y(), r.width(), r.height(), &data);

         TQMenuItem *mi = opt.menuItem();
         int alignment = AlignCenter|ShowPrefix|DontClip|SingleLine;
#if TQT_VERSION >= 0x030300
         if (!styleHint(SH_UnderlineAccelerator, widget, TQStyleOption::Default, 0)) alignment |= NoAccel;
#endif

         TQRect r2 = r;
         if (state & MT_ACTIVE) {
            r2.moveBy(mt_engine->metric_size[MT_MENUBAR_ITEM_TEXT_OFFSET].x, mt_engine->metric_size[MT_MENUBAR_ITEM_TEXT_OFFSET].y);
         }

         MT_STRING_SET_TYPE(MT_MENUBAR_ITEM, state);
         drawItem(p, r2, alignment, cg, flags & Style_Enabled, mi->pixmap(), mi->text(), -1, &cg.buttonText());
         break;
      }

      case CE_PopupMenuItem:
      {
         if (!widget || opt.isDefault()) break;

         const TQPopupMenu *popup = (const TQPopupMenu *)widget;
         TQMenuItem *mi = opt.menuItem();
         if (!mi) break;

         int tab = opt.tabWidth();
         bool checkable = popup->isCheckable();
         int maxpmw = opt.maxIconWidth();
         int left;

         if (state & MT_MOUSE_ACTIVE) state = (state & ~MT_MOUSE_ACTIVE) | MT_SELECTED;

         // for compatibility with current implementation of menu in GTK2:
         if ((state & MT_DISABLED) && (state & MT_SELECTED)) state &= ~MT_SELECTED;

         if (mi->isSeparator()) {
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_MENU_ITEM_SEPARATOR, state, r.x(), r.y(), r.width(), r.height(), &data);
            break;
         }

         mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_MENU_ITEM, state, r.x(), r.y(), r.width(), r.height(), &data);

         maxpmw = TQMAX(maxpmw, 16);

         left = mt_engine->metric_size[MT_MENU_BORDER].x + maxpmw + 3;

         if (mi->iconSet()) {
            TQIconSet::Mode mode = (state & MT_DISABLED)? TQIconSet::Disabled : TQIconSet::Normal;
            if ((state & MT_ACTIVE) && !(state & MT_DISABLED)) mode = TQIconSet::Active;
            TQPixmap pixmap;
            if (checkable && mi->isChecked()) {
               pixmap = mi->iconSet()->pixmap(TQIconSet::Small, mode, TQIconSet::On);
            }
            else {
               pixmap = mi->iconSet()->pixmap(TQIconSet::Small, mode);
            }
            left = TQMAX(left, pixmap.width());

            int offset = (r.height() - pixmap.height()) / 2;

            p->drawPixmap(r.x() + mt_engine->metric_size[MT_MENU_BORDER].x, r.y() + offset, pixmap/*, 0, 0, pixmap.width(), TQMIN(pixmap.height(), r.height() - offset)*/);
         }
         else if (checkable) {
            int s = state & ~(MT_ACTIVE);
            if (mi->isChecked()) s |= MT_ACTIVE;
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_MENU_ITEM_CHECK, s, r.x() + mt_engine->metric_size[MT_MENU_BORDER].x, r.y() + mt_engine->metric_size[MT_MENU_BORDER].y, maxpmw, r.height() - 2*mt_engine->metric_size[MT_MENU_BORDER].y, &data);
         }

         TQString s = mi->text();
         if (!s.isNull()) {
            int t = s.find('\t');
            int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
            TQString ts;
            int tleft;

#if TQT_VERSION >= 0x030300
            if (!styleHint(SH_UnderlineAccelerator, widget)) text_flags |= NoAccel;
#endif
            text_flags |= (TQApplication::reverseLayout()? AlignRight : AlignLeft);

            if (t >= 0) {
               ts = s.mid(t + 1);
               s = s.left(t);
               tleft = r.width() - tab - 3 - mt_engine->metric_size[MT_MENU_BORDER].x;
            }

            int col;

            if (state & MT_DISABLED) {
               col = MT_DISABLED_FOREGROUND;
            }
            else {
               col = (state & MT_SELECTED)? MT_SELECTED_FOREGROUND : MT_FOREGROUND;
            }

            TQColor penColor(mt_engine->palette[col].r, mt_engine->palette[col].g, mt_engine->palette[col].b);

            p->save();
            MT_STRING_SET_TYPE(MT_MENU_ITEM, state);
            drawItem(p, TQRect(r.x() + left, r.y() + mt_engine->metric_size[MT_MENU_BORDER].y, r.width() - left, r.height() - 2*mt_engine->metric_size[MT_MENU_BORDER].y), text_flags, popup->colorGroup(), mi->isEnabled(), NULL, s, t, &penColor);

            if (t >= 0) {
               MT_STRING_SET_TYPE(MT_MENU_ITEM, state);
               drawItem(p, TQRect(r.x() + tleft, r.y() + mt_engine->metric_size[MT_MENU_BORDER].y, r.width() - tleft, r.height() - 2*mt_engine->metric_size[MT_MENU_BORDER].y), text_flags, popup->colorGroup(), mi->isEnabled(), NULL, ts, -1, &penColor);
            }
            p->restore();
         }

         if (mi->popup()) {
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_MENU_ITEM_ARROW, state, r.x() + r.width() - 1 - mt_engine->metric_size[MT_MENU_BORDER].x - 8, r.y() + mt_engine->metric_size[MT_MENU_BORDER].y, 8, r.height() - 2*mt_engine->metric_size[MT_MENU_BORDER].y, &data);
         }
         break;
      }

      case CE_ProgressBarGroove:
      {
         data.flags = 0;
         data.orientation = MT_HORIZONTAL;
         mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_PROGRESSBAR, state, r.x(), r.y(), r.width(), r.height(), &data);
         break;
      }

      case CE_ProgressBarContents:
      {
         const TQProgressBar *pb = dynamic_cast<const TQProgressBar*>(widget);
         data.flags = 0;
         data.orientation = MT_HORIZONTAL;
         mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_PROGRESSBAR_SLIDER, state, r.x(), r.y(), (int)((double)r.width() * ((double)pb->progress() / (double)pb->totalSteps())), r.height(), &data);
         break;
      }

      case CE_ProgressBarLabel:
      {
         const TQProgressBar *pb = dynamic_cast<const TQProgressBar*>(widget);
         TQRect r2(r);
         int progress = (int)((double)r.width() * ((double)pb->progress() / (double)pb->totalSteps()));

         p->save();

         r2.setWidth(progress);
         p->setClipRect(r2);
         MT_STRING_SET_TYPE(MT_PROGRESSBAR_SLIDER, state);
         drawItem(p, r, AlignCenter | SingleLine, cg, flags & Style_Enabled, 0, pb->progressString(), -1, &cg.highlightedText());

         r2.setX(r.x() + progress);
         r2.setWidth(r.width() - progress);
         p->setClipRect(r2);
         MT_STRING_SET_TYPE(MT_PROGRESSBAR, state);
         drawItem(p, r, AlignCenter | SingleLine, cg, flags & Style_Enabled, 0, pb->progressString(), -1, &cg.foreground());

         p->restore();
         break;
      }

      case CE_TabBarTab:
      {
         const TQTabBar *tb = (const TQTabBar *)widget;
         const TQTab *t = opt.tab();
         MT_WIDGET_DATA data;

         data.widget = (MT_WIDGET)widget;

         data.gap_position = (tb->shape() == TQTabBar::RoundedAbove || tb->shape() == TQTabBar::TriangularAbove)? MT_POSITION_TOP : MT_POSITION_BOTTOM;
         data.gap_x = 0;
         data.gap_width = 0;

         data.flags = 0;
         if (tb->indexOf(t->identifier()) == 0) data.flags |= MT_NOTEBOOK_FIRST_VISIBLE_TAB | MT_NOTEBOOK_FIRST_TAB;
         if (tb->indexOf(t->identifier()) == tb->count() - 1) data.flags |= MT_NOTEBOOK_LAST_TAB;

         // TODO: special case: Konsole
         /*
         if (tb->parent() && tb->parent()->parent() && tb->parent()->parent()->inherits("Konsole")) {
            data.flags &= ~(MT_NOTEBOOK_FIRST_VISIBLE_TAB | MT_NOTEBOOK_FIRST_TAB);
         }
         */

         mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_NOTEBOOK_TAB, state, r.x(), r.y(), r.width(), r.height(), &data);
         break;
      }

      case CE_TabBarLabel:
      {
         if (opt.isDefault()) break;

         const TQTabBar *tb = (const TQTabBar *)widget;
         TQTab *t = opt.tab();

         TQRect tr = r;
         if (t->identifier() == tb->currentTab()) {
            if (tb->shape() == TQTabBar::RoundedBelow || tb->shape() == TQTabBar::TriangularBelow) {
               tr.moveBy(0, mt_engine->metric[MT_NOTEBOOK_TEXT_OFFSET]);
            }
            else {
               tr.moveBy(0, -mt_engine->metric[MT_NOTEBOOK_TEXT_OFFSET]);
            }
         }

         int alignment = AlignCenter | ShowPrefix;
#if TQT_VERSION >= 0x030300
         if (!styleHint(SH_UnderlineAccelerator, widget, TQStyleOption::Default, 0)) {
            alignment |= NoAccel;
         }
#endif

         drawItem(p, tr, alignment, cg, flags & Style_Enabled, 0, t->text());

         if ((flags & Style_HasFocus) && !t->text().isEmpty()) {
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_FOCUS_TAB, state, r.x(), r.y(), r.width(), r.height(), &data);
         }
         break;
      }

      case CE_PushButton:
         if (widget == hoverWidget) flags |= Style_MouseOver;
         if (widget) {
            TQPushButton *btn = (TQPushButton *)widget;
            if (btn->isToggleButton()) toggleButton = true;
            if (btn->isDefault()) defaultButton = true;
         }
         KStyle::drawControl(element, p, widget, r, cg, flags, opt);
         toggleButton = false;
         defaultButton = false;
         break;

      case CE_PushButtonLabel:
      {
         int x, y, w, h;
         r.rect(&x, &y, &w, &h);

         const TQPushButton *button = static_cast<const TQPushButton *>(widget);
         bool active = button->isOn() || button->isDown();

         // shift button contents if pushed:
         if (active) {
            x += pixelMetric(PM_ButtonShiftHorizontal, widget);
            y += pixelMetric(PM_ButtonShiftVertical, widget);
            flags |= Style_Sunken;
         }

         // draw the icon if there is one:
         if (button->iconSet() && !button->iconSet()->isNull()) {
            TQIconSet::Mode mode = TQIconSet::Disabled;
            TQIconSet::State state = TQIconSet::Off;

            if (button->isEnabled()) {
               mode = button->hasFocus()? (TQIconSet::Active):(TQIconSet::Normal);
            }
            if (button->isToggleButton() && button->isOn()) {
               state = TQIconSet::On;
            }

            TQPixmap pixmap = button->iconSet()->pixmap(TQIconSet::Small, mode, state);

            if (button->text().isEmpty() && !button->pixmap()) {
               p->drawPixmap(x + w/2 - pixmap.width()/2, y + h / 2 - pixmap.height() / 2, pixmap);
            }
            else {
               p->drawPixmap(x + 4, y + h / 2 - pixmap.height() / 2, pixmap);
            }

            int pw = pixmap.width();
            x += pw + 4;
            w -= pw + 4;
         }

         MT_STRING_SET_TYPE(MT_BUTTON, state);
         drawItem(p, TQRect(x, y, w, h), AlignCenter|ShowPrefix, button->colorGroup(), button->isEnabled(), button->pixmap(), button->text(), -1, &button->colorGroup().buttonText());

         if (flags & Style_HasFocus) {
            drawPrimitive(PE_FocusRect, p, visualRect(subRect(SR_PushButtonFocusRect, widget), widget), cg, flags);
         }
         break;
      }

      default:
         KStyle::drawControl(element, p, widget, r, cg, flags, opt);
   }
}


void MetaThemeStyle::drawControlMask(ControlElement element,
                              TQPainter *p,
                              const TQWidget *widget,
                              const TQRect &r,
                              const TQStyleOption& opt) const
{
   TQColorGroup cg(color1, color1, color1, color1, color1, color1, color1, color1, color0);

   switch (element) {
      case CE_CheckBox:
      case CE_RadioButton:
      {
         TQPixmap pixmap(r.size());
         TQRect r2(r);
         r2.moveTopLeft(TQPoint(0, 0));

         TQPainter *p2 = new TQPainter(&pixmap);
         p2->fillRect(r2, TQBrush(TQColor(255, 255, 255)));
         drawMask = true;
         drawPrimitive((element == CE_CheckBox)? PE_Indicator : PE_ExclusiveIndicator, p2, r2, cg, 0);
         drawMask = false;
         delete p2;

         TQImage image = pixmap.convertToImage().convertDepth(1, TQt::MonoOnly | TQt::ThresholdDither);
         p->drawImage(r.x(), r.y(), image);
         break;
      }

      default:
         p->fillRect(r, color1);
   }
}


void MetaThemeStyle::drawComplexControl(ComplexControl control,
                                     TQPainter *p,
                                     const TQWidget *widget,
                                     const TQRect &r,
                                     const TQColorGroup &cg,
                                     SFlags flags,
                                     SCFlags controls,
                                     SCFlags active,
                                     const TQStyleOption& opt) const
{
   MT_WIDGET_DATA data;
   int type=0, state=retrieveState(flags);

   retrieveColors(widget, flags, cg, &data);
   data.widget = (MT_WIDGET)widget;

   switch(control) {
      case CC_ScrollBar:
      {
         const TQScrollBar *scrollbar = (const TQScrollBar *) widget;
         TQRect addline, subline, addpage, subpage, slider, first, last;

         data.flags = 0;
         if (scrollbar->minValue() == scrollbar->maxValue() || (flags & Style_Sunken)) {
            data.flags |= MT_SCROLLBAR_UNSCROLLABLE;
         }

         subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
         addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
         subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
         addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
         slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
         first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
         last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);

         if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
            if (hoverWidget == widget && hoverPart == 2 && !scrollbar->draggingSlider()) state |= MT_HOVER;

            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL,
                  (scrollbar->orientation() == TQt::Horizontal)? MT_SCROLLBAR_ARROW_LEFT : MT_SCROLLBAR_ARROW_UP,
                  state | (active == SC_ScrollBarSubLine? MT_ACTIVE : 0),
                  subline.x(), subline.y(), subline.width(), subline.height(),
                  &data);

            state &= ~MT_HOVER;
         }

         if ((controls & SC_ScrollBarAddLine) && addline.isValid()) {
            if (hoverWidget == widget && hoverPart == 3 && !scrollbar->draggingSlider()) state |= MT_HOVER;

            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL,
                  (scrollbar->orientation() == TQt::Horizontal)? MT_SCROLLBAR_ARROW_RIGHT : MT_SCROLLBAR_ARROW_DOWN,
                  state | (active == SC_ScrollBarAddLine? MT_ACTIVE : 0),
                  addline.x(), addline.y(), addline.width(), addline.height(),
                  &data);

            state &= ~MT_HOVER;
         }

         if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
            data.orientation = (scrollbar->orientation() == TQt::Horizontal)? MT_HORIZONTAL : MT_VERTICAL;

            if (data.flags & MT_SCROLLBAR_UNSCROLLABLE) {
               TQRegion region = slider;
               p->save();
               p->setClipping(true);
               p->setClipRegion(region);
               mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL,
                     MT_SCROLLBAR,
                     state | (active == SC_ScrollBarSlider? MT_ACTIVE : 0),
                     widget->rect().x(), widget->rect().y(), widget->rect().width(), widget->rect().height(),
                     &data);
               p->restore();
               return;
            }

            if ((hoverWidget == widget && hoverPart == 1) || scrollbar->draggingSlider()) state |= MT_HOVER;

            if (data.orientation == MT_VERTICAL) {
               data.handle_position = slider.y() - subpage.y();
               data.groove_size = subpage.height() + slider.height() + addpage.height();
            }
            else {
               data.handle_position = slider.x() - subpage.x();
               data.groove_size = subpage.width() + slider.width() + addpage.width();
            }

            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL,
                  MT_SCROLLBAR_HANDLE,
                  state | (active == SC_ScrollBarSlider? MT_ACTIVE : 0),
                  slider.x(), slider.y(), slider.width(), slider.height(),
                  &data);

            state &= ~MT_HOVER;
         }

         if (((controls & SC_ScrollBarSubPage) && subpage.isValid()) ||
             ((controls & SC_ScrollBarAddPage) && addpage.isValid())) {
            TQRegion region;
            p->save();
            p->setClipping(true);
            if (controls & SC_ScrollBarSubPage) region = region.unite(subpage);
            if (controls & SC_ScrollBarAddPage) region = region.unite(addpage);
            p->setClipRegion(region);

            MT_WIDGET_DATA data;
            data.widget = (MT_WIDGET)widget;
            data.orientation = (scrollbar->orientation() == TQt::Horizontal)? MT_HORIZONTAL : MT_VERTICAL;

            if (active == SC_ScrollBarSubPage) data.flags |= MT_SCROLLBAR_SUBPAGE_ACTIVE;
            if (active == SC_ScrollBarAddPage) data.flags |= MT_SCROLLBAR_ADDPAGE_ACTIVE;

            if (data.flags & (MT_SCROLLBAR_SUBPAGE_ACTIVE | MT_SCROLLBAR_ADDPAGE_ACTIVE)) {
               if (data.orientation == MT_VERTICAL) {
                  data.handle_position = slider.y() - subpage.y();
                  data.groove_size = slider.height();
               }
               else {
                  data.handle_position = slider.x() - subpage.x();
                  data.groove_size = slider.width();
               }
            }

            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL,
                  MT_SCROLLBAR,
                  state,
                  widget->rect().x(),
                  widget->rect().y(),
                  widget->rect().width(),
                  widget->rect().height(),
                  &data);
            p->restore();
         }
         break;
      }

      case CC_SpinWidget:
      {
         const TQSpinWidget *sw = (const TQSpinWidget *)widget;
         SFlags flags;
         PrimitiveElement pe;

         if (controls & SC_SpinWidgetFrame) {
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_SPINBUTTON, state, r.x(), r.y(), r.width(), r.height(), &data);
         }

         if (controls & SC_SpinWidgetUp) {
            flags = Style_Default;
            if (hoverWidget == widget) flags |= Style_MouseOver;
            if (active == SC_SpinWidgetUp) {
               flags |= Style_On;
               flags |= Style_Sunken;
            }
            else {
               flags |= Style_Raised;
            }
            if (sw->isUpEnabled()) flags |= Style_Enabled;

            if (sw->buttonSymbols() == TQSpinWidget::PlusMinus)
               pe = PE_SpinWidgetPlus;
            else
               pe = PE_SpinWidgetUp;

            TQRect re = sw->upRect();
            TQColorGroup ucg = sw->isUpEnabled() ? cg : sw->palette().disabled();
            drawPrimitive(pe, p, re, ucg, flags);
         }

         if (controls & SC_SpinWidgetDown) {
            flags = Style_Default;
            if (hoverWidget == widget) flags |= Style_MouseOver;
            if (active == SC_SpinWidgetDown ) {
               flags |= Style_On;
               flags |= Style_Sunken;
            }
            else {
               flags |= Style_Raised;
            }
            if (sw->isDownEnabled()) flags |= Style_Enabled;

            if ( sw->buttonSymbols() == TQSpinWidget::PlusMinus )
               pe = PE_SpinWidgetMinus;
            else
               pe = PE_SpinWidgetDown;

            TQRect re = sw->downRect();
            TQColorGroup dcg = sw->isDownEnabled() ? cg : sw->palette().disabled();
            drawPrimitive(pe, p, re, dcg, flags);
         }
         break;
      }

      case CC_ComboBox:
      {
         TQRect re;
         if (hoverWidget == widget) state |= MT_HOVER;

         p->save();
         
         if (controls & SC_ComboBoxFrame) {
            re = querySubControlMetrics(control, widget, SC_ComboBoxFrame, opt);
            p->setClipRect(re);
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_CHOICE, state, re.x(), re.y(), re.width(), re.height(), &data);
         }
         if (controls & SC_ComboBoxEditField) {
            re = querySubControlMetrics(control, widget, SC_ComboBoxEditField, opt);
         }
         if (controls & SC_ComboBoxArrow) {
            re = querySubControlMetrics(control, widget, SC_ComboBoxArrow, opt);
            p->setClipRect(re);
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_CHOICE_BUTTON, state, re.x(), re.y(), re.width(), re.height(), &data);
         }

         p->restore();
         break;
      }

      case CC_ToolButton:
      {
         const TQToolButton *toolbutton = (const TQToolButton *) widget;

         TQRect button, menuarea;
         button   = visualRect(querySubControlMetrics(control, widget, SC_ToolButton, opt), widget);
         menuarea = visualRect(querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt), widget);

         if (toolbutton->parent() && toolbutton->parent()->inherits("TQTabBar")) {
            type = (opt.arrowType() == TQt::LeftArrow)? MT_NOTEBOOK_ARROW_LEFT : MT_NOTEBOOK_ARROW_RIGHT;
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, type, state, button.x(), button.y(), button.width(), button.height(), &data);
            tabBarButton = true;
            break;
         }

         TQColorGroup c = cg;
         if (toolbutton->backgroundMode() != PaletteButton) {
            c.setBrush(TQColorGroup::Button, toolbutton->paletteBackgroundColor());
         }

         SFlags bflags = flags,
         mflags = flags;

         if (active & SC_ToolButton) bflags |= Style_Down;
         if (active & SC_ToolButtonMenu) mflags |= Style_Down;

         if (controls & SC_ToolButton) {
            if (toolbutton->parentWidget() && toolbutton->parentWidget()->backgroundPixmap() &&
               !toolbutton->parentWidget()->backgroundPixmap()->isNull()) {
               TQPixmap pixmap = *(toolbutton->parentWidget()->backgroundPixmap());
               p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
            }

            if ((flags & Style_Raised) && !(flags & Style_Down)) state |= MT_HOVER;

            type = MT_TOOLBAR_ITEM;
            if (toolbutton->isToggleButton()) {
               type = MT_TOOLBAR_ITEM_TOGGLE;
               if (flags & Style_On) state |= MT_ACTIVE;
            }

#ifdef HAVE_KDE
            if (toolbutton->inherits("KToolBarButton") && toolbutton->popup()) {
               type = MT_TOOLBAR_ITEM_DROPDOWN;
               data.flags = 0;
               if (widget == toolButtonDropDownActiveWidget) {
                  data.flags |= MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE;
               }
            }
#endif

            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, type, state, button.x(), button.y(), button.width(), button.height(), &data);
         }

         /*
         if (controls & SC_ToolButtonMenu) {
            if (mflags & (Style_Down | Style_On | Style_Raised)) {
               drawPrimitive(PE_ButtonDropDown, p, menuarea, c, mflags, opt);
            }
            drawPrimitive(PE_ArrowDown, p, menuarea, c, mflags, opt);
         }
         */

         if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
            TQRect fr = toolbutton->rect();
            fr.addCoords(3, 3, -3, -3);
            drawPrimitive(PE_FocusRect, p, fr, c);
         }

#ifdef HAVE_KDE
         // hack:
         if (toolbutton->inherits("KToolBarButton") && toolbutton->popup()) {
            const TQObject *parent = toolbutton->parent();

            if (parent && parent->inherits("KToolBar") && static_cast<const KToolBar *>(parent)->iconText() == KToolBar::IconTextRight) {
               /* nothing */
            }
            else {
               p->translate(- (mt_engine->metric[MT_TOOLBAR_ITEM_DROPDOWN_WIDTH] / 2), 0);
            }

            if ((state & MT_ACTIVE) && widget == toolButtonDropDownActiveWidget) {
               p->translate(-mt_engine->metric_size[MT_TOOLBAR_ITEM_TEXT_OFFSET].x, -mt_engine->metric_size[MT_TOOLBAR_ITEM_TEXT_OFFSET].y);
            }
            toolButtonPopup = true;
         }

         // hack:
         if (toolbutton->inherits("KToolBarButton") && (flags & Style_Down)) {
            p->translate(mt_engine->metric_size[MT_TOOLBAR_ITEM_TEXT_OFFSET].x - 1, mt_engine->metric_size[MT_TOOLBAR_ITEM_TEXT_OFFSET].y - 1);
         }
#endif

         // hack:
         if (toolbutton->isToggleButton() && (flags & Style_On) && !(flags & Style_Down)) {
            p->translate(mt_engine->metric_size[MT_TOOLBAR_ITEM_TEXT_OFFSET].x, mt_engine->metric_size[MT_TOOLBAR_ITEM_TEXT_OFFSET].y);
         }
         break;
      }

      case CC_Slider:
      {
         const TQSlider *sl = (const TQSlider *)widget;
         TQRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove, opt);
         TQRect handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle, opt);

         data.orientation = (sl->orientation() == Horizontal)? MT_HORIZONTAL : MT_VERTICAL;

         if (controls & SC_SliderGroove) {
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_SCALE, state, groove.x(), groove.y(), groove.width(), groove.height(), &data);
         }

         if (controls & SC_SliderTickmarks) {
            TQCommonStyle::drawComplexControl(control, p, widget, r, cg, flags, SC_SliderTickmarks, active, opt);
         }

         if (controls & SC_SliderHandle) {
            mt_engine->draw_widget(mt_engine, (MT_WINDOW *)p, NULL, MT_SCALE_HANDLE, state, handle.x(), handle.y(), handle.width(), handle.height(), &data);
         }
         break;
      }

      default:
         KStyle::drawComplexControl(control, p, widget, r, cg, flags, controls, active, opt);
   }
}


void MetaThemeStyle::drawItem(TQPainter *p,
                              const TQRect &r,
                              int flags,
                              const TQColorGroup &g,
                              bool enabled,
                              const TQPixmap *pixmap,
                              const TQString& text,
                              int len,
                              const TQColor *penColor) const
{
   static bool alreadyHere = false;

   if (!alreadyHere && !pixmap && !text.isNull()) {
      int state = 0;
      MT_QT_STRING s = {this, r, flags, g, text, len, penColor};

      if (stringType != 0) state = stringState;

      if (!enabled) state |= MT_DISABLED;

      alreadyHere = true;
      mt_engine->draw_string(mt_engine, (MT_WINDOW *)p, stringType, state, &s);
      alreadyHere = false;

      stringType = 0;
   }
   else {
      KStyle::drawItem(p, r, flags, g, enabled, pixmap, text, len, penColor);
   }
}


TQRect MetaThemeStyle::subRect(SubRect r, const TQWidget *widget) const
{
   switch (r) {
      case SR_ProgressBarGroove:
         return TQRect(widget->rect());

      case SR_ProgressBarContents:
      case SR_ProgressBarLabel:
      {
         TQRect rw = widget->rect();
         int x = mt_engine->metric_size[MT_BORDER].x;
         int y = mt_engine->metric_size[MT_BORDER].y;
         return TQRect(rw.left()+x, rw.top()+y, rw.width()-x*2, rw.height()-y*2);
      }

      default:
         return KStyle::subRect(r, widget);
   }
}


TQRect MetaThemeStyle::querySubControlMetrics(ComplexControl control,
                                          const TQWidget *widget,
                                          SubControl subcontrol,
                                          const TQStyleOption &opt) const
{
   if (!widget) {
      return TQRect();
   }

   TQRect r(widget->rect());

   switch (control) {
      case CC_SpinWidget:
      {
         int bw = 14, bh=21, by = (r.height() - bh) / 2;

         switch (subcontrol) {
            case SC_SpinWidgetUp:
               return TQRect(r.width() - bw, by, bw, bh / 2);

            case SC_SpinWidgetDown:
               return TQRect(r.width() - bw, by + bh / 2, bw, bh / 2 + 1);

            case SC_SpinWidgetFrame:
               return TQRect(0, 0, r.width() - bw, r.height());

            case SC_SpinWidgetEditField:
               return TQRect(2, 2, r.width() - bw - 4, r.height() - 4);

            case SC_SpinWidgetButtonField:
               return TQRect(0, 0, 0, 0);
         }
         break;
      }

      case CC_ComboBox:
      {
         switch (subcontrol) {
            case SC_ComboBoxFrame:
               return TQRect(r.left(), r.top(), r.width()-mt_engine->metric[MT_CHOICE_BUTTON_WIDTH], r.height());

            case SC_ComboBoxEditField:
               // TODO: remove this silly hack:
               if (!strcmp(mt_engine->name, "highcolor")) {
                  return TQRect(r.left()+3, r.top()+3, r.width()-6-mt_engine->metric[MT_CHOICE_BUTTON_WIDTH], r.height()-6);
               }
               return TQRect(r.left()+2, r.top()+2, r.width()-4-mt_engine->metric[MT_CHOICE_BUTTON_WIDTH], r.height()-4 - (!strcmp(mt_engine->name, "redmond")? 1:0) );

            case SC_ComboBoxArrow:
               return TQRect(r.width()-mt_engine->metric[MT_CHOICE_BUTTON_WIDTH], r.top(), mt_engine->metric[MT_CHOICE_BUTTON_WIDTH], r.height());

            case SC_ComboBoxListBoxPopup:
                return opt.rect();
         }
      }
   }

   return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
}


int MetaThemeStyle::pixelMetric(PixelMetric m, const TQWidget *widget) const
{
   switch (m) {
      case PM_ButtonMargin: return 0; /* MT_BUTTON_BORDER: real value is returned by sizeFromContents method */
      case PM_ButtonDefaultIndicator: return 0;

      case PM_MenuButtonIndicator: // width of the menu button indicator proportional to the widget height.
         return 0;

      case PM_ButtonShiftHorizontal: // horizontal contents shift of a button when the button is down.
      case PM_ButtonShiftVertical: // vertical contents shift of a button when the button is down.
      {
         int i = MT_BUTTON_TEXT_OFFSET;

         if (widget && widget->inherits("TQPushButton") && ((TQPushButton *)widget)->isToggleButton()) {
            i = MT_BUTTON_TOGGLE_TEXT_OFFSET;
         }
         else if (widget && widget->inherits("TQToolButton")) {
            i = MT_TOOLBAR_ITEM_TEXT_OFFSET;
         }

         return (m == PM_ButtonShiftHorizontal)? mt_engine->metric_size[i].x : mt_engine->metric_size[i].y;
      }

      case PM_DefaultFrameWidth: // default frame width, usually 2.
         if (widget && !qstrcmp(widget->name(), "tab pages")) return 2;
         return mt_engine->metric[MT_BORDER];

      case PM_SpinBoxFrameWidth: // frame width of a spin box.
         return mt_engine->metric[MT_BORDER]; // TODO

#if TQT_VERSION >= 0x030200
      case PM_MDIFrameWidth: // frame width of an MDI window.
         return mt_engine->metric[MT_BORDER]; // TODO

      case PM_MDIMinimizedWidth: // width of a minimized MSI window.
         return 120; // TODO
#endif

      case PM_MaximumDragDistance: // Some feels require the scrollbar or other sliders to jump back to the original position when the mouse pointer is too far away while dragging. A value of -1 disables this behavior.
         return -1;

      case PM_ScrollBarExtent: // width of a vertical scrollbar and the height of a horizontal scrollbar.
         return mt_engine->metric[MT_SCROLLBAR_SIZE];

      case PM_ScrollBarSliderMin: // the minimum height of a vertical scrollbar's slider and the minimum width of a horiztonal scrollbar slider.
         return 20; // TODO: add new metric type

      // TODO:
      case PM_SliderThickness: // total slider thickness.
      case PM_SliderControlThickness: // thickness of the slider handle.
         break;

      case PM_SliderLength: // length of the slider.
         return mt_engine->metric[MT_SCALE_SLIDER_SIZE];

      // TODO:
      case PM_SliderTickmarkOffset: // the offset between the tickmarks and the slider.
      case PM_SliderSpaceAvailable: // the available space for the slider to move.
         break;

      case PM_DockWindowSeparatorExtent: // width of a separator in a horiztonal dock window and the height of a separator in a vertical dock window.
         break; // TODO

      // TODO:
      case PM_DockWindowHandleExtent: // width of the handle in a horizontal dock window and the height of the handle in a vertical dock window.
         return 10;

      // TODO:
      case PM_DockWindowFrameWidth: // frame width of a dock window.
         break;

      case PM_MenuBarFrameWidth: // frame width of a menubar.
         return TQMAX(mt_engine->metric_size[MT_MENUBAR_BORDER].x, mt_engine->metric_size[MT_MENUBAR_BORDER].y); // TODO: handle by sizeFromContents, how?

#if TQT_VERSION >= 0x030300
      case PM_MenuBarItemSpacing: // spacing between menubar items.
         return mt_engine->metric[MT_MENUBAR_ITEM_SPACING];

      case PM_ToolBarItemSpacing: // spacing between toolbar items.
         return 0;
#endif

      case PM_TabBarTabOverlap: // number of pixels the tabs should overlap.
         return mt_engine->metric[MT_NOTEBOOK_TAB_OVERLAP];

      case PM_TabBarTabHSpace: // extra space added to the tab width.
         return 24;

      case PM_TabBarTabVSpace: // extra space added to the tab height.
         return 10;

      case PM_TabBarBaseHeight: // height of the area between the tab bar and the tab pages.
         return 0;

      case PM_TabBarBaseOverlap: // number of pixels the tab bar overlaps the tab bar base.
         return mt_engine->metric[MT_NOTEBOOK_OVERLAP] - 2;

#if TQT_VERSION >= 0x030300
      case PM_TabBarScrollButtonWidth:
         return mt_engine->metric[MT_NOTEBOOK_ARROW_WIDTH];
#endif

#if TQT_VERSION >= 0x030200
      case PM_TabBarTabShiftHorizontal: // horizontal pixel shift when a tab is selected.
         return 0;

      case PM_TabBarTabShiftVertical: // vertical pixel shift when a tab is selected.
      {
         if (widget) {
            const TQTabBar *tb = static_cast<const TQTabBar*>(widget);
            if (tb->shape() == TQTabBar::RoundedBelow || tb->shape() == TQTabBar::TriangularBelow) {
               return -mt_engine->metric[MT_NOTEBOOK_TEXT_OFFSET];
            }
         }
         return mt_engine->metric[MT_NOTEBOOK_TEXT_OFFSET];
      }
#endif

      case PM_ProgressBarChunkWidth: // width of a chunk in a progress bar indicator.
         break;

      case PM_SplitterWidth: // width of a splitter.
         return mt_engine->metric[MT_SPLITTER_WIDTH];

      case PM_TitleBarHeight: // height of the title bar.
         break;

#if TQT_VERSION >= 0x030200
      case PM_PopupMenuFrameHorizontalExtra: // additional border, e.g. for panels
         return mt_engine->metric_size[MT_MENU_BORDER].x - mt_engine->metric[MT_BORDER];

      case PM_PopupMenuFrameVerticalExtra: // additional border, e.g. for panels
         return mt_engine->metric_size[MT_MENU_BORDER].y - mt_engine->metric[MT_BORDER];
#endif

      case PM_IndicatorWidth: // width of a check box indicator.
      case PM_IndicatorHeight: // height of a checkbox indicator.
         return mt_engine->metric[MT_CHECKBOX_SIZE];

      case PM_ExclusiveIndicatorWidth: // width of a radio button indicator.
      case PM_ExclusiveIndicatorHeight: // height of a radio button indicator.
         return mt_engine->metric[MT_CHECKBOX_SIZE]; // TODO

      case PM_PopupMenuScrollerHeight: // height of the scroller area in a popupmenu.
         break;

      case PM_CheckListButtonSize: // area (width/height) of the checkbox/radiobutton in a TQCheckListItem
         return mt_engine->metric[MT_CHECKBOX_SIZE];

#if TQT_VERSION >= 0x030200
      case PM_CheckListControllerSize: // area (width/height) of the controller in a TQCheckListItem
         break;

      case PM_DialogButtonsSeparator: // distance between buttons in a dialog buttons widget.
         break;

      case PM_DialogButtonsButtonWidth: // minimum width of a button in a dialog buttons widget.
         break;

      case PM_DialogButtonsButtonHeight: // minimum height of a button in a dialog buttons widget.
         break;

      case PM_HeaderMarkSize:
      case PM_HeaderGripMargin:
      case PM_HeaderMargin:
         break;
#endif
   }

   return KStyle::pixelMetric(m, widget);
}


TQSize MetaThemeStyle::sizeFromContents(ContentsType t,
                                    const TQWidget *widget,
                                    const TQSize &s,
                                    const TQStyleOption &opt) const
{
   switch (t) {

      case CT_PushButton:
      {
         const TQPushButton *button = (const TQPushButton *)widget;
         int w = s.width(), h = s.height();
         int fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
         int m = button->isToggleButton()? MT_BUTTON_TOGGLE_BORDER : MT_BUTTON_BORDER;

         w += mt_engine->metric_size[m].x*2 + fw;
         h += mt_engine->metric_size[m].y*2 + fw;

         if (w < 80 && !button->pixmap()) w = 80;
         if (h < 23) h = 23;

         return TQSize(w, h);
      }

      case CT_PopupMenuItem:
      {
         if (!widget || opt.isDefault()) break;

         const TQPopupMenu *popup = (const TQPopupMenu *)widget;
         TQMenuItem *mi = opt.menuItem();
         int maxpmw = opt.maxIconWidth();
         int w = s.width(), h = s.height();

         if (mi->custom()) {
            return mi->custom()->sizeHint();
         }
         else if (mi->widget()) {
            return s;
         }

         if (maxpmw == 0) {
            w += 16;
         }
         else if (maxpmw < 16) {
            maxpmw = 16 - maxpmw;
         }

         w += maxpmw + 3*3 + 2*mt_engine->metric_size[MT_MENU_ITEM_BORDER].x + 8;

         if (mi->iconSet()) {
            TQPixmap pixmap = mi->iconSet()->pixmap(TQIconSet::Small, TQIconSet::Normal, TQIconSet::On);
            h = pixmap.height();
            if (h <= 18) h -= 3;
         }
         else {
            h = 0;
         }

         h = TQMAX(h, popup->fontMetrics().height() + 2);
         h += 2*mt_engine->metric_size[MT_MENU_ITEM_BORDER].y;

         if (mi->isSeparator()) {
            h = mt_engine->metric[MT_MENU_SEPARATOR_HEIGHT];
         }

         return TQSize(w, h);
      }

      case CT_SpinBox:
      {
         TQSize ret = KStyle::sizeFromContents (t, widget, s, opt);
         ret.setHeight(TQMAX(ret.height(), 21));
         return ret;
      }

      case CT_ToolButton:
      {
         const TQToolButton *button = (const TQToolButton *)widget;
         int w = s.width(), h = s.height();

         /*
         if (button->parent() && button->parent()->inherits("TQTabBar")) {
            w = 50;
            h = 50;
         }
         */

         w += mt_engine->metric_size[MT_TOOLBAR_ITEM_BORDER].x*2;
         h += mt_engine->metric_size[MT_TOOLBAR_ITEM_BORDER].y*2;

#ifdef HAVE_KDE
         if (widget->inherits("KToolBarButton") && button->popup()) {
            w += mt_engine->metric[MT_TOOLBAR_ITEM_DROPDOWN_WIDTH];
         }

         // additional space for buttons with text in right of icon:
         if (widget->inherits("KToolBarButton")) {
            const TQObject *parent = button->parent();
            if (parent && parent->inherits("KToolBar") && static_cast<const KToolBar *>(parent)->iconText() == KToolBar::IconTextRight) {
               w += 3;
            }
         }
#endif

         return TQSize(w, h);
      }
   }

   return KStyle::sizeFromContents (t, widget, s, opt);
}


int MetaThemeStyle::styleHint(StyleHint stylehint, const TQWidget *widget, const TQStyleOption &opt, TQStyleHintReturn *returnData) const
{
   switch (stylehint) {

      case SH_EtchDisabledText: // disabled text is "etched" like Windows.
         return 1;

      case SH_GUIStyle: // the GUI style to use.
         return TQt::WindowsStyle;

      case SH_ScrollBar_BackgroundMode: // the background mode for a TQScrollBar. Possible values are any of those in the BackgroundMode enum.
         return TQt::NoBackground;

      case SH_ScrollBar_MiddleClickAbsolutePosition: // a boolean value. If TRUE, middle clicking on a scrollbar causes the slider to jump to that position. If FALSE, the middle clicking is ignored.
         return 1;

#if TQT_VERSION >= 0x030200
      case SH_ScrollBar_LeftClickAbsolutePosition: // a boolean value. If TRUE, left clicking on a scrollbar causes the slider to jump to that position. If FALSE, the left clicking will behave as appropriate for each control.
         return 0;
#endif

      case SH_ScrollBar_ScrollWhenPointerLeavesControl: // a boolean value. If TRUE, when clicking a scrollbar SubControl, holding the mouse button down and moving the pointer outside the SubControl, the scrollbar continues to scroll. If FALSE, the scollbar stops scrolling when the pointer leaves the SubControl.
         return 1;

      case SH_TabBar_Alignment: // the alignment for tabs in a TQTabWidget. Possible values are TQt::AlignLeft, TQt::AlignCenter and TQt::AlignRight.
         return TQt::AlignLeft;

      case SH_Header_ArrowAlignment: // the placement of the sorting indicator may appear in list or table headers. Possible values are TQt::Left or TQt::Right.
         return TQt::Right;

      case SH_Slider_SnapToValue: // sliders snap to values while moving, like Windows
         return 1;

      case SH_Slider_SloppyKeyEvents: // key presses handled in a sloppy manner, i.e. left on a vertical slider subtracts a line.
         return 1;

      case SH_ProgressDialog_CenterCancelButton: // center button on progress dialogs, like Motif, otherwise right aligned.
         return 0;

      case SH_ProgressDialog_TextLabelAlignment: // TQt::AlignmentFlags -- text label alignment in progress dialogs; Center on windows, Auto|VCenter otherwise.
         return TQt::AlignCenter;

      case SH_PrintDialog_RightAlignButtons: // right align buttons in the print dialog, like Windows.
         return 1;

      case SH_MainWindow_SpaceBelowMenuBar: // 1 or 2 pixel space between the menubar and the dockarea, like Windows.
         return 0;

      case SH_FontDialog_SelectAssociatedText: // select the text in the line edit, or when selecting an item from the listbox, or when the line edit receives focus, like Windows.
         return 1;

      case SH_PopupMenu_AllowActiveAndDisabled: // allows disabled menu items to be active.
         return 0;

      case SH_PopupMenu_SpaceActivatesItem: // pressing Space activates the item, like Motif.
         return 0;

      case SH_PopupMenu_SubMenuPopupDelay: // the number of milliseconds to wait before opening a submenu; 256 on windows, 96 on Motif.
         return 256;

      case SH_PopupMenu_Scrollable: // whether popupmenu's must support scrolling.
         return 0;

      case SH_PopupMenu_SloppySubMenus: // whether popupmenu's must support sloppy submenu; as implemented on Mac OS.
         return 0;

      case SH_ScrollView_FrameOnlyAroundContents: // whether scrollviews draw their frame only around contents (like Motif), or around contents, scrollbars and corner widgets (like Windows).
         return 0;

      case SH_MenuBar_AltKeyNavigation: // menubars items are navigable by pressing Alt, followed by using the arrow keys to select the desired item.
         return 1;

      case SH_ComboBox_ListMouseTracking: // mouse tracking in combobox dropdown lists.
         return 1;

      case SH_PopupMenu_MouseTracking: // mouse tracking in popup menus.
         return 1;

      case SH_MenuBar_MouseTracking: // mouse tracking in menubars.
         return 1;

      case SH_ItemView_ChangeHighlightOnFocus: // gray out selected items when losing focus.
         return 1; // TODO

      case SH_Widget_ShareActivation: // turn on sharing activation with floating modeless dialogs.
         return 0; // TODO

      case SH_TabBar_SelectMouseType: // which type of mouse event should cause a tab to be selected.
         return TQEvent::MouseButtonPress;

#if TQT_VERSION >= 0x030200
      case SH_ListViewExpand_SelectMouseType: // which type of mouse event should cause a listview expansion to be selected.
         return TQEvent::MouseButtonPress;

      case SH_TabBar_PreferNoArrows: // whether a tabbar should suggest a size to prevent scoll arrows.
         return 0; // TODO
#endif

      case SH_ComboBox_Popup: // allows popups as a combobox dropdown menu.
         return 0;

      case SH_Workspace_FillSpaceOnMaximize: // the workspace should maximize the client area.
         return 0;

      case SH_TitleBar_NoBorder: // the titlebar has no border
         return 0;

      case SH_ScrollBar_StopMouseOverSlider: // stops autorepeat when slider reaches mouse
         return 0;

      case SH_BlinkCursorWhenTextSelected: // whether cursor should blink when text is selected
         return 1;

      case SH_RichText_FullWidthSelection: // whether richtext selections should extend the full width of the document.
         return 0;

      case SH_GroupBox_TextLabelVerticalAlignment: // how to vertically align a groupbox's text label.
         return TQt::AlignVCenter;

      case SH_GroupBox_TextLabelColor: // how to paint a groupbox's text label.
         return (int)(widget? widget->paletteForegroundColor().rgb() : 0);

#if TQT_VERSION >= 0x030200
      case SH_DialogButtons_DefaultButton: // which buttons gets the default status in a dialog's button widget.
         return 0x01/*TQDialogButtons::Accept*/;
#endif

#if TQT_VERSION >= 0x030300
      case SH_ToolButton_Uses3D: // indicates whether TQToolButtons should use a 3D frame when the mouse is over them
         return 1;
#endif

#if TQT_VERSION >= 0x030200
      case SH_ToolBox_SelectedPageTitleBold: // Boldness of the selected page title in a TQToolBox.
         return 0;

      case SH_LineEdit_PasswordCharacter: // The TQChar Unicode character to be used for passwords.
         return '*';

      case SH_Table_GridLineColor:
         return (int)(widget? widget->colorGroup().mid().rgb() : 0);
#endif

#if TQT_VERSION >= 0x030300
      case SH_UnderlineAccelerator: // whether accelerators are underlined
         return 1;
#endif
   }

   return KStyle::styleHint(stylehint, widget, opt, returnData);
}


void MetaThemeStyle::updateFont()
{
   setFont();
}


bool MetaThemeStyle::eventFilter(TQObject *obj, TQEvent *ev)
{
   if (KStyle::eventFilter(obj, ev)) return true;

   if (!obj->isWidgetType()) return false;

   if (mt_engine->metric[MT_NOTEBOOK_IS_FILLED] && ev->type() == TQEvent::Paint) {
      TQWidget *widget = static_cast<TQWidget*>(obj);
      TQWidget *w = tabWidgets.find(obj);
      if (w) {
         MT_WIDGET_DATA data;
         int state = 0;
         data.widget = (MT_WIDGET)widget;

         TQPainter p;
         TQPixmap *pixmap = NULL;

         if (widget->inherits("TQButton") && (widget->backgroundMode() != TQt::FixedPixmap || widget->paletteBackgroundPixmap()->size() != widget->size())) {
            pixmap = new TQPixmap(widget->size());
            p.begin(pixmap);
         }
         else {
            p.begin(widget);
            TQRegion region = static_cast<TQPaintEvent*>(ev)->region();
            p.setClipRegion(region);
         }

         TQPoint pos = widget->mapFromGlobal(w->mapToGlobal(TQPoint(0, 0)));
         mt_engine->draw_widget(mt_engine, (MT_WINDOW *)(&p), NULL, MT_NOTEBOOK, state, pos.x(), pos.y(), w->width(), w->height(), &data);

         p.end();

         if (pixmap) {
            widget->setBackgroundPixmap(*pixmap);
            widget->setBackgroundMode(TQt::FixedPixmap);
            delete pixmap;
            return true;
         }
      }
   }

   if (obj->inherits("TQPushButton")) {
      if (ev->type() == TQEvent::MouseButtonPress) {
         static_cast<TQWidget*>(obj)->setFocus();
      }
   }

   if (obj->inherits("TQPushButton") || obj->inherits("TQComboBox") || obj->inherits("TQSpinWidget") ||
       obj->inherits("TQSpinWidget") || obj->inherits("TQCheckBox") ||
       obj->inherits("TQRadioButton") || obj->inherits("TQScrollBar")) {

      if ((ev->type() == TQEvent::Enter) && static_cast<TQWidget*>(obj)->isEnabled()) {
         TQWidget* button = static_cast<TQWidget*>(obj);
         hoverWidget = button;
         button->repaint(false);
      }
      else if ((ev->type() == TQEvent::Leave) && (obj == hoverWidget)) {
         TQWidget* button = static_cast<TQWidget*>(obj);
         hoverWidget = 0;
         hoverPart = 0;
         button->repaint(false);
      }
   }

   if (obj->inherits("TQScrollBar") && (ev->type() == TQEvent::MouseMove || ev->type() == TQEvent::MouseButtonRelease)) {
      TQScrollBar *scrollbar = static_cast<TQScrollBar*>(obj);
      TQMouseEvent *mev = static_cast<TQMouseEvent*>(ev);
      TQRect subline, addline, slider;
      int oldPart = hoverPart;

      if (ev->type() == TQEvent::MouseButtonRelease) {
         oldPart = 0;
      }

      subline = querySubControlMetrics(CC_ScrollBar, scrollbar, SC_ScrollBarSubLine);
      addline = querySubControlMetrics(CC_ScrollBar, scrollbar, SC_ScrollBarAddLine);
      slider  = querySubControlMetrics(CC_ScrollBar, scrollbar, SC_ScrollBarSlider);

      hoverPart = 0;
      if (slider.contains(mev->pos())) {
         hoverPart = 1;
      }
      else if (subline.contains(mev->pos())) {
         hoverPart = 2;
      }
      else if (addline.contains(mev->pos())) {
         hoverPart = 3;
      }

      if ((hoverWidget != scrollbar || hoverPart != oldPart) && (!scrollbar->draggingSlider() || ev->type() == TQEvent::MouseButtonRelease)) {
         scrollbar->update();
      }

      hoverWidget = scrollbar;

      return false;
   }

#ifdef HAVE_KDE
   if (obj->inherits("KToolBarButton")) {
      KToolBarButton* button = static_cast<KToolBarButton*>(obj);

      if (ev->type() == TQEvent::MouseButtonPress && button->popup()) {
         if (static_cast<TQMouseEvent*>(ev)->x() >= button->width() - mt_engine->metric[MT_TOOLBAR_ITEM_DROPDOWN_WIDTH]) {
            toolButtonDropDownActiveWidget = static_cast<TQWidget*>(obj);
            button->setPopupDelay(1);
         }
         else {
            button->setPopupDelay(0);
         }
      }
      else if (ev->type() == TQEvent::MouseButtonRelease && button->popup()) {
         button->setPopupDelay(0);
         toolButtonDropDownActiveWidget = NULL;
      }
   }
#endif

   if ((obj->inherits("TQLineEdit") || obj->inherits("TQComboBox")) && ev->type() == TQEvent::Paint) {
      TQWidget *le = static_cast<TQWidget*>(obj);

      const TQPixmap *old = le->backgroundPixmap();
      if (!old || old->size() != le->size()) {
         TQPixmap pixmap(le->size());
         TQPainter p(&pixmap);
         int state = MT_NORMAL;
         MT_WIDGET_DATA data;
         data.widget = (MT_WIDGET)le;

         if (!le->isEnabled()) state |= MT_DISABLED;
         if (!le->hasFocus()) state |= MT_SELECTED;

         p.setPen(le->colorGroup().base());
         p.setBrush(le->colorGroup().base());
         p.drawRect(0, 0, le->width(), le->height());

         retrieveColors((const TQWidget *)le, 0, le->colorGroup(), &data);
         mt_engine->draw_widget(mt_engine, (MT_WINDOW *)(&p), NULL, MT_ENTRY, state, 0, 0, le->width(), le->height(), &data);

         le->setBackgroundPixmap(pixmap);
      }
   }

   if (!qstrcmp(obj->name(), "kde toolbar widget")) {
      TQWidget* lb = static_cast<TQWidget*>(obj);
      if (lb->backgroundMode() == TQt::PaletteButton) lb->setBackgroundMode(TQt::PaletteBackground);
      lb->removeEventFilter(this);
   }

   if (obj->inherits("TQFrame") && !obj->inherits("TEWidget") && ev->type() == TQEvent::Paint) {
      TQFrame *f = static_cast<TQFrame*>(obj);
      TQPaintEvent *pev = static_cast<TQPaintEvent*>(ev);
      static bool alreadyHere = false;

      if (alreadyHere) {
         return false;
      }

      if (f->frameShape() != TQFrame::HLine && f->frameShape() != TQFrame::VLine && f->frameShape() != TQFrame::Panel && f->frameShape() != TQFrame::WinPanel) {
         return false;
      }

      if ((f->frameShape() == TQFrame::Panel || f->frameShape() == TQFrame::WinPanel) && f->lineWidth() == 1) {
         return false;
      }

      TQRegion region = pev->region().intersect(f->contentsRect());
      TQPaintEvent fake(region, pev->erased());

      alreadyHere = true;
      TQApplication::sendEvent(f, &fake);
      alreadyHere = false;

      int type;
      switch (f->frameShape()) {
         case TQFrame::HLine: type = MT_HLINE; break;
         case TQFrame::VLine: type = MT_VLINE; break;

         case TQFrame::Panel:
         case TQFrame::WinPanel:
            type = (f->frameShadow() == TQFrame::Raised)? MT_BORDER_OUT : MT_BORDER_IN;
            break;

         default: return true;
      }

      TQPainter p(f);
      TQRect r(f->frameRect());
      int state = MT_NORMAL;
      MT_WIDGET_DATA data;
      data.widget = (MT_WIDGET)f;

      retrieveColors((const TQWidget *)f, 0, f->colorGroup(), &data);
      mt_engine->draw_widget(mt_engine, (MT_WINDOW *)(&p), NULL, type, state, r.x(), r.y(), r.width(), r.height(), &data);

      return true;
   }

   if (mt_engine->metric[MT_NOTEBOOK_IS_FILLED] && ((!qstrcmp(obj->name(), "tab pages") && ev->type() == TQEvent::Resize) ||
       (ev->type() == TQEvent::Move && tabWidgets.find(obj)))) {
      TQObjectList *l = obj->queryList("TQWidget");
      TQObjectListIt it(*l);
      TQWidget *w;

      while ((w = (TQWidget *)it.current())) {
         w->update();
         ++it;
      }

      delete l;
      return false;
   }

   return false;
}


void MetaThemeStyle::setColorPalette(MT_COLOR_PALETTE *palette)
{
   TQColorGroup g;
   int i;

   i = MT_BACKGROUND;          g.setColor(TQColorGroup::Background,      TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_FOREGROUND;          g.setColor(TQColorGroup::Foreground,      TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_WINDOW_BACKGROUND;   g.setColor(TQColorGroup::Base,            TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_WINDOW_FOREGROUND;   g.setColor(TQColorGroup::Text,            TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_BUTTON_BACKGROUND;   g.setColor(TQColorGroup::Button,          TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_BUTTON_FOREGROUND;   g.setColor(TQColorGroup::ButtonText,      TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_SELECTED_BACKGROUND; g.setColor(TQColorGroup::Highlight,       TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_SELECTED_FOREGROUND; g.setColor(TQColorGroup::HighlightedText, TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));

   i = MT_WINDOW_FOREGROUND;   g.setColor(TQColorGroup::BrightText,      TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));

   i = MT_BUTTON_LIGHT;        g.setColor(TQColorGroup::Light,           TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_BUTTON_MIDLIGHT;     g.setColor(TQColorGroup::Midlight,        TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_BUTTON_DARK;         g.setColor(TQColorGroup::Dark,            TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_BUTTON_MID;          g.setColor(TQColorGroup::Mid,             TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_BUTTON_SHADOW;       g.setColor(TQColorGroup::Shadow,          TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));

   g.setColor(TQColorGroup::Link, TQt::blue);
   g.setColor(TQColorGroup::LinkVisited, TQt::magenta);

   qpalette.setActive(g);
   qpalette.setInactive(g);

   g.setColor(TQColorGroup::Light, TQColor(255, 255, 255));
   i = MT_DISABLED_FOREGROUND; g.setColor(TQColorGroup::Text, TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));
   i = MT_DISABLED_FOREGROUND; g.setColor(TQColorGroup::ButtonText, TQColor((*palette)[i].r, (*palette)[i].g, (*palette)[i].b));

   qpalette.setDisabled(g);
}


void MetaThemeStyle::setFont()
{
   qfont.setFamily(mt_engine->font);
   qfont.setPointSize(mt_engine->font_size);
   qfont.setWeight(TQFont::Normal);
   qfont.setItalic(false);

   TQApplication::setFont(qfont, true);
   TQApplication::setFont(qfont, true, "TQMenuBar");
   TQApplication::setFont(qfont, true, "TQPopupMenu");
   TQApplication::setFont(qfont, true, "KPopupTitle");

   // "patch" standard TQStyleSheet to follow our fonts
   TQStyleSheet *sheet = TQStyleSheet::defaultSheet();
   sheet->item("pre")->setFontFamily(qfont.family());
   sheet->item("code")->setFontFamily(qfont.family());
   sheet->item("tt")->setFontFamily(qfont.family());
}
