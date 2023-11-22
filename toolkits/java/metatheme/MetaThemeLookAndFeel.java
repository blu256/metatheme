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
import javax.swing.UIManager;
import java.awt.*;
import java.awt.event.*;
import javax.swing.plaf.ColorUIResource;
import javax.swing.plaf.InsetsUIResource;
import javax.swing.LookAndFeel;
import javax.swing.SwingUtilities;
import javax.swing.Timer;

public final class MetaThemeLookAndFeel extends javax.swing.plaf.metal.MetalLookAndFeel implements MT {

	private boolean initialized = false;
	private MetaThemeDefaultTheme theme;
	private Object fontNormal;
	private Object fontBold;

	private ThemeEngine engine;

	private Timer reloadTimer = null;

	public MetaThemeLookAndFeel() {
	}

	public String getDescription() {
		return "MetaTheme";
	}

	public String getID() {
		return "MetaThemeLookAndFeel";
	}

	public String getName() {
		return "MetaThemeLookAndFeel";
	}

	public boolean getSupportsWindowDecorations() {
		return true;
	}

	public void initialize() {
		try {
			engine = new ThemeEngine();
			initialized = true;
		}
		catch (IllegalStateException e) {
			System.err.println(e + ", reverting to Metal look'n'feel");
			initialized = false;
			return;
		}
		finally {
			super.initialize();
		}

		fontNormal = new UIDefaults.ProxyLazyValue("javax.swing.plaf.FontUIResource", null,
			new Object[] {engine.fontName, new Integer(Font.PLAIN), new Integer(engine.fontSize)}
		);

		fontBold = new UIDefaults.ProxyLazyValue("javax.swing.plaf.FontUIResource", null,
			new Object[] {engine.fontName, new Integer(Font.BOLD), new Integer(engine.fontSize)}
		);

		theme = new MetaThemeDefaultTheme();

		theme.selected = new ColorUIResource(engine.palette[MT_SELECTED_FOREGROUND]);

		theme.primary1 = new ColorUIResource(engine.palette[MT_BACKGROUND]);
		theme.primary2 = new ColorUIResource(engine.palette[MT_SELECTED_BACKGROUND]);
		theme.primary3 = new ColorUIResource(engine.palette[MT_SELECTED_BACKGROUND]);

		theme.secondary1 = new ColorUIResource(engine.palette[MT_BUTTON_MID]);
		theme.secondary2 = new ColorUIResource(engine.palette[MT_DISABLED_FOREGROUND]);
		theme.secondary3 = new ColorUIResource(engine.palette[MT_BACKGROUND]);
	}

	public boolean isNativeLookAndFeel() {
		return true;
	}

	public boolean isSupportedLookAndFeel() {
		return true;
	}

	public void uninitialize() {
		if (initialized == true) {
			engine.destroy();
			initialized = false;
		}
		super.uninitialize();

		if (reloadTimer != null) {
			reloadTimer.stop();
		}

		reloadTimer = new Timer(1000, new ActionListener() {
			private int ticks = 0;

			public void actionPerformed(ActionEvent evt) {
				try {
					// abort when it was changed to MetaTheme by the application:
					LookAndFeel laf = UIManager.getLookAndFeel();
					if (laf != null && laf.getClass().getName().equals("metatheme.MetaThemeLookAndFeel")) {
						reloadTimer.stop();
						reloadTimer = null;
						return;
					}

					if (ticks == 0) {
						//System.out.println("metatheme was unloaded, forcing reload...");
						UIManager.setLookAndFeel("metatheme.MetaThemeLookAndFeel");
					}

					// update all frames:
					Frame[] frames = Frame.getFrames();
					for (int i=0; i<frames.length; i++) {
						SwingUtilities.updateComponentTreeUI(frames[i]);
						
						// update all dialogs:
						Window[] windows = frames[i].getOwnedWindows();
						for (int j=0; j<windows.length; j++) {
							SwingUtilities.updateComponentTreeUI(windows[j]);
						}
					}
				}
				catch (Exception e) {
					e.printStackTrace();
				}

				ticks++;
				if (ticks == 3) {
					reloadTimer.stop();
					reloadTimer = null;
				}
			}
		});

		reloadTimer.setInitialDelay(200);
		reloadTimer.start();
	}

