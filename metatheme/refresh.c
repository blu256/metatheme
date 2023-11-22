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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utime.h>
#include <X11/X.h>
#include <X11/Xlib.h>

static Display *dpy; 


static long get_simple_property(Window w, Atom a)
{
   Atom real_type;
   int format;
   unsigned long n, extra, *p = 0, res;
   int status;

   res = 0;
   status = XGetWindowProperty(dpy, w, a, 0L, 1L, False, a, &real_type, &format, &n, &extra, (unsigned char **)&p);
   
   if ((status == Success) && (n == 1) && (format == 32)) {
      res = p[0];
   }
   
   if (p) {
      XFree((char *) p);
   }

   return res;
}


static void send_qt_reload_message(Window w)
{
   static Atom a = 0;
   XEvent ev;

   if (a == 0) a = XInternAtom(dpy, "KIPC_COMM_ATOM", False);
   
   ev.xclient.type = ClientMessage;
   ev.xclient.display = dpy;
   ev.xclient.window = (Window) w;
   ev.xclient.message_type = a;
   ev.xclient.format = 32;
   ev.xclient.data.l[0] = 2 /* message type: style changed */;
   ev.xclient.data.l[1] = 0;
   XSendEvent(dpy, (Window) w, False, 0L, &ev);
}


static void send_gtk_reload_message(Window w)
{
   static Atom a = 0;
   XEvent ev;

   if (a == 0) a = XInternAtom(dpy, "_GTK_READ_RCFILES", False);
   
   ev.xclient.type = ClientMessage;
   ev.xclient.display = dpy;
   ev.xclient.window = (Window) w;
   ev.xclient.message_type = a;
   ev.xclient.format = 32;
   XSendEvent(dpy, (Window) w, True, 0L, &ev);
   XFlush(dpy);
}


static void send_reload_messages()
{
   unsigned int i, nrootwins;
   Window dw1, dw2, *rootwins = 0;
   int s, screen_count = ScreenCount(dpy);
   Atom kde_atom;

   kde_atom = XInternAtom(dpy, "KDE_DESKTOP_WINDOW", False);

   for (s=0; s < screen_count; s++) {
      Window root = RootWindow(dpy, s);

      XQueryTree(dpy, root, &dw1, &dw2, &rootwins, &nrootwins);
   
      for (i = 0; i < nrootwins; i++) {
         /* GTK2: */
         send_gtk_reload_message(rootwins[i]);

         /* QT: */
         if (get_simple_property(rootwins[i], kde_atom) != 0L) {
            send_qt_reload_message(rootwins[i]);
         }
      }
      XFree((char *)rootwins);
   }

   XSync(dpy, False);
}


void metatheme_refresh(Display *display)
{
   char buf[256];

   if (display) {
      dpy = display;
   }
   else {
      dpy = XOpenDisplay(NULL);
      if (!dpy) {
         printf("Cannot connect to X server (%s).\n", getenv("DISPLAY"));
         return;
      }
   }

   snprintf(buf, 256, "%s/.gtkrc-2.0", getenv("HOME"));
   utime(buf, NULL);
   
   send_reload_messages();

   if (!display) {
      XCloseDisplay(dpy);
   }
}
