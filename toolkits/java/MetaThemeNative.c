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

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "MetaThemeNative.h"
#include "metathemeInt.h"

#define MIN(a, b) ((a) < (b)? (a):(b))

static MT_COLOR button_bg;
static JNIEnv *_env = NULL;
static jclass cls = NULL;
static jmethodID mid_setMetric = 0;
static jmethodID mid_setMetricSize = 0;
static jmethodID mid_setColor = 0;
static jmethodID mid_setFont = 0;
static jmethodID mid_newImage = 0;
static jmethodID mid_newImageFromXpm = 0;
static jmethodID mid_copyImage = 0;
static jmethodID mid_imageReplaceColor = 0;
static jmethodID mid_setImagePixel = 0;
static jmethodID mid_getImagePixel = 0;
static jmethodID mid_fillImage = 0;
static jmethodID mid_imageGetWidth = 0;
static jmethodID mid_imageGetHeight = 0;
static jmethodID mid_drawPoint = 0;
static jmethodID mid_drawLine = 0;
static jmethodID mid_drawRectangle = 0;
static jmethodID mid_drawPolygon = 0;
static jmethodID mid_drawImage = 0;
static jmethodID mid_drawImageTiled = 0;
static jmethodID mid_drawImageScaled = 0;
static jmethodID mid_drawString = 0;
static jmethodID mid_widgetGetParent = 0;
static jmethodID mid_widgetSetData = 0;
static jmethodID mid_widgetGetData = 0;

typedef struct {
   MT_COLOR col;
} MT_JAVA_GC;

typedef struct {
   jint x, y;
   jstring text;
   MT_COLOR col;
   jint mnem_index;
} MT_JAVA_STRING;

#define java_set_metric(engine, idx, value) _call_static_void_method(&mid_setMetric, "callbackSetMetric", "(Lmetatheme/ThemeEngine;II)V", engine, idx, value)
#define java_set_metric_size(engine, idx, x, y) _call_static_void_method(&mid_setMetricSize, "callbackSetMetricSize", "(Lmetatheme/ThemeEngine;III)V", engine, idx, x, y)
#define java_set_color(engine, idx, r, g, b) _call_static_void_method(&mid_setColor, "callbackSetColor", "(Lmetatheme/ThemeEngine;IIII)V", engine, idx, r, g, b)
#define java_set_font(engine, name, size) \
   { \
      jstring _name = (*_env)->NewStringUTF(_env, name); \
      _call_static_void_method(&mid_setFont, "callbackSetFont", "(Lmetatheme/ThemeEngine;Ljava/lang/String;I)V", engine, _name, size); \
      (*_env)->DeleteLocalRef(_env, _name); \
   }

#define java_new_image(width, height) _call_static_object_method(&mid_newImage, "callbackNewImage", "(II)Ljava/awt/image/BufferedImage;", width, height)

#define java_new_image_from_xpm(ret, xpm_data) \
   { \
      jstring _xpm_data = (*_env)->NewStringUTF(_env, xpm_data); \
      ret = _call_static_object_method(&mid_newImageFromXpm, "callbackNewImageFromXpm", "(Ljava/lang/String;)Ljava/awt/image/BufferedImage;", _xpm_data); \
      (*_env)->DeleteLocalRef(_env, _xpm_data); \
   }

#define java_copy_image(img) _call_static_object_method(&mid_copyImage, "callbackCopyImage", "(Ljava/awt/Image;)Ljava/awt/image/BufferedImage;", img)
#define java_image_replace_color(img, r1, g1, b1, r2, g2, b2, a2) _call_static_object_method(&mid_imageReplaceColor, "callbackImageReplaceColor", "(Ljava/awt/image/BufferedImage;IIIIIII)V", img, r1, g1, b1, r2, g2, b2, a2)
#define java_set_image_pixel(img, x, y, r, g, b, a) _call_static_object_method(&mid_setImagePixel, "callbackSetImagePixel", "(Ljava/awt/image/BufferedImage;IIIIII)V", img, x, y, r, g, b, a)
#define java_get_image_pixel(img, x, y) _call_static_int_method(&mid_getImagePixel, "callbackGetImagePixel", "(Ljava/awt/image/BufferedImage;II)I", img, x, y)
#define java_fill_image(img, r, g, b) _call_static_object_method(&mid_fillImage, "callbackFillImage", "(Ljava/awt/image/BufferedImage;III)V", img, r, g, b)
#define java_image_get_width(img) _call_static_int_method(&mid_imageGetWidth, "callbackImageGetWidth", "(Ljava/awt/image/BufferedImage;)I", img)
#define java_image_get_height(img) _call_static_int_method(&mid_imageGetHeight, "callbackImageGetHeight", "(Ljava/awt/image/BufferedImage;)I", img)

