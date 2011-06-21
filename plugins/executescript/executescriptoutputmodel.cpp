/*  This file is part of KDevelop
    Copyright 2011 by Sven Brauch <svenbrauch@googlemail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "executescriptoutputmodel.h"

#include <KDebug>
#include <QRegExp>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <QFileInfo>

namespace KDevelop {

ExecuteScriptOutputModel::ExecuteScriptOutputModel(): OutputModel()
{
    kDebug() << "Creating script output model";

    QRegExp python("^  File \"(.*)\", line (\\d*), in(.*)$");
    m_patterns << python;

    QRegExp phpstacktrace("^.*(/.*):(\\d*).*$");
    m_patterns << phpstacktrace;

    QRegExp phperror("^.* in (/.*) on line (\\d*).*$");
    m_patterns << phperror;

    for ( int i = m_patterns.length() - 1; i >= 0; i-- ) {
        m_patterns[i].setMinimal(true);
    }
}

int ExecuteScriptOutputModel::lineMatches(const QString& line) const
{
    for ( int i = m_patterns.length() - 1; i >= 0; i-- ) {
        if ( m_patterns[i].exactMatch(line) ) {
            QFileInfo file(m_patterns.at(i).capturedTexts()[1]);
            if ( ! file.exists() ) {
                continue;
            }
            return i;
        }
    }
    return -1;
}

void ExecuteScriptOutputModel::activate(const QModelIndex& index)
{
    QString line = data(index).toString();
    int matches = lineMatches(line);
    if ( matches != -1 ) {
        const QRegExp& pattern = m_patterns.at(matches);
        QString path = pattern.capturedTexts()[1];
        int linenumber = pattern.capturedTexts()[2].toInt() - 1;
        linenumber = qMax(0, linenumber);
        KTextEditor::Cursor cur(linenumber, 0);
        ICore::self()->documentController()->openDocument(path, cur);
    }
}

QModelIndex ExecuteScriptOutputModel::scanIndices(const QModelIndex& currentIndex, int direction, const QModelIndex& ifInvalid) const
{
    int matched = -1;
    QModelIndex next;
    if ( ! currentIndex.isValid() ) {
        next = ifInvalid;
    } else {
        next = currentIndex;
    }

    do {
        next = next.sibling(next.row() + direction, next.column());
        if ( ! next.isValid() )
            break;
        matched = lineMatches(next.data().toString());
    } while ( matched < 0 );
    return next;
}

QModelIndex ExecuteScriptOutputModel::nextHighlightIndex(const QModelIndex& currentIndex)
{
    return scanIndices(currentIndex, +1, index(0, 0));
}

QModelIndex ExecuteScriptOutputModel::previousHighlightIndex(const QModelIndex& currentIndex)
{
    return scanIndices(currentIndex, -1, index(rowCount() - 1, 0));
}

}
