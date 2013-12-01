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

#include "generationexpressionsolver.h"
#include <QString>
#include <QDebug>
#include <QStringList>

QHash<QString, QString> GenerationExpressionSolver::s_vars;
QSet<QString> GenerationExpressionSolver::s_neededValues;

GenerationExpressionSolver::GenerationExpressionSolver(const CMakeProperties& properties, const QHash<QString, QString>& alias)
    : m_alias(alias)
    , m_props(properties)
{
    if(s_vars.isEmpty()) {
        s_vars["ANGLE-R"] = QChar('>');
        s_vars["COMMA"] = QChar(',');
        s_vars["SEMICOLON"] = QChar(';');

        s_neededValues.insert("BUILD_INTERFACE");
        s_neededValues.insert("INSTALL_INTERFACE");
    }
}

void GenerationExpressionSolver::defineVariable(const QString& key, const QString& value)
{
    m_values.insert(key, value);
}

QString GenerationExpressionSolver::run(const QString& op)
{
    if(!op.startsWith("$<"))
        return op;

    return process(op);
}

QString GenerationExpressionSolver::process(const QString& op)
{
    QString ret;
    int i = op.indexOf("$<"), last = 0;
    for(; i>=0 && i<op.size(); ) {
        ret += op.mid(last, i-last);
        int depth = 0, split=-1;
        i+=2;
        for(int j=i; i<op.size(); ++i) {
            if(op[i] == '>') {
                if(depth==0) {
                    if(split<0)
                        split = i;

                    ret += calculate(op.mid(j, split-j), op.mid(split+1, i-split-1));
                    last = i+1;
                    break;
                } else
                    --depth;
            } else if(op.mid(i, 2)=="$<") {
                ++depth;
            } else if(op[i]==':') {
                split = i;
            }
        }
        Q_ASSERT(depth==0);
        i = op.indexOf("$<", i+1);
    }
    ret += op.mid(last);
    return ret;
}

QString GenerationExpressionSolver::calculate(const QString& pre, const QString& post)
{
    if(pre == "0")
        return QString();
    else if(pre == "1") {
        return process(post);
    } else if(pre == "BOOL") {
        static QSet<QString> s_trueDefinitions=QSet<QString>() << "1" << "on" << "yes" << "true" << "y";
        bool ret = s_trueDefinitions.contains(post.toLower());
        return ret ? "1" : "0";
    } else if(pre == "AND") {
        QStringList vals = post.split(',');
        bool ret = true;
        foreach(const QString& op, vals) {
            ret &= (process(op) == "1");
        }
        return ret ? "1" : "0";
    } else if(pre == "OR") {
        QStringList vals = post.split(',');
        bool ret = false;
        foreach(const QString& op, vals) {
            ret |= (process(op) == "1");
        }
        return ret ? "1" : "0";
    } else if(pre == "STREQUAL") {
        QStringList vals = post.split(',');
        bool ret = false;
        if(!vals.isEmpty()) {
            QString first = vals.first();
            for(QStringList::const_iterator it=vals.constBegin()+1, itEnd=vals.constEnd(); it!=itEnd; ++it) {
                ret |= (process(*it) == first);
            }
        }
        return ret ? "1" : "0";
    } else if(pre == "NOT") {
        bool val = process(post) == "1";
        return !val ? "1" : "0";
    } else if(pre == "TARGET_PROPERTY") {
        int idx = post.indexOf(',');
        QString targetName, propName;
        if(idx>0) {
            targetName = post.left(idx);
            propName = post.mid(idx+1);
            targetName = m_alias.value(targetName, targetName);
        } else {
            targetName = m_name;
            propName = post;
        }
        return m_props[TargetProperty][targetName][propName].join(":");
    } else if(s_neededValues.contains(pre)) {
        return process(post);
    } else if(m_values.contains(pre)) {
        return m_values.value(pre);
    } else {
        QString ret = s_vars.value(pre);
        if(ret.isEmpty()) {
            kDebug() << "unknown expression" << pre << post;
        }
        return ret;
    }
}

void GenerationExpressionSolver::setTargetName(const QString& name)
{
    m_name = m_alias.value(name, name);
}