#define java_draw_point(win, r, g, b, x, y) _call_static_void_method(&mid_drawPoint, "callbackDrawPoint", "(Ljava/awt/Graphics;IIIII)V", win, r, g, b, x, y)
#define java_draw_line(win, r, g, b, x1, y1, x2, y2) _call_static_void_method(&mid_drawLine, "callbackDrawLine", "(Ljava/awt/Graphics;IIIIIII)V", win, r, g, b, x1, y1, x2, y2)
#define java_draw_rectangle(win, r, g, b, filled, x1, y1, x2, y2) _call_static_void_method(&mid_drawRectangle, "callbackDrawRectangle", "(Ljava/awt/Graphics;IIIIIIII)V", win, r, g, b, filled, x1, y1, x2, y2)
#define java_draw_polygon(win, r, g, b, filled, xp, yp, np) _call_static_void_method(&mid_drawPolygon, "callbackDrawPolygon", "(Ljava/awt/Graphics;IIII[I[II)V", win, r, g, b, filled, xp, yp, np)
#define java_draw_image(win, img, xsrc, ysrc, xdest, ydest, width, height) _call_static_void_method(&mid_drawImage, "callbackDrawImage", "(Ljava/awt/Graphics;Ljava/awt/image/BufferedImage;IIIIII)V", win, img, xsrc, ysrc, xdest, ydest, width, height)
#define java_draw_image_tiled(win, img, srcwidth, srcheight, xdest, ydest, width, height) _call_static_void_method(&mid_drawImageTiled, "callbackDrawImageTiled", "(Ljava/awt/Graphics;Ljava/awt/image/BufferedImage;IIIIII)V", win, img, srcwidth, srcheight, xdest, ydest, width, height)
#define java_draw_image_scaled(win, img, xsrc, ysrc, wsrc, hsrc, xdest, ydest, width, height) _call_static_void_method(&mid_drawImageScaled, "callbackDrawImageScaled", "(Ljava/awt/Graphics;Ljava/awt/image/BufferedImage;IIIIIIII)V", win, img, xsrc, ysrc, wsrc, hsrc, xdest, ydest, width, height)
#define java_draw_string(win, x, y, text, r, g, b, mnemIndex) _call_static_void_method(&mid_drawString, "callbackDrawString", "(Ljava/awt/Graphics;IILjava/lang/String;IIII)V", win, x, y, text, r, g, b, mnemIndex)

#define java_widget_get_parent(widget) _call_static_object_method(&mid_widgetGetParent, "callbackWidgetGetParent", "(Ljava/awt/Component;)Ljava/awt/Component;", widget)
#define java_widget_set_data(widget, data, data_dispose) _call_static_void_method(&mid_widgetSetData, "callbackWidgetSetData", "(Ljava/awt/Component;JJ)V", widget, data, data_dispose)
#define java_widget_get_data(widget) _call_static_long_method(&mid_widgetGetData, "callbackWidgetGetData", "(Ljava/awt/Component;)J", widget)

static void _get_method(jmethodID *mid, const char *name, const char *type)
{
   if (!_env) {
      fprintf(stderr, "!_env\n");
      return;
   }

   if (!cls) {
      cls = (*_env)->FindClass(_env, "metatheme/MetaThemeNative");
      if (!cls) {
         fprintf(stderr, "!cls\n");
         return;
      }

      cls = (*_env)->NewGlobalRef(_env, cls);
      if (!cls) {
         fprintf(stderr, "!cls\n");
         return;
      }
   }
   
   if (*mid == 0) {
      *mid = (*_env)->GetStaticMethodID(_env, cls, name, type);
      if (*mid == 0) {
         fprintf(stderr, "!mid\n");
         return;
      }
   }
}


static void _call_static_void_method(jmethodID *mid, const char *name, const char *type, ...)
{
   va_list ap;

   _get_method(mid, name, type);

   va_start(ap, type);
   (*_env)->CallStaticVoidMethodV(_env, cls, *mid, ap);
   va_end(ap);
}


static jobject _call_static_object_method(jmethodID *mid, const char *name, const char *type, ...)
{
   va_list ap;
   jobject ret;

   _get_method(mid, name, type);

   va_start(ap, type);
   ret = (*_env)->CallStaticObjectMethodV(_env, cls, *mid, ap);
   va_end(ap);

   return ret;
}


