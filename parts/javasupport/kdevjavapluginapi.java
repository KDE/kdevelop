/***************************************************************************
                            kdevjavapluginapi.java -  description
                             -------------------
    begin                : Thurs May 31 11:00:00 2001
    copyright            : (C) 2001 Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import java.net.*;
import java.lang.reflect.*;
import org.kde.koala.Core;

/** Loads the JNI function libraries, and provides a means of adding to the
	directories searched for classes at runtime.

	@author Richard_Dale@tipitina.demon.co.uk
*/
class kdevjavapluginapi  {

	/** Add a URL corresponding to the path of a .jar file within a KPart
		resource directory. Reflection is used to invoke the URLClassLoader.addURL()
		method, as it's protected.
	*/
	public static void addURLString(String url) {
		ClassLoader classLoader = kdevjavapluginapi.class.getClassLoader();
		Class classLoaderClass = classLoader.getClass();
		Method addURLMethod = null;

		do {
			try {
				addURLMethod = classLoaderClass.getDeclaredMethod("addURL", new Class[] { URL.class } );
			} catch (NoSuchMethodException e) {
				classLoaderClass = classLoaderClass.getSuperclass();
    		}
		} while (addURLMethod == null && classLoaderClass != null);

		try {
			addURLMethod.setAccessible(true);
			addURLMethod.invoke(classLoader, new Object[] { new URL(url) });
		} catch (MalformedURLException e) {
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			e.printStackTrace();
			return;
		} catch (IllegalAccessException e) {
			e.printStackTrace();
			return;
   		}

		return;
	}

	static {
		try {
			Class c = Class.forName("org.kde.qt.qtjava");
		} catch (Exception e) {
			System.out.println("Can't load qtjava class");
		}

		try {
			Class c = Class.forName("org.kde.koala.kdejava");
		} catch (Exception e) {
			System.out.println("Can't load kdejava class");
		}

		System.loadLibrary("kdevjavapluginapi");
		Core.setJavaSlotFactory();
	}
}
