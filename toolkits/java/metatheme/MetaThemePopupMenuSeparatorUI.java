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
import javax.swing.plaf.metal.MetalPopupMenuSeparatorUI;

public class MetaThemePopupMenuSeparatorUI extends MetalPopupMenuSeparatorUI implements MT {

	private ThemeEngine engine;

	public MetaThemePopupMenuSeparatorUI() {
		super();
		engine = ThemeEngine.get();
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemePopupMenuSeparatorUI();
	}

	public void paint(Graphics g, JComponent c) {
		Dimension size = c.getSize();
		int state = Utils.getState(c);

		engine.drawWidget(g, MT_MENU_ITEM_SEPARATOR, state, 0, 0, size.width, size.height, c);
	}

	public Dimension getPreferredSize(JComponent c) {
		return new Dimension(0, engine.metric[MT_MENU_SEPARATOR_HEIGHT]);
	}

}