static jlong _call_static_long_method(jmethodID *mid, const char *name, const char *type, ...)
{
   va_list ap;
   jlong ret;

   _get_method(mid, name, type);

   va_start(ap, type);
   ret = (*_env)->CallStaticLongMethodV(_env, cls, *mid, ap);
   va_end(ap);

   return ret;
}


static jint _call_static_int_method(jmethodID *mid, const char *name, const char *type, ...)
{
   va_list ap;
   jint ret;

   _get_method(mid, name, type);

   va_start(ap, type);
   ret = (*_env)->CallStaticIntMethodV(_env, cls, *mid, ap);
   va_end(ap);

   return ret;
}


static MT_GC *_mt_gc_new_with_foreground(MT_COLOR *col)
{
   MT_JAVA_GC *gc = (MT_JAVA_GC *)malloc(sizeof(MT_JAVA_GC));
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
   ((MT_JAVA_GC *)gc)->col = *col;
}


static MT_PIXMAP *_mt_pixmap_new_from_xpm(char **xpm_data)
{
   jobject img;
   int width, height, colors, count, alloc, i;
   char *buf;
   
   /* retrieve width, height and number of colors: */
   sscanf(xpm_data[0], "%d %d %d", &width, &height, &colors);
   count = 1+colors+height;

   /* calculate total length of string: */
   alloc = 10 + 24;
   for (i=0; i<count; i++) {
      alloc += strlen(xpm_data[i]) + 4;
   }
   alloc += 1;

   /* create string: */
   buf = (char *)malloc(alloc + 1);
   strcpy(buf, "/* XPM */\n"); /* 10 */
   strcat(buf, "static char * xpm[] = {\n"); /* 24 */
   for (i=0; i<count; i++) {
      strcat(buf, "\""); /* 1 */
      strcat(buf, xpm_data[i]);
      if (i < count-1) {
         strcat(buf, "\",\n"); /* 3 */
      }
      else {
         strcat(buf, "\"};\n"); /* 4 */
      }
   }

   java_new_image_from_xpm(img, buf);
   free(buf);

   img = (*_env)->NewGlobalRef(_env, img);
   return (MT_PIXMAP *)img;
}


static MT_PIXMAP *_mt_pixmap_new_from_image(MT_IMAGE *img)
{
   jobject pixmap;

   pixmap = (*_env)->NewGlobalRef(_env, img);
   return (MT_PIXMAP *)pixmap;
}


static void _mt_pixmap_destroy(MT_PIXMAP *pixmap)
{
   (*_env)->DeleteGlobalRef(_env, (jobject)pixmap);
}


static MT_IMAGE *_mt_image_new(int width, int height)
{
   jobject img;
   
   img = java_new_image(width, height);
   img = (*_env)->NewGlobalRef(_env, img);
   return (MT_IMAGE *)img;
}


static MT_IMAGE *_mt_image_new_from_xpm(char **xpm_data)
{
   return (MT_IMAGE *)_mt_pixmap_new_from_xpm(xpm_data);
}


static MT_IMAGE *_mt_image_copy(MT_IMAGE *img)
{
   jobject image;
   
   image = java_copy_image(img);
   image = (*_env)->NewGlobalRef(_env, image);
   return (MT_IMAGE *)image;
}


static void _mt_image_destroy(MT_IMAGE *image)
{
   (*_env)->DeleteGlobalRef(_env, (jobject)image);
}


static void _mt_image_replace_color(MT_IMAGE *img, int r, int g, int b, MT_COLOR *col)
{
   java_image_replace_color((jobject)img, r, g, b, col->r, col->g, col->b, col->a);
}


static void _mt_image_set_pixel(MT_IMAGE *img, int x, int y, MT_COLOR *col)
{
   java_set_image_pixel((jobject)img, x, y, col->r, col->g, col->b, col->a);
}


static void _mt_image_get_pixel(MT_IMAGE *img, int x, int y, MT_COLOR *col)
{
   unsigned int i = java_get_image_pixel((jobject)img, x, y);

   col->a = (i >> 24) & 0xFF;
   col->r = (i >> 16) & 0xFF;
   col->g = (i >>  8) & 0xFF;
   col->b = (i >>  0) & 0xFF;
}


static void _mt_image_fill(MT_IMAGE *img, MT_COLOR *col)
{
   java_fill_image((jobject)img, col->r, col->g, col->b);
}


static void _mt_image_get_size(MT_IMAGE *img, int *width, int *height)
{
   *width = java_image_get_width((jobject)img);
   *height = java_image_get_height((jobject)img);
}


