/***************************************************************************
    begin                : Tue Aug 17 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
	
                          Adapted for ruby debugging
                          --------------------------
    begin                : Mon Nov 1 2004
    copyright            : (C) 2004 by Richard Dale
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

#ifndef _RDBPARSER_H_
#define _RDBPARSER_H_

#include "variablewidget.h"

namespace RDBDebugger
{

namespace RDBParser
{
    void parseVariables(LazyFetchItem *parent, char *buf);
    void parseExpandedVariable(VarItem *parent, char *buf);
    DataType determineType(char *buf);
    void setItem(	LazyFetchItem *parent, const QString &varName, 
					DataType dataType, const QCString &value );
}

}

#endif
