/*
  KDevelop Autotools Support
  Copyright (c) 2005 by Matt Rogers <mattr@kde.org>
 
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
*/
#ifndef MAKEFILEHANDLER_H
#define MAKEFILEHANDLER_H

/**
 * This class is responsible for taking the data from the Makefile.am parser 
 * and providing various list view items for use in the automake manager 
 * widget
 * @author Matt Rogers
 */
class MakefileHandler
{
public:
    MakefileHandler();
    ~MakefileHandler();
	
	/**
	 * Parse a folder that has supported makefiles in it.
	 * \param folder, the folder to parse
	 * \param recursive if true, subfolders will be parsed - defaults to true
	 */
	void parse( const QString& folder, bool recursive = true );
	
	/**
	 * Get the AST for a certain path
	 * \param folderPath the path of the folder to get an AST for
	 */
	void astForFolder( const QString& folderPath );


private:
	class Private;
	Private* d;
};

#endif

//kate: space-indent off; tab-width 4;