static void _mt_draw_point(MT_WINDOW *win, MT_GC *gc, int x, int y)
{
   MT_JAVA_GC *jgc = (MT_JAVA_GC *)gc;

   java_draw_point(win, jgc->col.r, jgc->col.g, jgc->col.b, x, y);
}


static void _mt_draw_line(MT_WINDOW *win, MT_GC *gc, int x1, int y1, int x2, int y2)
{
   MT_JAVA_GC *jgc = (MT_JAVA_GC *)gc;

   java_draw_line(win, jgc->col.r, jgc->col.g, jgc->col.b, x1, y1, x2, y2);
}


static void _mt_draw_rectangle(MT_WINDOW *win, MT_GC *gc, int filled, int x1, int y1, int x2, int y2)
{
   MT_JAVA_GC *jgc = (MT_JAVA_GC *)gc;

   java_draw_rectangle(win, jgc->col.r, jgc->col.g, jgc->col.b, filled, x1, y1, x2, y2);
}


static void _mt_draw_polygon(MT_WINDOW *win, MT_GC *gc, int filled, MT_POINT *points, int npoints)
{
   MT_JAVA_GC *jgc = (MT_JAVA_GC *)gc;
   jintArray xp, yp;
   jint *xpb, *ypb;
   int i;
   
   xp = (*_env)->NewIntArray(_env, npoints);
   yp = (*_env)->NewIntArray(_env, npoints);
   
   xpb = (*_env)->GetIntArrayElements(_env, xp, 0);
   ypb = (*_env)->GetIntArrayElements(_env, yp, 0);

   for (i=0; i<npoints; i++) {
      xpb[i] = points[i].x;
      ypb[i] = points[i].y;
   }

   (*_env)->ReleaseIntArrayElements(_env, xp, xpb, 0);
   (*_env)->ReleaseIntArrayElements(_env, yp, ypb, 0);

   java_draw_polygon(win, jgc->col.r, jgc->col.g, jgc->col.b, filled, xp, yp, npoints);

   (*_env)->DeleteLocalRef(_env, xp);
   (*_env)->DeleteLocalRef(_env, yp);
}


static void _mt_draw_string(MT_WINDOW *win, MT_STRING *str, int x_offset, int y_offset, MT_COLOR *col)
{
   MT_JAVA_STRING *s = (MT_JAVA_STRING *)str;
   
   if (!col) col = &s->col;

   java_draw_string(win, s->x + x_offset, s->y + y_offset, s->text, col->r, col->g, col->b, s->mnem_index);
}


static void _mt_draw_pixmap(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int xsrc, int ysrc, int xdest, int ydest, int width, int height)
{
   if (!pixmap) return;
   java_draw_image((jobject)win, (jobject)pixmap, xsrc, ysrc, xdest, ydest, width, height);
}


static void _mt_draw_pixmap_tiled(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int srcwidth, int srcheight, int xdest, int ydest, int width, int height)
{
   if (!pixmap) return;
   java_draw_image_tiled((jobject)win, (jobject)pixmap, srcwidth, srcheight, xdest, ydest, width, height);
}


static void _mt_draw_image(MT_WINDOW *win, MT_GC *gc, MT_IMAGE *img, int xsrc, int ysrc, int wsrc, int hsrc, int xdest, int ydest, int width, int height)
{
   if (!img) return;
   java_draw_image_scaled((jobject)win, (jobject)img, xsrc, ysrc, wsrc, hsrc, xdest, ydest, width, height);
}


static MT_WIDGET *_mt_widget_ref(MT_WIDGET *widget)
{
   MT_WIDGET *result;
   
   result = (MT_WIDGET *)malloc(sizeof(MT_WIDGET));
   *result = (*_env)->NewGlobalRef(_env, *widget);
   return result;
}


static void _mt_widget_unref(MT_WIDGET *widget)
{
   (*_env)->DeleteGlobalRef(_env, *widget);
   free(widget);
}


static MT_WIDGET *_mt_widget_get_parent(MT_WIDGET *widget, MT_WIDGET *result)
{
   if (!result) result = (MT_WIDGET *)malloc(sizeof(MT_WIDGET));

   *result = (MT_WIDGET)java_widget_get_parent((jobject)*widget);
   return result;
}


static void _mt_widget_set_data(MT_WIDGET *widget, void *data, mt_destroy_func data_dispose)
{
   java_widget_set_data((jobject)*widget, (jlong)(int)data, (jlong)(int)data_dispose);
}


