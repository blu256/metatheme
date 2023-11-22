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
import java.awt.event.KeyEvent;
import javax.swing.*;
import javax.swing.plaf.*;
import javax.swing.plaf.basic.BasicMenuUI;
import javax.swing.plaf.basic.BasicGraphicsUtils;

public class MetaThemeMenuUI extends BasicMenuUI implements MT {

	private ThemeEngine engine;
	private JMenu menu;
	private int type;
	private MetaThemeMenuItemUI itemUI;
	private MouseListener mouseListener;

	public MetaThemeMenuUI(JMenu m) {
		super();
		engine = ThemeEngine.get();
		menu = m;

		type = MT_MENU_ITEM;
		itemUI = new MetaThemeMenuItemUI((JMenuItem)menu);
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeMenuUI((JMenu)c);
	}

	public void installUI(JComponent c) {
		super.installUI(c);

		mouseListener = new MouseListener();
		c.addMouseListener(mouseListener);
	}

	public void uninstallUI(JComponent c) {
		c.removeMouseListener(mouseListener);

		super.uninstallUI(c);
	}

	public Dimension getMaximumSize(JComponent c) {
		if (menu.isTopLevelMenu()) {
			return getMinimumSize(c);
		}
		else {
			return itemUI.getMaximumSize(c);
		}
	}

	public Dimension getMinimumSize(JComponent c) {
		if (menu.isTopLevelMenu()) {
			return getPreferredSize(c);
		}
		else {
			return itemUI.getMinimumSize(c);
		}
	}

	public Dimension getPreferredSize(JComponent c) {
		Dimension d = super.getPreferredSize(c);

		if (menu.isTopLevelMenu()) {
			String text = menu.getText();
			Font font = menu.getFont();
		   	FontMetrics fm = menu.getFontMetrics(font);

			d.width = SwingUtilities.computeStringWidth(fm, text);
			d.height = fm.getHeight();

			d.width += engine.metricSize[MT_MENUBAR_ITEM_BORDER].width * 2 + 8;
			d.height += engine.metricSize[MT_MENUBAR_ITEM_BORDER].height * 2;
		}
		else {
			if (d.height <= 18) d.height -= 3;

			d.width += engine.metricSize[MT_MENU_ITEM_BORDER].width * 2;
			d.height += engine.metricSize[MT_MENU_ITEM_BORDER].height * 2;

			if (d.height < 18) d.height = 18;
		}
		return d;
	}

	public void paint(Graphics g, JComponent c) {
		if (!menu.isTopLevelMenu()) {
			itemUI.paint(g, c);
		}
		else {
			int state = Utils.getState(menu);
			int x = engine.metricSize[MT_MENUBAR_ITEM_BORDER].width + 4;
			int y = engine.metricSize[MT_MENUBAR_ITEM_BORDER].height;

			ButtonModel model = menu.getModel();

			if (model.isSelected()) {
				state |= MT_ACTIVE | MT_SELECTED;
			}

			g.setColor(engine.palette[MT_BACKGROUND]);
			g.fillRect(0, 0, menu.getWidth(), menu.getHeight());

			engine.drawWidget(g, MT_MENUBAR_ITEM, state, 0, 0, menu.getWidth(), menu.getHeight(), c);

			if ((state & MT_SELECTED) != 0) {
				x += engine.metricSize[MT_MENUBAR_ITEM_TEXT_OFFSET].width;
				y += engine.metricSize[MT_MENUBAR_ITEM_TEXT_OFFSET].height;
			}

			engine.drawString(g, MT_MENUBAR_ITEM, state, x, y, menu.getText(), engine.palette[Utils.getTextColor(state)], menu.getDisplayedMnemonicIndex());
		}
	}

 	protected class MouseListener extends MouseAdapter {

		public void mouseEntered(MouseEvent e) {
			AbstractButton b = (AbstractButton)e.getComponent();
			ButtonModel model = b.getModel();
			model.setRollover(true);
		}

		public void mouseExited(MouseEvent e) {
			AbstractButton b = (AbstractButton)e.getComponent();
			ButtonModel model = b.getModel();
			model.setRollover(false);
		}

	}

}
