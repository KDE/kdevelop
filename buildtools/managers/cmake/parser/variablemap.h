/* KDevelop CMake Support
 *
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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
 
#ifndef CMAKE_VARIABLEMAP_H
#define CMAKE_VARIABLEMAP_H

#include <QHash>
#include <QStringList>
#include "cmakeexport.h"

class KDEVCMAKECOMMON_EXPORT VariableMap : public QHash<QString, QStringList>
{
    public:
//         bool contains(const QString& varName) const;
//         iterator insert(const QString& varName, const QStringList& value);
//         iterator insertMulti(const QString& varName, const QStringList& value);
        QStringList value(const QString& varName) const;
//         QStringList take(const QString& varName);
//         int remove(const QString& varName);
// 
//         int size() const { return QHash<QString, QStringList>::size(); }
//         QStringList keys() const { return QHash<QString, QStringList>::keys(); }
        static QString regexVar() { return "\\$\\{[A-z0-9\\-._]+\\}"; }
        static QString regexEnvVar() { return "\\$ENV\\{[A-z0-9\\-._]+\\}"; }
};

#endif
