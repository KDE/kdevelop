/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   Copyright (c) 2005 by Matt Rogers                                     *
 *   mattr@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef QMAKEQMAKEDRIVER_H
#define QMAKEQMAKEDRIVER_H

#include <qlist.h>
#include <kurl.h>

namespace AutoTools {

/**
@file autotoolsdriver.h
Driver for a qmake parser.
*/

class ProjectAST;

/**
 * Driver.
 * Use methods of this class to lauch parsing and build the AST.
 */
class Driver
{
public:
	/**
	 * Parses the file @p fileName and stores the resulting ProjectAST root
	 * into @p ast. @p ast should not be initialized before. Driver will
	 * initialize it on its own.
	 * @return The result of parsing. Result is 0 on success and <> 0 on failure.
	 */
	static int parseFile(const char *fileName, ProjectAST **ast);
	static int parseFile(const QString& fileName, ProjectAST **ast);
	static int parseFile(KUrl fileName, ProjectAST **ast);
	
};

}

#endif

// kate: indent-mode csands; tab-width 4; space-indent off; auto-insert-doxygen on;
