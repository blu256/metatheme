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

import javax.swing.UIDefaults;
import java.awt.Font;
import java.awt.Insets;
import javax.swing.plaf.ColorUIResource;
import javax.swing.plaf.metal.DefaultMetalTheme;

public final class MetaThemeDefaultTheme extends DefaultMetalTheme {

	public ColorUIResource primary1 = new ColorUIResource(255, 0, 255);
	public ColorUIResource primary2 = new ColorUIResource(255, 0, 255);
	public ColorUIResource primary3 = new ColorUIResource(255, 0, 255);

	public ColorUIResource secondary1 = new ColorUIResource(0, 255, 255);
	public ColorUIResource secondary2 = new ColorUIResource(0, 255, 255);
	public ColorUIResource secondary3 = new ColorUIResource(128, 255, 255);

	public ColorUIResource selected = new ColorUIResource(255, 0, 255);

	protected ColorUIResource getPrimary1() {
		return primary1;
	}

	protected ColorUIResource getPrimary2() {
		return primary2;
	}

	protected ColorUIResource getPrimary3() {
		return primary3;
	}

	protected ColorUIResource getSecondary1() {
		return secondary1;
	}

	protected ColorUIResource getSecondary2() {
		return secondary2;
	}

	protected ColorUIResource getSecondary3() {
		return secondary3;
	}

	public ColorUIResource getHighlightedTextColor() { return selected; }
	public ColorUIResource getMenuSelectedForeground() { return selected; }

}
