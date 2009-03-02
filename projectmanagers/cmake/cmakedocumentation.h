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

#ifndef CMAKEDOCUMENTATION_H
#define CMAKEDOCUMENTATION_H

//NOTE: This should probably be merged at some point with the KDevplatform documentation support.
// I'm just not doing it right now because I want to track cmake needs first.

#include <QString>
#include <QMap>
#include <QStringList>

namespace KDevelop { class Declaration; }

class CMakeDocumentation
{
    public:
        CMakeDocumentation(const QString& cmakeCmd);
        QString description(const QString& identifier);
        QString description(const KDevelop::Declaration* decl);
    private:
        enum Type { Command, Variable, Module, Property, Policy };
        void collectIds(const QString& param, Type type);
        
        QMap<QString, Type> m_typeForName;
        QString mCMakeCmd;
};

#endif // CMAKEDOCUMENTATION_H
