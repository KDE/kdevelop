/***************************************************************************
                            JavaSupportFactory.java -  description
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

import org.kde.qt.*;
import org.kde.koala.*;
import java.util.*;
import java.lang.Error;

public class JavaSupportFactory extends KDevFactory  {
	protected JavaSupportFactory(Class dummy){super((Class) null);}

	private native void newJavaSupportFactory( QObject parent, String name);
	public JavaSupportFactory( QObject parent, String name) {
		super((Class) null);
		newJavaSupportFactory( parent, name);
	}
	private native void newJavaSupportFactory();
	public JavaSupportFactory() {
		super((Class) null);
		newJavaSupportFactory();
	}
	protected native void finalize() throws InternalError;
	public native KDevPart createPartObject( KDevApi api, QObject parent, String[] args);
	public static native KInstanceInterface instance();
}
