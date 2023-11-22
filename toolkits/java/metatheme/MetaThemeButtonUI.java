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
import javax.swing.plaf.metal.MetalButtonUI;

public class MetaThemeButtonUI extends MetalButtonUI implements MT {

	private final static MetaThemeButtonUI buttonUI = new MetaThemeButtonUI();

	private ThemeEngine engine;
	private MouseListener mouseListener;

	public MetaThemeButtonUI() {
		super();
		engine = ThemeEngine.get();
	}

	public static ComponentUI createUI(JComponent c) {
		return buttonUI;
	}

	public Dimension getMinimumSize(JComponent c) {
		return super.getMinimumSize(c);
	}

	protected int getTextShiftOffset() {
		return 0;
	}

	public void paint(Graphics g, JComponent c) {
		Dimension size = c.getSize();
		int type = MT_BUTTON;
		int state = Utils.getState(c);

		if (c.getParent() instanceof JToolBar) {
			type = MT_TOOLBAR_ITEM;
		}

		engine.drawWidget(g, type, state, 0, 0, size.width, size.height, c);

		if ((state & MT_ACTIVE) != 0) {
			g.translate(engine.metricSize[MT_BUTTON_TEXT_OFFSET].width, engine.metricSize[MT_BUTTON_TEXT_OFFSET].height);
		}

		super.paint(g, c);

		if ((state & MT_ACTIVE) != 0) {
			g.translate(-engine.metricSize[MT_BUTTON_TEXT_OFFSET].width, -engine.metricSize[MT_BUTTON_TEXT_OFFSET].height);
		}
	}

	protected void paintButtonPressed(Graphics g, AbstractButton b) {
	}

	protected void paintFocus(Graphics g, AbstractButton b, Rectangle viewRect, Rectangle textRect, Rectangle iconRect) {
	}

	protected void paintText(Graphics g, JComponent c, Rectangle textRect, String text) {
		AbstractButton b = (AbstractButton)c;
		ButtonModel model = b.getModel();
		int mnemIndex = b.getDisplayedMnemonicIndex();
		int type = MT_BUTTON;
		int state = Utils.getState(c);

		if (c.getParent() instanceof JToolBar) {
			type = MT_TOOLBAR_ITEM;
		}

		engine.drawString(g, type, state, textRect.x, textRect.y, text, engine.palette[Utils.getTextColor(state, MT_BUTTON_FOREGROUND)], mnemIndex);
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
