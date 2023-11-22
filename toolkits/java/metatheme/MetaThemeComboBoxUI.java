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
import javax.swing.plaf.basic.BasicComboBoxUI;
import javax.swing.plaf.metal.MetalComboBoxUI;
import javax.swing.plaf.metal.MetalComboBoxButton;
import javax.swing.plaf.metal.MetalComboBoxIcon;
import javax.swing.plaf.metal.MetalComboBoxEditor;
import java.beans.PropertyChangeEvent;

public class MetaThemeComboBoxUI extends MetalComboBoxUI implements MT {

	private ThemeEngine engine;

	public MetaThemeComboBoxUI() {
		super();
		engine = ThemeEngine.get();
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeComboBoxUI();
	}

    protected JButton createArrowButton() {
		JButton button = new ComboBoxButton(
			comboBox,
			new MetalComboBoxIcon(),
			comboBox.isEditable(),
			currentValuePane,
			listBox
		);
		button.setMargin(new Insets(0, 0, 0, 0));
		return button;
    }

	protected ComboBoxEditor createEditor() {
		return new MyComboBoxEditor();
	}

	protected LayoutManager createLayoutManager() {
		return new MyComboBoxLayoutManager();
	}

	public Dimension getMinimumSize(JComponent c) {
		Dimension d = super.getMinimumSize(c);
		if (comboBox.isEditable()) {
			d.height -= 4;
		}
		return d;
	}

	public class ComboBoxButton extends MetalComboBoxButton {

		public ComboBoxButton(JComboBox cb, Icon i, boolean onlyIcon, CellRendererPane pane, JList list) {
			super(cb, i, onlyIcon, pane, list);
		}

		public void paintComponent(Graphics g) {
			int state = Utils.getState(this);

			int x = 0;
			int y = 0;
			int width = getWidth();
			int height = getHeight();

			x = width - engine.metric[MT_CHOICE_BUTTON_WIDTH];
			width = engine.metric[MT_CHOICE_BUTTON_WIDTH];

			Shape clip = g.getClip();
			g.clipRect(x, 0, width, getHeight());
			engine.drawWidget(g, MT_CHOICE_BUTTON, state, x, 0, width, getHeight(), this);
			g.setClip(clip);

			if (iconOnly || comboBox == null) return;

			width = getWidth() - engine.metric[MT_CHOICE_BUTTON_WIDTH];
			g.clipRect(0, 0, width, getHeight());
			engine.drawWidget(g, MT_CHOICE, state, 0, 0, width, getHeight(), this);
			g.setClip(clip);

			ListCellRenderer renderer = comboBox.getRenderer();
			boolean renderPressed = getModel().isPressed();
			Component c = renderer.getListCellRendererComponent(listBox, comboBox.getSelectedItem(), -1, renderPressed, false);

			c.setFont(rendererPane.getFont());
			/*
			TODO:
			if (model.isArmed() && model.isPressed()) {
				if (isOpaque()) {
					c.setBackground(UIManager.getColor("Button.select"));
				}
				c.setForeground(comboBox.getForeground());
			}
			else if (!comboBox.isEnabled()) {
				if (isOpaque()) {
					c.setBackground(UIManager.getColor("ComboBox.disabledBackground"));
				}
				c.setForeground(UIManager.getColor("ComboBox.disabledForeground"));
			}
			else {
				*/
				c.setForeground(comboBox.getForeground());
				c.setBackground(comboBox.getBackground());
			//}

			x = engine.metric[MT_BORDER] + 1;
			y = engine.metric[MT_BORDER] + 1;
			width -= x*2 - 2;
			height -= y*2;

			rendererPane.paintComponent(g, c, this, x, y, width, height, false);
		}

	}

	public class MyComboBoxEditor extends MetalComboBoxEditor {

		public MyComboBoxEditor() {
			super();
			editor.setBorder(new MetaThemeBorders.ButtonBorder(MT_CHOICE, MT_BUTTON_BORDER));
		}

	}

	public class MyComboBoxLayoutManager extends BasicComboBoxUI.ComboBoxLayoutManager {

		public void layoutContainer(Container parent) {
			super.layoutContainer(parent);

			Dimension d = parent.getSize();
			Insets insets = getInsets();

			if (editor != null && arrowButton != null) {
				editor.setBounds(
					insets.left,
					insets.top,
					d.width - insets.left - insets.right - engine.metric[MT_CHOICE_BUTTON_WIDTH],
					d.height - insets.top - insets.bottom
				);

				arrowButton.setBounds(
					d.width - insets.right - engine.metric[MT_CHOICE_BUTTON_WIDTH],
					insets.top,
					engine.metric[MT_CHOICE_BUTTON_WIDTH], d.height - insets.top - insets.bottom
				);
			}
			else if (arrowButton != null) {
				arrowButton.setBounds(
					insets.left,
					insets.top,
					d.width - insets.left - insets.right,
					d.height - insets.top - insets.bottom
				);
			}
		}

	}

}
