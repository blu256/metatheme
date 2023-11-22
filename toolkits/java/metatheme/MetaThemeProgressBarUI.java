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
import javax.swing.*;
import javax.swing.plaf.*;
import javax.swing.plaf.basic.BasicProgressBarUI;

public class MetaThemeProgressBarUI extends BasicProgressBarUI implements MT {

	private ThemeEngine engine;

	public MetaThemeProgressBarUI() {
		super();
		engine = ThemeEngine.get();
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeProgressBarUI();
	}

	public void paint(Graphics g, JComponent c) {
		Dimension size = c.getSize();
		int state = Utils.getState(c);
		int orientation = (progressBar.getOrientation() == JProgressBar.HORIZONTAL)? MT_HORIZONTAL : MT_VERTICAL;

		engine.drawWidget(g, MT_PROGRESSBAR, state, 0, 0, size.width, size.height, c, 0, orientation);

		Insets b = progressBar.getInsets();
		int barRectWidth = progressBar.getWidth() - (b.left + b.right);
		int barRectHeight = progressBar.getHeight() - (b.top + b.bottom);
		int amountFull = getAmountFull(b, barRectWidth, barRectHeight);

		if (orientation == MT_HORIZONTAL) {
			engine.drawWidget(g, MT_PROGRESSBAR_SLIDER, state, b.left, b.top, amountFull, barRectHeight, c, 0, orientation);
		}
		else {
			engine.drawWidget(g, MT_PROGRESSBAR_SLIDER, state, b.left, b.top, barRectWidth, amountFull, c, 0, orientation);
		}

		if (progressBar.isStringPainted()) {
			if (progressBar.getOrientation() == JProgressBar.HORIZONTAL) {
				paintString(g, b.left, b.top, barRectWidth, barRectHeight, amountFull, state);
			}
			else {
				paintString(g, b.left, b.top, barRectWidth, barRectHeight, amountFull, state);
			}
		}
	}

	private void paintString(Graphics g, int x, int y, int width, int height, int amountFull, int state) {
		if (progressBar.getOrientation() == JProgressBar.VERTICAL) {
			paintString(g, x, y, width, height, amountFull, null);
			return;
		}

		String progressString = progressBar.getString();
		Point p = getStringPlacement(g, progressString, x, y, width, height);

		FontMetrics fm = g.getFontMetrics();
		p.y -= fm.getAscent();

		Shape clip = g.getClip();
		g.setClip(x, y, amountFull, height);
		engine.drawString(g, MT_PROGRESSBAR_SLIDER, state, p.x, p.y, progressString, engine.palette[MT_SELECTED_FOREGROUND], -1);
		g.setClip(x+amountFull, y, width - amountFull, height);
		engine.drawString(g, MT_PROGRESSBAR, state, p.x, p.y, progressString, engine.palette[MT_FOREGROUND], -1);
		g.setClip(clip);
	}

}
