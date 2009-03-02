/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
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

#include "cmakedocumentation.h"
#include "cmakeutils.h"
#include <KDebug>
#include <language/duchain/declaration.h>

CMakeDocumentation::CMakeDocumentation(const QString& cmakeCmd)
    : mCMakeCmd(cmakeCmd)
{
    collectIds("--help-command-list", Command);
    collectIds("--help-variable-list", Variable);
    collectIds("--help-module-list", Module);
}

void CMakeDocumentation::collectIds(const QString& param, Type type)
{
    QStringList ids=CMake::executeProcess(mCMakeCmd, QStringList(param)).split('\n');
    ids.takeFirst();
    foreach(const QString& name, ids)
    {
        m_typeForName[name.toLower()]=type;
    }
}

QString CMakeDocumentation::description(const QString& identifier)
{
    kDebug() << "seeking documentation for " << identifier;
    QString arg, id=identifier.toLower();
    if(m_typeForName.contains(id)) {
        switch(m_typeForName[id])
        {
            case Command:
                arg="--help-command";
                break;
            case Variable:
                arg="--help-variable";
                break;
            case Module:
                arg="--help-module";
                break;
            case Property:
            case Policy:
                break;
        }
        qDebug() << "type for" << id << m_typeForName[id];
    }
    
    if(arg.isEmpty())
        return QString();
    else
        return "<pre>"+CMake::executeProcess(mCMakeCmd, QStringList(arg) << identifier)+"</pre>";
}

QString CMakeDocumentation::description(const KDevelop::Declaration* decl)
{
    QString ret=description(decl->identifier().toString());
    if(ret.isEmpty())
    {}
    
    return ret;
}
