/* KDevelop CMake Support
 *
 * Copyright 2017 Aleix Pol <aleixpol@kde.org>
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

#include "cmakeprojectdata.h"
#include "cmakeutils.h"

void CMakeFile::addDefine(const QString& define)
{
    if (define.isEmpty())
        return;
    const int eqIdx = define.indexOf(QLatin1Char('='));
    if (eqIdx < 0) {
        defines[define] = QString();
    } else {
        defines[define.left(eqIdx)] = define.mid(eqIdx + 1);
    }
}

CMakeTarget::Type CMakeTarget::typeToEnum(const QString& value)
{
    static const QHash<QString, CMakeTarget::Type> s_types = {
        {QStringLiteral("EXECUTABLE"), CMakeTarget::Executable},
        {QStringLiteral("STATIC_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("MODULE_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("SHARED_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("OBJECT_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("INTERFACE_LIBRARY"), CMakeTarget::Library}
    };
    return s_types.value(value, CMakeTarget::Custom);
}

CMakeProjectData::CMakeProjectData(const QHash<KDevelop::Path, QVector<CMakeTarget>>& targets,
                                   const CMakeFilesCompilationData& data, const QVector<CMakeTest>& tests)
    : compilationData(data)
    , targets(targets)
    , m_testSuites(tests)
{
}
