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
import java.awt.geom.Area;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.plaf.*;
import javax.swing.plaf.basic.BasicBorders;

public class MetaThemeBorders implements MT {

	public static class ButtonBorder extends AbstractBorder implements UIResource, MT {

		private ThemeEngine engine;
		private Insets borderInsets;
		private int widgetType, x, y;

		private ButtonBorder(int widgetType) {
			super();
			engine = ThemeEngine.get();

			this.widgetType = widgetType;
		}

		public ButtonBorder(int widgetType, int metricType) {
			this(widgetType);

			int width = engine.metricSize[metricType].width;
			int height = engine.metricSize[metricType].height;

			if (widgetType == MT_BUTTON) {
				int h1 = height - 2;
				int h2 = height - 1;

				if (h1 < 0) h1 = 0;
				if (h2 < 0) h2 = 0;

				borderInsets = new Insets(h1, width, h2, width);
			}
			else {
				borderInsets = new Insets(height, width, height, width);
			}
		}

		public ButtonBorder(int widgetType, int x, int y) {
			this(widgetType);
			borderInsets = new Insets(y, x, y, x);
		}

		public void paintBorder(Component c, Graphics g, int x, int y, int w, int h) {
			if (widgetType == MT_NONE || widgetType == MT_BUTTON) return;

			Shape clip = null;
			if (widgetType == MT_ENTRY_BORDER || widgetType == MT_CHOICE) {
				clip = g.getClip();
				Area area = new Area(new Rectangle(x, y, w, h));
				area.subtract(new Area(new Rectangle(x + borderInsets.left, y + borderInsets.top, w - borderInsets.left*2, h - borderInsets.top*2)));
				if (clip != null) area.intersect(new Area(clip));
				g.setClip(area);
			}
			else if (widgetType == MT_CHOICE_BUTTON) {
				clip = g.getClip();
				g.clipRect(x, y, w, h);
			}

			Dimension size = c.getSize();
			engine.drawWidget(g, widgetType, Utils.getState(c), x, y, w, h, c);

			if (clip != null) {
				g.setClip(clip);
			}
		}

		public Insets getBorderInsets(Component c) {
			return borderInsets;
		}

		public Insets getBorderInsets(Component c, Insets newInsets) {
			newInsets.top = borderInsets.top;
			newInsets.left = borderInsets.left;
			newInsets.bottom = borderInsets.bottom;
			newInsets.right = borderInsets.right;
			return newInsets;
		}
	}

	public static class MenuBarBorder extends AbstractBorder implements UIResource, MT {

		private Insets borderInsets;
		private ThemeEngine engine;

		public void init() {
			engine = ThemeEngine.get();
			int type = MT_MENUBAR_BORDER;
			borderInsets = new Insets(
				engine.metricSize[type].height,
				engine.metricSize[type].width,
				engine.metricSize[type].height,
				engine.metricSize[type].width
			);
		}

		public void paintBorder(Component c, Graphics g, int x, int y, int w, int h) {
		}

		public Insets getBorderInsets(Component c) {
			if (borderInsets == null) init();
			return borderInsets;
		}

		public Insets getBorderInsets(Component c, Insets newInsets) {
			if (borderInsets == null) init();
			newInsets.top = borderInsets.top;
			newInsets.left = borderInsets.left;
			newInsets.bottom = borderInsets.bottom;
			newInsets.right = borderInsets.right;
			return newInsets;
		}
	}

	public static class ToolBarBorder extends AbstractBorder implements UIResource, MT, SwingConstants {

		private ThemeEngine engine;

		public ToolBarBorder() {
			engine = ThemeEngine.get();
		}

		public void paintBorder(Component c, Graphics g, int x, int y, int w, int h) {
			JToolBar toolBar = (JToolBar)c;
			int orientation = (toolBar.getOrientation() == HORIZONTAL)? MT_HORIZONTAL : MT_VERTICAL;

			engine.drawWidget(g, MT_TOOLBAR, 0, x, y, w, h, c, 0, orientation);

			if (toolBar.isFloatable()) {
				if (toolBar.getOrientation() == HORIZONTAL) {
					x += 2;
					w = 10;
					y += 2;
					h -= 4;
				}
				else {
					y += 2;
					h = 10;
					x += 2;
					w -= 4;
				}

				engine.drawWidget(g, MT_TOOLBAR_HANDLE, 0, x, y, w, h, c, 0, orientation);
			}
		}

		public Insets getBorderInsets(Component c) {
			return getBorderInsets(c, new Insets(0,0,0,0));
		}

		public Insets getBorderInsets(Component c, Insets newInsets) {
			newInsets.top = 0;
			newInsets.left = 0;
			newInsets.bottom = 0;
			newInsets.right = 0;

			JToolBar toolBar = (JToolBar)c;
			if (toolBar.isFloatable()) {
				if (toolBar.getOrientation() == HORIZONTAL) {
					newInsets.left += 12;
					newInsets.top += 2;
					newInsets.bottom += 2;
				}
				else {
					newInsets.top += 12;
					newInsets.left += 2;
					newInsets.right += 2;
				}
			}

			/*
			Insets margin = toolBar.getMargin();
			if (margin != null) {
				newInsets.top += margin.top;
				newInsets.left += margin.left;
				newInsets.bottom += margin.bottom;
				newInsets.right += margin.right;
			}
			*/

			return newInsets;
		}
	}

	private static Border buttonBorder;
	private static Border textBorder;
	private static Border popupMenuBorder;
	private static Border menuItemBorder;
	private static Border scrollPaneBorder;
	private static Border toolBarItemBorder;
	private static Border titledBorder;

	public static Border getButtonBorder() {
		if (buttonBorder == null) {
			buttonBorder = new BorderUIResource.CompoundBorderUIResource(
				new MetaThemeBorders.ButtonBorder(MT_BUTTON, MT_BUTTON_BORDER),
				new BasicBorders.MarginBorder()
			);
		}
		return buttonBorder;
	}

	public static Border getTextFieldBorder() {
		if (textBorder == null) {
			textBorder = new BorderUIResource.CompoundBorderUIResource(
				new MetaThemeBorders.ButtonBorder(MT_ENTRY_BORDER, MT_BUTTON_BORDER),
				new BasicBorders.MarginBorder()
			);
		}
		return textBorder;
	}

	public static Border getPopupMenuBorder() {
		if (popupMenuBorder == null) {
			popupMenuBorder = new MetaThemeBorders.ButtonBorder(MT_MENU, MT_MENU_BORDER);
		}
		return popupMenuBorder;
	}

	public static Border getMenuItemBorder() {
		if (menuItemBorder == null) {
			menuItemBorder = new MetaThemeBorders.ButtonBorder(MT_NONE, 0, 0);
		}
		return menuItemBorder;
	}

	public static Border getScrollPaneBorder() {
		if (scrollPaneBorder == null) {
			ThemeEngine engine = ThemeEngine.get();
			scrollPaneBorder = new MetaThemeBorders.ButtonBorder(MT_SCROLLED_WINDOW, engine.metric[MT_BORDER], engine.metric[MT_BORDER]);
		}
		return scrollPaneBorder;
	}

	public static Border getToolBarItemBorder() {
		if (toolBarItemBorder == null) {
			toolBarItemBorder = new MetaThemeBorders.ButtonBorder(MT_NONE, MT_TOOLBAR_ITEM_BORDER);
		}
		return toolBarItemBorder;
	}

	public static Border getTitledBorder() {
		if (titledBorder == null) {
			titledBorder = new MetaThemeBorders.ButtonBorder(MT_STATICBOX, 2, 2);
		}
		return titledBorder;
	}

}
