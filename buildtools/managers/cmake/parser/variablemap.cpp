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

#include "variablemap.h"

#if 0
bool VariableMap::contains(const QString & varName) const
{
    return QHash<QString, QStringList>::contains(varName.toLower());
}

QHash<QString, QStringList>::iterator VariableMap::insert(const QString & varName, const QStringList & value)
{
    return QHash<QString, QStringList>::insert(varName.toLower(), value);
}

QHash<QString, QStringList>::iterator VariableMap::insertMulti(const QString & varName, const QStringList & value)
{
    return QHash<QString, QStringList>::insertMulti(varName.toLower(), value);
}

QStringList VariableMap::value(const QString & varName) const
{
    return QHash<QString, QStringList>::value(varName.toLower());
}

QStringList VariableMap::take(const QString & varName)
{
    return QHash<QString, QStringList>::take(varName.toLower());
}

int VariableMap::remove(const QString & varName)
{
    return QHash<QString, QStringList>::remove(varName.toLower());
}
#endif




