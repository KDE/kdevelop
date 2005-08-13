/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "automakeprojectmodel.h"

#include <urlutil.h>

#include <qregexp.h>

QStringList AutomakeFolderModel::subdirs() const
{
    QString s = attribute("SUBDIRS").toString();
    return QStringList::split(QRegExp("[ \t]+"), s);
}

void AutomakeFolderModel::addSubdir(const QString &path)
{
    QString s = URLUtil::relativePathToFile(name(), path);
    Q_ASSERT(!s.isEmpty());
    QString subdirs = attribute("SUBDIRS").toString() + " " + s;
    setAttribute("SUBDIRS", subdirs.stripWhiteSpace());
}
