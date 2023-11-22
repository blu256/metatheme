/*
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

package metatheme;

import java.util.HashMap;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.plaf.basic.BasicGraphicsUtils;

public final class MetaThemeNative implements MT {

	static {
		System.loadLibrary("mt_java");
	}

	/* native functions: */

	public static native long init(ThemeEngine engine);

	public static native void exit(ThemeEngine engine, long handle);

	public static native void realize(ThemeEngine engine, long handle);

	public static native void drawWidget(long handle, Graphics g, int type, int state, int x, int y, int width, int height, int flags, int orientation, Component comp);

	public static native void drawString(long handle, Graphics g, int type, int state, int x, int y, String text, int red, int green, int blue, int mnemIndex);

	public static native void disposeWidgetData(long data, long dispose);

	/* callbacks: */

	public static void callbackSetMetric(ThemeEngine engine, int type, int value) {
		engine.metric[type] = value;
	}

	public static void callbackSetMetricSize(ThemeEngine engine, int type, int x, int y) {
		engine.metricSize[type] = new Dimension(x, y);
	}

	public static void callbackSetColor(ThemeEngine engine, int type, int r, int g, int b) {
		engine.palette[type] = new Color(r, g, b);
	}

	public static void callbackSetFont(ThemeEngine engine, String name, int size) {
		engine.fontName = name;
		engine.fontSize = size;
	}

	public static BufferedImage callbackNewImage(int width, int height) {
		return new BufferedImage(width, height, BufferedImage.TYPE_4BYTE_ABGR);
	}

	public static BufferedImage callbackNewImageFromXpm(String xpmData) {
		Image xpm = Xpm.XpmToImage(xpmData);
		if (xpm == null) return null;
		return callbackCopyImage(xpm);
	}

	public static BufferedImage callbackCopyImage(Image orig) {
		int width = orig.getWidth(null);
		int height = orig.getHeight(null);
		BufferedImage img = new BufferedImage(width, height, BufferedImage.TYPE_4BYTE_ABGR);
		Graphics g = img.getGraphics();
		g.drawImage(orig, 0, 0, width, height, null);
		return img;
	}

	public static void callbackImageReplaceColor(BufferedImage img, int r1, int g1, int b1, int r2, int g2, int b2, int a2) {
		int width = img.getWidth();
		int height = img.getHeight();
		int i;
		
		int[] rgb = img.getRGB(0, 0, width, height, null, 0, width);
		for (i=0; i<width*height; i++) {
			if ((rgb[i] & 0xFFFFFF) == ((r1<<16) + (g1<<8) + b1)) {
				rgb[i] = (r2<<16) + (g2<<8) + b2 + (a2 << 24);
			}
		}
		img.setRGB(0, 0, width, height, rgb, 0, width);
	}

	public static void callbackSetImagePixel(BufferedImage img, int x, int y, int r, int g, int b, int a) {
		img.setRGB(x, y, (a<<24) | (r<<16) | (g<<8) | b);
	}

	public static int callbackGetImagePixel(BufferedImage img, int x, int y) {
		return img.getRGB(x, y);
	}

	public static void callbackFillImage(BufferedImage img, int red, int green, int blue) {
		Graphics g = img.getGraphics();
		g.setColor(new Color(red, green, blue));
		g.fillRect(0, 0, img.getWidth(), img.getHeight());
	}

	public static int callbackImageGetWidth(BufferedImage img) {
		return img.getWidth();
	}

	public static int callbackImageGetHeight(BufferedImage img) {
		return img.getHeight();
	}

	public static void callbackDrawPoint(Graphics g, int red, int green, int blue, int x, int y) {
		g.setColor(new Color(red, green, blue));
		g.drawLine(x, y, x, y);
	}

	public static void callbackDrawLine(Graphics g, int red, int green, int blue, int x1, int y1, int x2, int y2) {
		g.setColor(new Color(red, green, blue));
		g.drawLine(x1, y1, x2, y2);
	}

	public static void callbackDrawRectangle(Graphics g, int red, int green, int blue, int filled, int x1, int y1, int x2, int y2) {
		g.setColor(new Color(red, green, blue));
		if (filled != 0) {
			g.fillRect(x1, y1, x2-x1+1, y2-y1+1);
		}
		else {
			g.drawRect(x1, y1, x2-x1+0, y2-y1+0);
		}
	}

	public static void callbackDrawPolygon(Graphics g, int red, int green, int blue, int filled, int[] xPoints, int[] yPoints, int nPoints) {
		g.setColor(new Color(red, green, blue));
		if (filled != 0) {
			g.fillPolygon(xPoints, yPoints, nPoints);
		}
		else {
			g.drawPolygon(xPoints, yPoints, nPoints);
		}
	}

	public static void callbackDrawImage(Graphics g, BufferedImage img, int xsrc, int ysrc, int xdest, int ydest, int width, int height) {
		if (width < img.getWidth() || height < img.getHeight()) {
			Shape clip = g.getClip();
			g.clipRect(xdest, ydest, width, height);
			g.drawImage(img, xdest, ydest, img.getWidth(), img.getHeight(), null);
			g.setClip(clip);
		}
		else {
			g.drawImage(img, xdest, ydest, width, height, null);
		}
	}

	public static void callbackDrawImageTiled(Graphics g, BufferedImage img, int srcwidth, int srcheight, int xdest, int ydest, int width, int height) {
		int x, y, w, h, width0;

		y = ydest;
		width0 = width;
		while (height > 0) {
			h = Math.min(srcheight, height);
			x = xdest;
			width = width0;
			while (width > 0) {
				w = Math.min(srcwidth, width);
				callbackDrawImage(g, img, 0, 0, x, y, w, h);
				x += w;
				width -= w;
			}
			y += h;
			height -= h;
		}
	}

	public static void callbackDrawImageScaled(Graphics g, BufferedImage img, int xsrc, int ysrc, int wsrc, int hsrc, int xdest, int ydest, int width, int height) {
		g.drawImage(img, xdest, ydest, xdest+width, ydest+height, xsrc, ysrc, xsrc+wsrc, ysrc+hsrc, null);
	}

	public static void callbackDrawString(Graphics g, int x, int y, String text, int red, int green, int blue, int mnemIndex) {
		g.setColor(new Color(red, green, blue));
		BasicGraphicsUtils.drawStringUnderlineCharAt(g, text, mnemIndex, x, y);
	}

	public static Component callbackWidgetGetParent(Component comp) {
		return comp.getParent();
	}

	public static void callbackWidgetSetData(Component comp, long data, long dispose) {
		widgetData.put(comp, new WidgetData(comp, data, dispose));
	}

	public static long callbackWidgetGetData(Component comp) {
		WidgetData data = (WidgetData)widgetData.get(comp);
		return (data != null)? data.data : 0;
	}

	private static HashMap widgetData = new HashMap();

	private static class WidgetData implements HierarchyListener {
		
		public long data;
		public long dispose;

		public WidgetData(Component comp, long data, long dispose) {
			this.data = data;
			this.dispose = dispose;

			if (dispose != 0) {
				comp.addHierarchyListener(this);
			}
		}

		public void hierarchyChanged(HierarchyEvent e) {
			if ((e.getChangeFlags() & e.DISPLAYABILITY_CHANGED) != 0 && !e.getComponent().isDisplayable()) {
				disposeWidgetData(data, dispose);
				e.getComponent().removeHierarchyListener(this);
				widgetData.remove(e.getComponent());
			}
		}
		
	}

}
