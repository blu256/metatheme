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
import javax.swing.plaf.basic.BasicGraphicsUtils;

public final class Utils implements MT {

	private Utils() {}

	public static int getState(Component c) {
		int state = MT_NORMAL;

		if (c.isFocusOwner()) state |= MT_SELECTED;
		if (!c.isEnabled()) state |= MT_DISABLED;

		if (c instanceof AbstractButton) {
			AbstractButton b = (AbstractButton)c;
			ButtonModel model = b.getModel();

			if (c instanceof JMenuItem) {
				if (model.isArmed()) state |= MT_SELECTED;
				if (model.isSelected()) state |= MT_ACTIVE;

				if (c instanceof JMenu) {
					if ((state & MT_ACTIVE) != 0) state ^= MT_ACTIVE | MT_SELECTED;
				}
			}
			else if (c instanceof JRadioButton || c instanceof JCheckBox || c instanceof JToggleButton) {
				if (model.isArmed()) state |= MT_SELECTED;
				if (model.isSelected()) state |= MT_ACTIVE;
				if (model.isPressed()) state |= MT_MOUSE_ACTIVE;

				if (c instanceof JToggleButton) {
					if ((state & MT_MOUSE_ACTIVE) != 0) state |= MT_ACTIVE;
				}
			}
			else {
				if (model.isArmed() && model.isPressed()) state |= MT_ACTIVE | MT_SELECTED;
			}

			if (!model.isEnabled()) state |= MT_DISABLED;
			if (model.isRollover()) state |= MT_HOVER;
		}

		return state;
	}

	public static int getTextColor(int state) {
		return getTextColor(state, MT_FOREGROUND, false);
	}

	public static int getTextColor(int state, int baseColor) {
		return getTextColor(state, baseColor, false);
	}

	public static int getTextColor(int state, int baseColor, boolean sensitiveToSelected) {
		int c = baseColor;

		if ((state & MT_DISABLED) != 0) {
			c = MT_DISABLED_FOREGROUND;
		}
		else if (sensitiveToSelected && ((state & MT_SELECTED) != 0)) {
			c = MT_SELECTED_FOREGROUND;
		}

		return c;
	}

	public static void paintFocusRect(Graphics g, Rectangle r) {
		BasicGraphicsUtils.drawDashedRect(g, r.x - 1, r.y, r.width + 2, r.height);
	}

}
