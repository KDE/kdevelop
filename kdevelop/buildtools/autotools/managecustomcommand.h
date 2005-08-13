/***************************************************************************
*   Copyright (C) 2004 by Alexander Dymo <adymo@kdevelop.org>             *
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
*   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
***************************************************************************/
#ifndef MANAGECUSTOMCOMMAND_H
#define MANAGECUSTOMCOMMAND_H

#include "managecustomcommandsbase.h"

class ManageCustomCommand: public ManageCustomBuildCommandsBase
{
	Q_OBJECT
public:
	ManageCustomCommand( QWidget *parent = 0, const char *name = 0 );

	void setRowProperties( int row );
public slots:
	virtual void addButton_clicked();
	virtual void removeButton_clicked();
};

#endif 
// kate: indent-mode csands; tab-width 4;
