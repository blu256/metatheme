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

import java.io.Serializable;
import java.awt.*;
import javax.swing.*;
import javax.swing.plaf.*;
import javax.swing.plaf.metal.MetalIconFactory;

public class MetaThemeIconFactory extends MetalIconFactory implements MT {

    private static Icon checkBoxIcon;
    private static Icon radioButtonIcon;
    private static Icon horizontalSliderThumbIcon;
    private static Icon verticalSliderThumbIcon;

	public static Icon getCheckBoxIcon() {
		if (checkBoxIcon == null) {
			checkBoxIcon = new CheckBoxIcon(ThemeEngine.get(), MT_CHECK_BOX);
		}
		return checkBoxIcon;
	}

	public static Icon getRadioButtonIcon() {
		if (radioButtonIcon == null) {
			radioButtonIcon = new CheckBoxIcon(ThemeEngine.get(), MT_RADIO_BOX);
		}
		return radioButtonIcon;
	}

	public static Icon getHorizontalSliderThumbIcon() {
		if (horizontalSliderThumbIcon == null) {
			horizontalSliderThumbIcon = new SliderThumbIcon(ThemeEngine.get(), true);
		}
		return horizontalSliderThumbIcon;
	}

	public static Icon getVerticalSliderThumbIcon() {
		if (verticalSliderThumbIcon == null) {
			verticalSliderThumbIcon = new SliderThumbIcon(ThemeEngine.get(), false);
		}
		return verticalSliderThumbIcon;
	}

	private static class CheckBoxIcon implements Icon, UIResource, Serializable {

		private ThemeEngine engine;
		private int type;

		public CheckBoxIcon(ThemeEngine engine, int type) {
			this.engine = engine;
			this.type = type;
		}

		public void paintIcon(Component c, Graphics g, int x, int y) {
			int state = Utils.getState(c);

			engine.drawWidget(g, type, state, x, y, engine.metric[MT_CHECKBOX_SIZE], engine.metric[MT_CHECKBOX_SIZE], c);
		}

		public int getIconWidth() {
			return engine.metric[MT_CHECKBOX_SIZE];
		}

		public int getIconHeight() {
			return engine.metric[MT_CHECKBOX_SIZE];
		}
	}

	private static class SliderThumbIcon implements Icon, UIResource, Serializable {

		private ThemeEngine engine;
		private boolean horizontal;

		public SliderThumbIcon(ThemeEngine engine, boolean horizontal) {
			this.engine = engine;
			this.horizontal = horizontal;
		}

		public void paintIcon(Component c, Graphics g, int x, int y) {
		}

		public int getIconWidth() {
			return (horizontal)? engine.metric[MT_SCALE_SLIDER_SIZE] : 20;
		}

		public int getIconHeight() {
			return (horizontal)? 20 : engine.metric[MT_SCALE_SLIDER_SIZE];
		}
	}

}
