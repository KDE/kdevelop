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
#include "managecustomcommand.h"

#include <qtable.h>

#include <klocale.h>

ManageCustomCommand::ManageCustomCommand(QWidget *parent, const char *name)
    :ManageCustomBuildCommandsBase(parent, name)
{
}

void ManageCustomCommand::addButton_clicked()
{
    commandsTable->setNumRows(commandsTable->numRows() + 1);
    setRowProperties(commandsTable->numRows()-1);
}

void ManageCustomCommand::removeButton_clicked()
{
    commandsTable->removeRow(commandsTable->currentRow());
}

void ManageCustomCommand::setRowProperties(int row)
{
    commandsTable->setItem(row, 2,
        new QComboTableItem(commandsTable, QStringList::split(",", 
        i18n("this is a list of items in the combobox",
        "Make target,Make target (as root),Make command,Make command (as root),Command,Command (as root)"))));
}

#include "managecustomcommand.moc"