static void *_mt_widget_get_data(MT_WIDGET *widget)
{
   return (void *)(int)java_widget_get_data((jobject)*widget);
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


JNIEXPORT jlong JNICALL Java_metatheme_MetaThemeNative_init(JNIEnv *env, jclass klass, jobject engine)
{
   MT_LIBRARY *lib;
   MT_ENGINE *mt_engine;
   int i;

   _env = env;

   metatheme_toolkit_init(&mt);
   
   lib = metatheme_library_init(&mt);
   if (!lib) {
      fprintf(stderr, "lib init :(\n");
      _env = NULL;
      return JNI_FALSE;
   }
   
   mt_engine = metatheme_load_engine(lib, NULL);
   if (!mt_engine) {
      metatheme_library_exit(lib);
      fprintf(stderr, "load engine :(\n");
      _env = NULL;
      return 0;
   }

   metatheme_load_default_settings(mt_engine);
   mt_engine->init(mt_engine);
   
   java_set_font(engine, mt_engine->font, mt_engine->font_size);
   for (i=0; i<MT_COLOR_MAX; i++) java_set_color(engine, i, mt_engine->palette[i].r, mt_engine->palette[i].g, mt_engine->palette[i].b);
   for (i=0; i<MT_METRIC_SIZE_MAX; i++) java_set_metric_size(engine, i, mt_engine->metric_size[i].x, mt_engine->metric_size[i].y);
   for (i=0; i<MT_METRIC_MAX; i++) java_set_metric(engine, i, mt_engine->metric[i]);

   button_bg = mt_engine->palette[MT_BUTTON_BACKGROUND];
   
   mt_engine->realize(mt_engine);
   fprintf(stderr, "initialized\n");

   _env = NULL;

   return (jlong)(int)mt_engine;
}


JNIEXPORT void JNICALL Java_metatheme_MetaThemeNative_exit(JNIEnv *env, jclass klass, jobject engine, jlong handle)
{
   fprintf(stderr, "exit\n");
}


JNIEXPORT void JNICALL Java_metatheme_MetaThemeNative_realize(JNIEnv *env, jclass klass, jobject engine, jlong handle)
{
   fprintf(stderr, "realize\n");
}


JNIEXPORT void JNICALL Java_metatheme_MetaThemeNative_drawWidget(JNIEnv *env, jclass klass, jlong handle, jobject graphics, jint type, jint state, jint x, jint y, jint width, jint height, jint flags, jint orientation, jobject widget)
{
   MT_WIDGET_DATA data;
   MT_ENGINE *mt_engine;

   mt_engine = (MT_ENGINE *)(int)handle;
   data.widget = (MT_WIDGET)widget;
   data.flags = flags;
   data.orientation = orientation;
   if (type == MT_NOTEBOOK_TAB) {
      data.gap_position = orientation;
   }
   else {
      data.gap_position = -1;
   }
   data.gap_x = 0;
   data.gap_width = 0;

   /* special case: */
   if (type == MT_SCROLLBAR_HANDLE || type == MT_SCROLLBAR) {
      data.handle_position = data.flags >> 4;
      data.groove_size = data.orientation >> 4;

      data.flags &= 0xF;
      data.orientation &= 0xF;
   }
   
   _env = env;
   data.background_color = button_bg;
   mt_engine->draw_widget(mt_engine, (MT_WINDOW *)graphics, NULL, type, state, x, y, width, height, &data);
   _env = NULL;
}


JNIEXPORT void JNICALL Java_metatheme_MetaThemeNative_drawString(JNIEnv *env, jclass klass, jlong handle, jobject graphics, jint type, jint state, jint x, jint y, jstring text, jint red, jint green, jint blue, jint mnem_index)
{
   MT_ENGINE *mt_engine;
   MT_JAVA_STRING str;

   str.x = x;
   str.y = y;
   str.text = text;
   str.col.r = red;
   str.col.g = green;
   str.col.b = blue;
   str.mnem_index = mnem_index;

   mt_engine = (MT_ENGINE *)(int)handle;
   _env = env;
   mt_engine->draw_string(mt_engine, (MT_WINDOW *)graphics, type, state, &str);
   _env = NULL;
}


JNIEXPORT void JNICALL Java_metatheme_MetaThemeNative_disposeWidgetData(JNIEnv *env, jclass klass, jlong data, jlong data_dispose)
{
   mt_destroy_func destroy_func;

   if (data_dispose == 0) return;
   
   destroy_func = (mt_destroy_func)(int)data_dispose;

   _env = env;
   destroy_func((void *)(int)data);
   _env = NULL;
}
