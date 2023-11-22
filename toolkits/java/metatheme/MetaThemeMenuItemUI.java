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
import javax.swing.plaf.basic.BasicMenuItemUI;
import javax.swing.plaf.basic.BasicGraphicsUtils;

public class MetaThemeMenuItemUI extends BasicMenuItemUI implements MT {

	private ThemeEngine engine;
	private JMenuItem mi;
	private int type;

	public MetaThemeMenuItemUI(JMenuItem i) {
		super();
		engine = ThemeEngine.get();
		mi = i;

		type = MT_MENU_ITEM;
		if (mi instanceof JCheckBoxMenuItem) {
			type = MT_MENU_ITEM_CHECK;
		}
		else if (mi instanceof JRadioButtonMenuItem) {
			type = MT_MENU_ITEM_RADIO;
		}
		else if (mi instanceof JMenu) {
			type = MT_MENU_ITEM_ARROW;
		}
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeMenuItemUI((JMenuItem)c);
	}

	protected void paintMenuItem(Graphics g, JComponent c,
	                             Icon checkIcon, Icon arrowIcon,
	                             Color background, Color foreground,
	                             int defaultTextIconGap) {
		JMenuItem menuItem = (JMenuItem)c;
		ButtonModel model = menuItem.getModel();
		int x,y;
		int maxpmw = 16;

		Font font = menuItem.getFont();
		FontMetrics fm = menuItem.getFontMetrics(font);

		int state = Utils.getState(menuItem);

		paintBackground(g, menuItem, null);

		Icon icon = menuItem.getIcon();
		if (icon != null) {
			if (icon.getIconWidth() > maxpmw) {
				maxpmw = icon.getIconWidth();
			}

			if ((state & MT_DISABLED) != 0) {
				icon = (Icon)menuItem.getDisabledIcon();
			}
			else if (model.isPressed() && model.isArmed()) {
				icon = (Icon)menuItem.getPressedIcon();
				if (icon == null) {
					icon = (Icon)menuItem.getIcon();
				}
			} else {
				icon = (Icon)menuItem.getIcon();
			}

		}

		if (type == MT_MENU_ITEM_CHECK || type == MT_MENU_ITEM_RADIO) {
			if ((state & MT_ACTIVE) != 0) {
				x = engine.metricSize[MT_MENU_BORDER].width;
				y = engine.metricSize[MT_MENU_BORDER].height;
				engine.drawWidget(g, type, state, x, y, maxpmw, menuItem.getHeight() - 2*y, c);
			}
		}
		else {
			if (icon != null) {
				y = (menuItem.getHeight() - icon.getIconHeight() - 1) / 2;
				icon.paintIcon(menuItem, g, engine.metricSize[MT_MENU_ITEM_BORDER].width, y);
			}
		}

		String text = menuItem.getText();
		if (text != null) {
			g.setFont(font);
			paintText(g, menuItem, new Rectangle(0, 0, menuItem.getWidth(), menuItem.getHeight()), text);
		}

		KeyStroke accelerator = menuItem.getAccelerator();
		String acceleratorText = "";
		if (accelerator != null) {
			int modifiers = accelerator.getModifiers();

			if (modifiers > 0) {
				acceleratorText = KeyEvent.getKeyModifiersText(modifiers);
				acceleratorText += /*acceleratorDelimiter*/"-";
			}

			int keyCode = accelerator.getKeyCode();
			if (keyCode != 0) {
				acceleratorText += KeyEvent.getKeyText(keyCode);
			} else {
				acceleratorText += accelerator.getKeyChar();
			}
		}

		if (acceleratorText != null && !acceleratorText.equals("")) {
			x = menuItem.getWidth() - engine.metricSize[MT_MENU_ITEM_BORDER].width - 3;
			y = (menuItem.getHeight() - fm.getHeight() + 1) / 2;

			x -= SwingUtilities.computeStringWidth(fm, acceleratorText);
			g.setFont(font);
			
			engine.drawString(g, MT_MENU_ITEM, state, x, y, acceleratorText, engine.palette[Utils.getTextColor(state, MT_FOREGROUND, true)], -1);
		}

		if (type == MT_MENU_ITEM_ARROW) {
			x = menuItem.getWidth() - 1 - engine.metricSize[MT_MENU_ITEM_BORDER].width - 8;
			y = engine.metricSize[MT_MENU_ITEM_BORDER].height;

			engine.drawWidget(g, MT_MENU_ITEM_ARROW, state, x, y, 8, menuItem.getHeight() - 2*y, c);
		}
	}

	protected void paintBackground(Graphics g, JMenuItem menuItem, Color bgColor) {
		Dimension size = menuItem.getSize();
		int state = Utils.getState(menuItem);

		Color oldColor = g.getColor();
		engine.drawWidget(g, MT_MENU_ITEM, state, 0, 0, size.width, size.height, menuItem);
		g.setColor(oldColor);
	}

	protected void paintText(Graphics g, JMenuItem menuItem, Rectangle textRect, String text) {
		FontMetrics fm = g.getFontMetrics();
		int mnemIndex = menuItem.getDisplayedMnemonicIndex();
		int state = Utils.getState(menuItem);
		Rectangle tr = new Rectangle(textRect);
		int maxpmw = 16;

		Icon icon = menuItem.getIcon();
		if (icon != null && icon.getIconWidth() > maxpmw) maxpmw = icon.getIconWidth();

		tr.x += engine.metricSize[MT_MENU_ITEM_BORDER].width + maxpmw + 3;
		tr.y += (tr.height - fm.getHeight() + 1) / 2;

		int sc = MT_FOREGROUND;
		if ((state & MT_SELECTED) != 0) sc = MT_SELECTED_FOREGROUND;

		engine.drawString(g, MT_MENU_ITEM, state, tr.x, tr.y, text, engine.palette[Utils.getTextColor(state, MT_FOREGROUND, true)], mnemIndex);
	}

	protected Dimension getPreferredMenuItemSize(JComponent c, Icon checkIcon, Icon arrowIcon, int defaultTextIconGap) {
		Dimension d = super.getPreferredMenuItemSize(c, checkIcon, arrowIcon, defaultTextIconGap);

		if (d.height <= 18) d.height -= 3;

		d.width += engine.metricSize[MT_MENU_ITEM_BORDER].width * 2;
		d.height += engine.metricSize[MT_MENU_ITEM_BORDER].height * 2;

		if (d.height < 18) d.height = 18;

		return d;
	}

}
