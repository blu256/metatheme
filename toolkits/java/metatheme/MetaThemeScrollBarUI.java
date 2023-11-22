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
import java.awt.event.*;
import javax.swing.*;
import javax.swing.plaf.*;
import javax.swing.plaf.metal.MetalScrollBarUI;

public class MetaThemeScrollBarUI extends MetalScrollBarUI implements MT {

	private ThemeEngine engine;
	private MouseListener mouseListener;
	private JComponent component;
	public boolean redrawAfterDragging = false;
	protected boolean scrollByBlockUsed = false;

	public MetaThemeScrollBarUI(JComponent c) {
		super();
		engine = ThemeEngine.get();
		component = c;
	}

	public static ComponentUI createUI(JComponent c) {
		return new MetaThemeScrollBarUI(c);
	}

	public Dimension getPreferredSize(JComponent c) {
		int size = engine.metric[MT_SCROLLBAR_SIZE];

		if (scrollbar.getOrientation() == JScrollBar.VERTICAL) {
			return new Dimension(size, size * 3 + 10);
		}
		else {
			return new Dimension(size * 3 + 10, size);
		}
	}

	protected void paintTrack(Graphics g, JComponent c, Rectangle rect) {
		int state = MT_NORMAL;
		int type = MT_SCROLLBAR;
		int orientation = (scrollbar.getOrientation() == JScrollBar.VERTICAL)? MT_VERTICAL : MT_HORIZONTAL;
		Rectangle r = new Rectangle(rect);

		if (type == MT_SCROLLBAR) {
			Dimension decrSize = decrButton.getSize();
			Dimension incrSize = incrButton.getSize();

			if (orientation == MT_HORIZONTAL) {
				r.x -= decrSize.width;
				r.width += decrSize.width + incrSize.width;
			}
			else {
				r.y -= decrSize.height;
				r.height += decrSize.height + incrSize.height;
			}
		}

		int flags = 0;
		if (trackHighlight == DECREASE_HIGHLIGHT) {
			flags |= MT_SCROLLBAR_SUBPAGE_ACTIVE;
		}
		else if (trackHighlight == INCREASE_HIGHLIGHT) {
			flags |= MT_SCROLLBAR_ADDPAGE_ACTIVE;
		}

		int pos = (orientation == MT_HORIZONTAL)? (thumbRect.x - trackRect.x) : (thumbRect.y - trackRect.y);
		int size = (orientation == MT_HORIZONTAL)? thumbRect.width : thumbRect.height;

		engine.drawWidget(g, type, state, r.x, r.y, r.width, r.height, c, flags | (pos << 4), orientation | (size << 4));
	}

	protected void paintThumb(Graphics g, JComponent c, Rectangle r) {
		int state = MT_NORMAL;
		int orientation = (scrollbar.getOrientation() == JScrollBar.VERTICAL)? MT_VERTICAL : MT_HORIZONTAL;

		if (isThumbRollover()) state |= MT_HOVER;
		if (isDragging()) state |= MT_ACTIVE;

		int pos = (orientation == MT_HORIZONTAL)? (thumbRect.x - trackRect.x) : (thumbRect.y - trackRect.y);
		int size = (orientation == MT_HORIZONTAL)? trackRect.width : trackRect.height;

		engine.drawWidget(g, MT_SCROLLBAR_HANDLE, state, r.x, r.y, r.width, r.height, c, pos << 4, orientation | (size << 4));
	}

	protected Dimension getMinimumThumbSize() {
		int size = engine.metric[MT_SCROLLBAR_SIZE];
		return new Dimension(size / 2, size / 2);
	}

	protected JButton createDecreaseButton(int orientation) {
		return new MetaThemeScrollButton((orientation == NORTH)? MT_SCROLLBAR_ARROW_UP : MT_SCROLLBAR_ARROW_LEFT, this);
	}

	protected JButton createIncreaseButton(int orientation) {
		return new MetaThemeScrollButton((orientation == SOUTH)? MT_SCROLLBAR_ARROW_DOWN : MT_SCROLLBAR_ARROW_RIGHT, this);
	}

	public boolean isDragging() {
		return isDragging;
	}

	public void installUI(JComponent c) {
		super.installUI(c);

		mouseListener = new MouseListener();
		c.addMouseListener(mouseListener);
	}

   	public void uninstallUI(JComponent c) {
		c.removeMouseListener(mouseListener);

		super.uninstallUI(c);
	}

	protected void installListeners() {
		super.installListeners();
		scrollTimer = new MyTimer(60, scrollListener);
		scrollTimer.setInitialDelay(300);
	}
	
	protected ScrollListener createScrollListener() {
		return new ScrollListener();
	}

	protected TrackListener createTrackListener() {
		return new TrackListener();
	}

	protected void scrollByBlock(int direction) {
		super.scrollByBlock(direction);
		scrollByBlockUsed = true;
	}

 	protected class MouseListener extends MouseAdapter {

		public void mousePressed(MouseEvent e) {
			component.repaint();
		}

		public void mouseReleased(MouseEvent e) {
			if (redrawAfterDragging) {
				decrButton.repaint();
				incrButton.repaint();
				redrawAfterDragging = false;
			}
		}

	}

	protected class ScrollListener extends javax.swing.plaf.basic.BasicScrollBarUI.ScrollListener {
		
		private int dir = +1;
		
		public void setDirection(int direction) {
			super.setDirection(direction);
			this.dir = direction;
		}
		
		public void actionPerformed(ActionEvent e) {
			MyTimer t = (MyTimer)e.getSource();

			t.ignoreStop = true;
			super.actionPerformed(e);
			t.ignoreStop = false;

			boolean stop = false;
			if (dir > 0 && scrollbar.getValue()+scrollbar.getVisibleAmount() >= scrollbar.getMaximum()) {
				stop = true;
			}
			else if (dir < 0 && scrollbar.getValue() <= scrollbar.getMinimum()) {
				stop = true;
			}

			if (stop) {
				t.stop();
			}
		}
		
	}

	protected class TrackListener extends javax.swing.plaf.basic.BasicScrollBarUI.TrackListener {

		public void mousePressed(MouseEvent e) {
			scrollByBlockUsed = false;
			super.mousePressed(e);

			if (scrollTimer.isRunning()) return;

			if (scrollByBlockUsed) {
				scrollTimer.start();
			}
		}
		
	}

	private class MyTimer extends Timer {

		boolean ignoreStop = false;
		
		public MyTimer(int delay, ActionListener listener) {
			super(delay, listener);
		}

		public void stop() {
			if (ignoreStop) return;

			super.stop();
		}
		
	}

}
