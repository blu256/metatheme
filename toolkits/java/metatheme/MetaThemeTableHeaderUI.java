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
import javax.swing.border.EmptyBorder;
import javax.swing.plaf.*;
import javax.swing.plaf.basic.BasicTableHeaderUI;
import javax.swing.table.JTableHeader;
import javax.swing.table.DefaultTableCellRenderer;

public class MetaThemeTableHeaderUI extends BasicTableHeaderUI implements MT {

	private ThemeEngine engine;
	private JComponent component;

	public MetaThemeTableHeaderUI(JComponent c) {
		super();
		engine = ThemeEngine.get();
		component = c;
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeTableHeaderUI(c);
	}

	public void installUI(JComponent c) {
		super.installUI(c);

		if (header.getDefaultRenderer() instanceof UIResource) {
			header.setDefaultRenderer(new MyDefaultRenderer());
		}
	}

 	protected class MyDefaultRenderer extends DefaultTableCellRenderer implements UIResource {

		public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
			if (table != null) {
				JTableHeader header = table.getTableHeader();
				if (header != null) {
					setFont(header.getFont());
				}
			}

			setText((value == null)? "" : value.toString());
			setBorder(new EmptyBorder(3, 4, 3, 4));

			return this;
		}

		public void paint(Graphics g) {
			int state = Utils.getState(this);

			engine.drawWidget(g, MT_BUTTON_HEADER, state, 0, 0, getWidth(), getHeight(), component);

			super.paint(g);
		}

	}

}
