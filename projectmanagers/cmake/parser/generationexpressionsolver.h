/* KDevelop CMake Support
 *
 * Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#ifndef GENERATIONEXPRESSIONSOLVER_H
#define GENERATIONEXPRESSIONSOLVER_H

#include "cmakeexport.h"
#include "cmaketypes.h"

class KDEVCMAKECOMMON_EXPORT GenerationExpressionSolver
{
    public:
        GenerationExpressionSolver(const CMakeProperties& properties);
        void setTargetName(const QString& name) { m_name = name; }

        void defineVariable(const QString& key, const QString& value);
        QString run(const QString& op);

    private:
        QString process(const QString& op);
        QString calculate(const QString& pre, const QString& post);

        static QHash<QString, QString> s_vars;
        static QSet<QString> s_neededValues;
        QHash<QString, QString> m_values;
        const CMakeProperties m_props;
        QString m_name;
};

#endif // GENERATIONEXPRESSIONSOLVER_H
