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
import javax.swing.*;
import javax.swing.plaf.*;
import javax.swing.plaf.metal.MetalTextFieldUI;

public class MetaThemeTextFieldUI extends MetalTextFieldUI implements MT {

	private ThemeEngine engine;
	private JComponent component;
	private MouseListener mouseListener;

	public MetaThemeTextFieldUI(JComponent c) {
		super();
		engine = ThemeEngine.get();
		component = c;
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeTextFieldUI(c);
	}

	protected void paintBackground(Graphics g) {
		Dimension size = component.getSize();
		int state = Utils.getState(component);

		if ((state & MT_DISABLED) != 0) {
			g.setColor(engine.palette[MT_BACKGROUND]);
		}
		else {
			g.setColor(engine.palette[MT_WINDOW_BACKGROUND]);
		}
		g.fillRect(0, 0, size.width, size.height);

		engine.drawWidget(g, MT_ENTRY, state, 0, 0, size.width, size.height, component);
	}

	public void installUI(JComponent c) {
		super.installUI(c);

		/*
		mouseListener = new MouseListener();
		c.addMouseListener(mouseListener);
		*/
	}

	public void uninstallUI(JComponent c) {
		//c.removeMouseListener(mouseListener);

		super.uninstallUI(c);
	}

 	protected class MouseListener extends MouseAdapter {

		public void mousePressed(MouseEvent e) {
			if (e.getButton() == MouseEvent.BUTTON3) {
				JPopupMenu popup;
				JMenuItem menuItem;

				/* TODO: */

				popup = new JPopupMenu();

				menuItem = new JMenuItem("Cut");
				popup.add(menuItem);

				menuItem = new JMenuItem("Copy");
				popup.add(menuItem);

				menuItem = new JMenuItem("Paste");
				popup.add(menuItem);

				menuItem = new JMenuItem("Delete");
				popup.add(menuItem);

				popup.addSeparator();

				menuItem = new JMenuItem("Select all");
				popup.add(menuItem);

				popup.show(e.getComponent(), e.getX(), e.getY());
			}
		}

	}

}
