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


#ifndef EXECUTESCRIPTOUTPUTMODEL_H
#define EXECUTESCRIPTOUTPUTMODEL_H

#include <outputview/outputmodel.h>
#include <outputview/ioutputviewmodel.h>
#include <QObject>
#include <QAbstractItemView>

namespace KDevelop {

class ExecuteScriptOutputModel : public OutputModel 
{
    Q_OBJECT
public:
    ExecuteScriptOutputModel(QObject* parent);
    virtual void activate(const QModelIndex& index);
    virtual QModelIndex nextHighlightIndex(const QModelIndex& currentIndex);
    virtual QModelIndex previousHighlightIndex(const QModelIndex& currentIndex);
private:
    QModelIndex scanIndices(const QModelIndex& currentIndex, int direction, const QModelIndex& ifInvalid) const;
    int lineMatches(const QString& line) const;
    QList<QRegExp> m_patterns;
};

}

#endif // EXECUTESCRIPTOUTPUTMODEL_H
