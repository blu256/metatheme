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
import javax.swing.plaf.*;
import javax.swing.plaf.metal.MetalSliderUI;

public class MetaThemeSliderUI extends MetalSliderUI implements MT {

	private ThemeEngine engine;
	private JComponent component;

	public MetaThemeSliderUI(JComponent c) {
		super();
		engine = ThemeEngine.get();
		component = c;
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeSliderUI(c);
	}

	public void paintThumb(Graphics g) {
		int state = Utils.getState(slider);
		int orientation = (slider.getOrientation() == JSlider.HORIZONTAL)? MT_HORIZONTAL : MT_VERTICAL;

		engine.drawWidget(g, MT_SCALE_HANDLE, state, thumbRect.x, thumbRect.y, thumbRect.width, thumbRect.height, component, 0, orientation);
	}

	public void paintTrack(Graphics g) {
		int state = Utils.getState(slider);
		int orientation = (slider.getOrientation() == JSlider.HORIZONTAL)? MT_HORIZONTAL : MT_VERTICAL;

		if (orientation == MT_HORIZONTAL) {
			engine.drawWidget(g, MT_SCALE, state, trackRect.x - trackBuffer, trackRect.y, trackRect.width + trackBuffer*2, trackRect.height, component, 0, orientation);
		}
		else {
			engine.drawWidget(g, MT_SCALE, state, trackRect.x, trackRect.y - trackBuffer, trackRect.width, trackRect.height + trackBuffer*2, component, 0, orientation);
		}
	}

}
