/***************************************************************************
*   Copyright (C) 2003 by Jonas B. Jacobi                                 *
*   j.jacobi@gmx.de                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef DOXYDOC_H
#define DOXYDOC_H

#include <qstring.h>
#include <qdom.h>
#include <qdir.h>
#include <qfile.h>
#include <list>

/**
This class is used for getting the description on functions, stored in xml files, which are created by Doxygen
@author Jonas B. Jacobi
@version 0.1
*/
class DoxyDoc
{
public:
	/**
		Constructs a new DoxyDoc object.
		@param dir all directories, which should be searched for files containing the Doxygen-documentation.
	*/
	DoxyDoc( const QStringList& dir );
	~DoxyDoc()
	{}
	;
	/**
		Get the documentation referring to a special function, therefor most parts of the functions signature have to be provided
		@param scope Scope of the function (e.g. name of class, it is member of, namespace etc ...)
		@param name Name of the function(just the name, no '(' etc. )
		@param type Return-type of the function
		@param arguments String containing the arguments of the function, separated by commas and _without_ the argument names
	*/
	QString functionDescription( const QString& scope, const QString& name, const QString& type, const QString& arguments );
	/*not implemented yet, maybe later, if they are useful
	QString classDescription();
	QString enumDescription();
	QString namespaceDescription();
	QString memberVarDescription();
	*/
private:
	static void formatType( QString& type );
	std::list<QDir> m_dirs;
	//used for temporary storing/performance reasons
	QFile m_file;
	QDomNodeList m_list;
};

#endif 
// kate: indent-mode csands; tab-width 4;

