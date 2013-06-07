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

#ifndef CMAKECONDITION_H
#define CMAKECONDITION_H

#include "cmakeprojectvisitor.h"

class KDEVCMAKECOMMON_EXPORT CMakeCondition
{
    public:
        CMakeCondition(const CMakeProjectVisitor* v);
        bool condition(const QStringList &expression);
        QList<int> variableArguments() const { return m_argUses; }
        QStringList matches() const { return m_matches; }
        
        enum conditionToken { None=0, variable, NOT, AND, OR, COMMAND, EXISTS, IS_NEWER_THAN, IS_DIRECTORY, IS_ABSOLUTE, MATCHES,
            LESS, GREATER, EQUAL, STRLESS, STRGREATER, STREQUAL, DEFINED, LPR, RPR, VERSION_LESS, VERSION_EQUAL, VERSION_GREATER, Last };
    private:
        static conditionToken typeName(const QString& name);
        QStringList::const_iterator prevOperator(QStringList::const_iterator it, QStringList::const_iterator itStop) const;
        bool evaluateCondition(QStringList::const_iterator it, QStringList::const_iterator itEnd);
        bool isTrue(const QStringList::const_iterator& var);
        const VariableMap* m_vars;
        const CacheValues* m_cache;
        const CMakeProjectVisitor *m_visitor;
        
        static QVector<int> m_priorities;
        static QMap<QString, conditionToken> nameToToken;
        static QSet<QString> s_falseDefinitions;
        static QSet<QString> s_trueDefinitions;
        
        QList<QStringList::const_iterator> m_varUses;
        QList<int> m_argUses;
//         int m_parameters[Last];

        QStringList::const_iterator conditionBegin;
        QRegExp m_numberRx;
        QStringList m_matches;
};

#endif
