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
#include "../debug.h"

VariableMap::VariableMap()
{
    m_scopes.push(QSet<QString>());
}

QStringList splitVariable(const QStringList& input)
{
    QStringList ret;
    foreach(const QString& v, input)
    {
        if(v.isEmpty())
            continue;

        ret += v.split(';');
    }
    return ret;
}

void VariableMap::insert(const QString& varName, const QStringList& value, bool parentScope)
{
    QSet< QString >* current;
//     qCDebug(CMAKE) << "leeeeeeeeeeeeE" << varName << value << parentScope;
    if(parentScope && m_scopes.size()>1) { //TODO: provide error?
        current = &m_scopes[m_scopes.size()-2];
        m_scopes.top().remove(varName);
    } else
        current = &m_scopes.top();

    QStringList ret = splitVariable(value);

    if(current->contains(varName))
        (*this)[varName]=ret;
    else {
        current->insert(varName);
        QHash<QString, QStringList>::insertMulti(varName, ret);
    }

//     QHash<QString, QStringList>::insert(varName, ret);
//     qCDebug(CMAKE) << "++++++++" << varName << QHash<QString, QStringList>::value(varName);
}

QHash<QString, QStringList>::iterator VariableMap::insertMulti(const QString & varName, const QStringList & value)
{
    return QHash<QString, QStringList>::insertMulti(varName, splitVariable(value));
}

void VariableMap::insertGlobal(const QString& varName, const QStringList& value)
{
    QHash<QString, QStringList>::insert(varName, value);
}

void VariableMap::pushScope()
{
    m_scopes.push(QSet<QString>());
}

void VariableMap::popScope()
{
    QSet<QString> t=m_scopes.pop();
    foreach(const QString& var, t) {
//         qCDebug(CMAKE) << "removing........" << var << QHash<QString, QStringList>::value(var);
        take(var);
    }
}

int VariableMap::removeMulti(const QString& varName)
{
    iterator it = find(varName);
    if(it==end())
        return 0;
    else {
        erase(it);
        return 1;
    }
}
