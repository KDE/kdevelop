/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakecodecompletionmodel.h"
#include <QVariant>
#include <QModelIndex>
#include <ktexteditor/document.h>

#include "astfactory.h"

using namespace KTextEditor;

CMakeCodeCompletionModel::CMakeCodeCompletionModel(QObject *parent)
    : CodeCompletionModel(parent), m_commands(AstFactory::self()->commands())
{
    setRowCount(m_commands.count());
}

QVariant CMakeCodeCompletionModel::data (const QModelIndex & index, int role) const
{
    if(index.isValid() && role==Qt::DisplayRole && index.column()==CodeCompletionModel::Name)
        return m_commands[index.row()];
    else
        return QVariant();
}

void CMakeCodeCompletionModel::executeCompletionItem(Document* document, const Range& word, int row) const
{
    document->replaceText(word, data(index(row, Name, QModelIndex())).toString()+'(');
}

