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

import java.awt.*;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.image.BufferedImage;
import javax.swing.UIManager;
import javax.swing.plaf.basic.BasicGraphicsUtils;

public final class ThemeEngine implements MT {

	public static int metric[] = new int[MT_METRIC_MAX];
	public static Dimension metricSize[] = new Dimension[MT_METRIC_SIZE_MAX];
	public static Color palette[] = new Color[MT_COLOR_MAX];
	public static String fontName;
	public static int fontSize;

	private long handle;

	public ThemeEngine() {
		handle = MetaThemeNative.init(this);
		if (handle == 0) throw new IllegalStateException("ThemeEngine initialization failed");

		MetaThemeNative.realize(this, handle);
	}

	public void destroy() {
		MetaThemeNative.exit(this, handle);
	}

	public void drawWidget(Graphics g, int type, int state, int x, int y, int width, int height, Component comp) {
		MetaThemeNative.drawWidget(handle, g, type, state, x, y, width, height, 0, 0, comp);
	}

	public void drawWidget(Graphics g, int type, int state, int x, int y, int width, int height, Component comp, int flags, int orientation) {
		MetaThemeNative.drawWidget(handle, g, type, state, x, y, width, height, flags, orientation, comp);
	}

	public void drawString(Graphics g, int type, int state, int x, int y, String text, Color color, int mnemIndex) {
		FontMetrics fm = g.getFontMetrics();
		y += fm.getAscent();

		Color oldColor = g.getColor();
		if (color == null) color = oldColor;
		MetaThemeNative.drawString(handle, g, type, state, x, y, text, color.getRed(), color.getGreen(), color.getBlue(), mnemIndex);
		g.setColor(oldColor);
	}

	public static ThemeEngine get() {
		return (ThemeEngine)UIManager.get("MetaThemeEngine");
	}

}