	public UIDefaults getDefaults() {
		UIDefaults d = super.getDefaults();
		if (initialized == false) return d;

		d.put("MetaThemeEngine", engine);

		d.put("ButtonUI", "metatheme.MetaThemeButtonUI");
		d.put("Button.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getButtonBorder"));

		d.put("ToggleButtonUI", "metatheme.MetaThemeToggleButtonUI");
		d.put("ToggleButton.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getButtonBorder"));

		d.put("MenuBar.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders$MenuBarBorder"));

		d.put("ScrollBarUI", "metatheme.MetaThemeScrollBarUI");

		d.put("TextFieldUI", "metatheme.MetaThemeTextFieldUI");
		d.put("TextField.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getTextFieldBorder"));
		d.put("PasswordField.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getTextFieldBorder"));
		d.put("FormattedTextField.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getTextFieldBorder"));

		d.put("PopupMenu.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getPopupMenuBorder"));

		d.put("MenuItemUI", "metatheme.MetaThemeMenuItemUI");
		d.put("CheckBoxMenuItemUI", "metatheme.MetaThemeMenuItemUI");
		d.put("RadioButtonMenuItemUI", "metatheme.MetaThemeMenuItemUI");
		d.put("MenuUI", "metatheme.MetaThemeMenuUI");
		d.put("MenuItem.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getMenuItemBorder"));
		d.put("Menu.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getMenuItemBorder"));

		d.put("PopupMenuSeparatorUI", "metatheme.MetaThemePopupMenuSeparatorUI");

		d.put("ScrollPane.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getScrollPaneBorder"));
		d.put("TextPane.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getScrollPaneBorder"));

		d.put("TabbedPaneUI", "metatheme.MetaThemeTabbedPaneUI");
		d.put("TabbedPane.tabInsets", new InsetsUIResource(3, 8, 4, 8));
		d.put("TabbedPane.tabAreaInsets", new InsetsUIResource(0, 0, 0, 0));

		d.put("RadioButtonUI", "metatheme.MetaThemeRadioButtonUI");
		d.put("RadioButton.icon", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeIconFactory", "getRadioButtonIcon"));

		d.put("CheckBoxUI", "metatheme.MetaThemeCheckBoxUI");
		d.put("CheckBox.icon", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeIconFactory", "getCheckBoxIcon"));

		d.put("ToolBar.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders$ToolBarBorder"));
		d.put("ToolBar.rolloverBorder", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getToolBarItemBorder"));
		d.put("ToolBar.nonrolloverBorder", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getToolBarItemBorder"));
		d.put("ToolBar.isRollover", Boolean.TRUE);

		d.put("TitledBorder.border", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getTitledBorder"));

		d.put("InternalFrame.border", new UIDefaults.ProxyLazyValue("javax.swing.plaf.basic.BasicBorders", "getInternalFrameBorder"));
		d.put("DesktopIcon.border", new UIDefaults.ProxyLazyValue("javax.swing.plaf.basic.BasicBorders", "getInternalFrameBorder"));
		d.put("InternalFrameUI", "javax.swing.plaf.basic.BasicInternalFrameUI");
		d.put("RootPaneUI", "javax.swing.plaf.basic.BasicRootPaneUI");
		d.put("DesktopPaneUI", "javax.swing.plaf.basic.BasicDesktopPaneUI");
		d.put("DesktopIconUI", "javax.swing.plaf.basic.BasicDesktopIconUI");

		d.put("ToolTip.border", new UIDefaults.ProxyLazyValue("javax.swing.plaf.BorderUIResource$LineBorderUIResource", new Object[] { Color.BLACK }));

		d.put("ComboBoxUI", "metatheme.MetaThemeComboBoxUI");
		d.put("ComboBox.background", engine.palette[MT_WINDOW_BACKGROUND]);
		d.put("ComboBox.selectionForeground", engine.palette[MT_SELECTED_FOREGROUND]);

		d.put("Table.scrollPaneBorder", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeBorders", "getScrollPaneBorder"));
		d.put("Table.gridColor", engine.palette[MT_BUTTON_MIDLIGHT]);
		d.put("TableHeaderUI", "metatheme.MetaThemeTableHeaderUI");

		d.put("ProgressBarUI", "metatheme.MetaThemeProgressBarUI");
		Dimension dim = engine.metricSize[MT_PROGRESSBAR_BORDER];
		d.put("ProgressBar.border", new UIDefaults.ProxyLazyValue("javax.swing.plaf.BorderUIResource$EmptyBorderUIResource", new Object[] { new Insets(dim.height, dim.width, dim.height, dim.width) }));

		d.put("SliderUI", "metatheme.MetaThemeSliderUI");
		d.put("Slider.horizontalThumbIcon", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeIconFactory", "getHorizontalSliderThumbIcon"));
		d.put("Slider.verticalThumbIcon", new UIDefaults.ProxyLazyValue("metatheme.MetaThemeIconFactory", "getVerticalSliderThumbIcon"));

		d.put("SplitPaneUI", "metatheme.MetaThemeSplitPaneUI");
		d.put("SplitPane.dividerSize", new Integer(engine.metric[MT_SPLITTER_WIDTH]));
		d.put("SplitPane.supportsOneTouchButtons", Boolean.FALSE);
		d.put("SplitPane.border", new UIDefaults.ProxyLazyValue("javax.swing.plaf.BorderUIResource$EmptyBorderUIResource", new Object[] { new Insets(0, 0, 0, 0) }));
		d.put("SplitPaneDivider.border", new UIDefaults.ProxyLazyValue("javax.swing.plaf.BorderUIResource$EmptyBorderUIResource", new Object[] { new Insets(0, 0, 0, 0) }));

		d.put("Button.font", fontNormal);
		d.put("ToggleButton.font", fontNormal);
		d.put("RadioButton.font", fontNormal);
		d.put("CheckBox.font", fontNormal);
		d.put("ColorChooser.font", fontNormal);
		d.put("ComboBox.font", fontNormal);
		d.put("Label.font", fontNormal);
		d.put("List.font", fontNormal);
		d.put("MenuBar.font", fontNormal);
		d.put("MenuItem.font", fontNormal);
		d.put("RadioButtonMenuItem.font", fontNormal);
		d.put("CheckBoxMenuItem.font", fontNormal);
		d.put("Menu.font", fontNormal);
		d.put("PopupMenu.font", fontNormal);
		d.put("OptionPane.font", fontNormal);
		d.put("Panel.font", fontNormal);
		d.put("ProgressBar.font", fontNormal);
		d.put("ScrollPane.font", fontNormal);
		d.put("Viewport.font", fontNormal);
		d.put("Spinner.font", fontNormal);
		d.put("TabbedPane.font", fontNormal);
		d.put("Table.font", fontNormal);
		d.put("TableHeader.font", fontNormal);
		d.put("TextField.font", fontNormal);
		d.put("FormattedTextField.font", fontNormal);
		d.put("PasswordField.font", fontNormal);
		d.put("TextArea.font", fontNormal);
		d.put("TextPane.font", fontNormal);
		d.put("EditorPane.font", fontNormal);
		d.put("TitledBorder.font", fontNormal);
		d.put("ToolBar.font", fontNormal);
		d.put("ToolTip.font", fontNormal);
		d.put("Tree.font", fontNormal);
		d.put("InternalFrame.titleFont", fontBold);

		return d;
	}

	protected void createDefaultTheme() {
		if (initialized == false) {
			super.createDefaultTheme();
			return;
		}
		
		setCurrentTheme(theme);
	}

	protected void initSystemColorDefaults(UIDefaults d) {
		super.initSystemColorDefaults(d);
		if (initialized == false) return;

		d.put("info", new Color(255, 255, 225));
		d.put("infoText", Color.BLACK);

		d.put("desktop",             engine.palette[MT_BUTTON_MID]);
		d.put("activeCaption",       engine.palette[MT_SELECTED_BACKGROUND]);
		d.put("activeCaptionText",   engine.palette[MT_SELECTED_FOREGROUND]);
		d.put("activeCaptionBorder", engine.palette[MT_BACKGROUND]);
		d.put("window",              engine.palette[MT_WINDOW_BACKGROUND]);
		d.put("windowBorder",        engine.palette[MT_BACKGROUND]);

		d.put("control",             engine.palette[MT_BACKGROUND]);
		d.put("controlText",         engine.palette[MT_FOREGROUND]);
		d.put("controlHighlight",    engine.palette[MT_BUTTON_MIDLIGHT]);
		d.put("controlLtHighlight",  engine.palette[MT_BUTTON_LIGHT]);
		d.put("controlShadow",       engine.palette[MT_BUTTON_MID]);
		d.put("controlDkShadow",     engine.palette[MT_BUTTON_SHADOW]);
	}

}
