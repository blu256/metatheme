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

public class MetaThemeScrollButton extends JButton implements MT, UIResource {

	private ThemeEngine engine;
	private int type;
	private MetaThemeScrollBarUI ui = null;
	private MetaThemeTabbedPaneUI tabui = null;

	public MetaThemeScrollButton(int type, MetaThemeScrollBarUI ui) {
		super();
		engine = ThemeEngine.get();
		this.type = type;
		this.ui = ui;
	}

	public MetaThemeScrollButton(int type, MetaThemeTabbedPaneUI tabui) {
		super();
		engine = ThemeEngine.get();
		this.type = type;
		this.tabui = tabui;
	}

	public Dimension getPreferredSize() {
		if (tabui != null) {
			int size = engine.metric[MT_NOTEBOOK_ARROW_WIDTH];
			return new Dimension(size, tabui.getTabAreaHeight());
		}
		else {
			int size = engine.metric[MT_SCROLLBAR_SIZE];
			return new Dimension(size, size);
		}
	}

	public Dimension getMinimumSize() {
		return getPreferredSize();
	}

	public void paint(Graphics g) {
		int state = Utils.getState(this);

		if ((state & MT_HOVER) != 0 && ui != null && ui.isDragging()) {
			ui.redrawAfterDragging = true;
			state &= ~MT_HOVER;
		}

		engine.drawWidget(g, type, state, 0, 0, getWidth(), getHeight(), this);
	}

	public boolean isFocusable() {
		return false;
	}

}
