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
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;
import java.awt.geom.Area;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.plaf.*;
import javax.swing.plaf.metal.MetalTabbedPaneUI;
import javax.swing.plaf.basic.BasicTabbedPaneUI;

import javax.swing.plaf.basic.BasicArrowButton;

public class MetaThemeTabbedPaneUI extends MetalTabbedPaneUI implements MT {

	private ThemeEngine engine;
	private JComponent component;

	public MetaThemeTabbedPaneUI(JComponent c) {
		super();
		engine = ThemeEngine.get();
		component = c;
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeTabbedPaneUI(c);
	}

	public void installUI(JComponent c) {
		super.installUI(c);
	}

	public void uninstallUI(JComponent c) {
		super.uninstallUI(c);
	}

	public void paint(Graphics g, JComponent c) {
		// TODO: discover why it must be here (XXE settings dialog don't like better places)
		if (tabPane.getTabLayoutPolicy() != JTabbedPane.SCROLL_TAB_LAYOUT) {
			tabPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
		}

		super.paint(g, c);
	}

	protected void paintTabBorder(Graphics g, int tabPlacement, int tabIndex, int x, int y, int w, int h, boolean isSelected) {
		int state = MT_NORMAL;
		int flags = 0;
		int orientation = -1;

		if (isSelected) state |= MT_SELECTED;

		int tabCount = tabPane.getTabCount();
		if (tabIndex == 0) flags |= MT_NOTEBOOK_FIRST_TAB;
		if (tabIndex == tabCount - 1) flags |= MT_NOTEBOOK_LAST_TAB;

		Rectangle rect = new Rectangle();
		getTabBounds(tabIndex, rect);
		if (rect.x == 0) flags |= MT_NOTEBOOK_FIRST_VISIBLE_TAB;

		switch (tabPlacement) {
			case TOP:    orientation = MT_POSITION_TOP; break;
			case BOTTOM: orientation = MT_POSITION_BOTTOM; break;
			case LEFT:   orientation = MT_POSITION_LEFT; break;
			case RIGHT:  orientation = MT_POSITION_RIGHT; break;
		}

		w += engine.metric[MT_NOTEBOOK_TAB_OVERLAP];
		engine.drawWidget(g, MT_NOTEBOOK_TAB, state, x, y, w, h, component, flags, orientation);
	}

	protected void paintTabBackground(Graphics g, int tabPlacement, int tabIndex, int x, int y, int w, int h, boolean isSelected) {
	}

	protected void paintFocusIndicator(Graphics g, int tabPlacement, Rectangle[] rects, int tabIndex, Rectangle iconRect, Rectangle textRect, boolean isSelected) {
	}

	protected void paintContentBorder(Graphics g, int tabPlacement, int selectedIndex) {
		int state = MT_NORMAL;
		int flags = 0;
		int orientation = -1;
		int x, y, w, h;

		switch (tabPlacement) {
			case TOP:    orientation = MT_POSITION_TOP; break;
			case BOTTOM: orientation = MT_POSITION_BOTTOM; break;
			case LEFT:   orientation = MT_POSITION_LEFT; break;
			case RIGHT:  orientation = MT_POSITION_RIGHT; break;
		}

		x = 0;
		y = 0;
		w = tabPane.getWidth();
		h = tabPane.getHeight();

		int tabsWidth = calculateTabAreaWidth(tabPlacement, runCount, maxTabHeight);
		int tabsHeight = calculateTabAreaHeight(tabPlacement, runCount, maxTabHeight);

		Rectangle bounds = new Rectangle();
		Rectangle r = new Rectangle(0, 0, 0, tabsHeight);

		for (int i = 0; i < tabPane.getTabCount(); i++) {
			Component component = tabPane.getComponentAt(i);
			if (component == null) continue;

			bounds = getTabBounds(i, bounds);
			r.width += bounds.width;
		}


		switch (tabPlacement) {
			case TOP:
				y += tabsHeight - engine.metric[MT_NOTEBOOK_OVERLAP];
				h -= y;
				//r.width += 2 + engine.metric[MT_NOTEBOOK_TAB_OVERLAP];
				break;

			case BOTTOM:
				h -= tabsHeight - engine.metric[MT_NOTEBOOK_OVERLAP];
				//r.width += 2 + engine.metric[MT_NOTEBOOK_TAB_OVERLAP];
				break;

			case LEFT:
				x += tabsWidth - engine.metric[MT_NOTEBOOK_OVERLAP];
				w -= x;
				break;

			case RIGHT:
				w -= tabsWidth - engine.metric[MT_NOTEBOOK_OVERLAP];
				break;
		}

		Shape oldClip = g.getClip();
		Area area = new Area(oldClip);
		//area.subtract(new Area(r));
		g.setClip(area);

		//g.setColor(new Color(255, 0, 255));
		//g.fillRect(x, y, w, h);

		engine.drawWidget(g, MT_NOTEBOOK, state, x, y, w, h, component, flags, orientation);
		g.setClip(oldClip);
	}

	protected int getTabLabelShiftX(int tabPlacement, int tabIndex, boolean isSelected) {
		if (!isSelected) return 0;

		int offset = engine.metric[MT_NOTEBOOK_TEXT_OFFSET];
		switch (tabPlacement) {
			case LEFT: return -offset;
			case RIGHT: return offset;
		}

		return 0;
	}

	protected int getTabLabelShiftY(int tabPlacement, int tabIndex, boolean isSelected) {
		if (!isSelected) return 1;

		int offset = engine.metric[MT_NOTEBOOK_TEXT_OFFSET];
		switch (tabPlacement) {
			case TOP: return -offset + 1;
			case BOTTOM: return offset;
		}

		return 0;
	}

	protected JButton createScrollButton(int direction) {
		int type;

		switch (direction) {
			case WEST: type = MT_NOTEBOOK_ARROW_LEFT; break;
			case EAST: type = MT_NOTEBOOK_ARROW_RIGHT; break;

			default:
				return super.createScrollButton(direction);
		}

		return new MetaThemeScrollButton(type, this);
	}

	public int getTabAreaHeight() {
		return calculateTabAreaHeight(tabPane.getTabPlacement(), runCount, maxTabHeight);
	}

}
