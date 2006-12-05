/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef _MISC_H_
#define _MISC_H_

#include <qcstring.h>
#include <qmap.h>
#include <qwidget.h>

/**
 * Very small helper class. It has just static methods.
 */
class AutoProjectTool
{
public:

	/**
	 * Loads the compiler options plugin for the given compiler, executes the dialog
	 * with some initial flags, and returns the new flags.
	 */
	static QString execFlagsDialog( const QString &compiler, const QString &flags, QWidget *parent );

	/**
	 * Returns the canonicalized version of a file name, i.e.
	 * the file name with special characters replaced by underscores
	 */
	static QString canonicalize( const QString &str );

	/**
	 * Parses a Makefile.am and stores its variable assignments
	 * in a map.
	 */
	static void parseMakefileam( const QString &filename, QMap<QString, QString> *variables );

	static void addToMakefileam( const QString &filename, QMap<QString, QString> variables );
	static void removeFromMakefileam( const QString &filename, QMap<QString, QString> variables );
	static void setMakefileam ( const QString &fileName, QMap<QString, QString> variables );

	static void addRemoveMakefileam(const QString &fileName, QMap<QString, QString> variables,  bool add);

	/**
	 * Parses configure.in and splits AC_OUTPUT into a QStringList
	 */
	static QStringList configureinLoadMakefiles( QString configureinpath );

	/**
	 * Receives a QStringList and puts it into
	 * configure.in as arguments to AC_OUTPUT
	 */
	static void configureinSaveMakefiles( QString configureinpath, QStringList makefiles );

};

#endif
// kate: indent-mode csands; tab-width 4;

