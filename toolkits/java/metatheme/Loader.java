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

import java.lang.reflect.*;
import java.util.Iterator;
import java.util.jar.*;
import java.awt.EventQueue;
import javax.swing.UIManager;

public final class Loader {

	public static void main(String[] args) throws Exception {
		String mainClass = args[0];

		System.setProperty("swing.systemlaf", "metatheme.MetaThemeLookAndFeel");
		System.setProperty("swing.crossplatformlaf", "metatheme.MetaThemeLookAndFeel");
		UIManager.setLookAndFeel("metatheme.MetaThemeLookAndFeel");
		
		// obtain the name of main class from the JAR:
		if (mainClass.endsWith(".jar")) {
			JarFile jar = new JarFile(mainClass);
			mainClass = jar.getManifest().getMainAttributes().getValue("Main-Class");
		}
	
		// run the main class:
		Class cls = Class.forName(mainClass, true, Thread.currentThread().getContextClassLoader());
		Method m[] = cls.getDeclaredMethods();
		for (int i=0; i<m.length; i++) {
			if (m[i].getName().equals("main")) {
				try {
					m[i].setAccessible(true);
					m[i].invoke(null, new Object[] { new String[0] });
				}
				catch (InvocationTargetException e) {
					e.getCause().printStackTrace();
				}
				break;
			}
		}
	}

}
