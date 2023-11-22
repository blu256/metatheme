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
import javax.swing.plaf.metal.MetalCheckBoxUI;

public class MetaThemeCheckBoxUI extends MetalCheckBoxUI implements MT {

	private final static MetaThemeCheckBoxUI buttonUI = new MetaThemeCheckBoxUI();

	private ThemeEngine engine;
	private MouseListener mouseListener;

	public MetaThemeCheckBoxUI() {
		super();
		engine = ThemeEngine.get();
	}

	public static ComponentUI createUI(JComponent c) {
		return buttonUI;
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

	protected void paintFocus(Graphics g, Rectangle textRect, Dimension size) {
		Utils.paintFocusRect(g, textRect);
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
