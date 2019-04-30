/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesonkwargsmodify.h"

#include <QStringList>

MesonKWARGSModify::MesonKWARGSModify(MesonKWARGSModify::Function fn, MesonKWARGSModify::Operation op, QString id)
    : m_func(fn)
    , m_op(op)
    , m_id(id)
{
}

QJsonObject MesonKWARGSModify::command()
{
    QJsonObject res;

    auto func2str = [](Function fn) -> QString {
        switch (fn) {
        case PROJECT:
            return QStringLiteral("project");
        case TARGET:
            return QStringLiteral("target");
        case DEPENDENCY:
            return QStringLiteral("dependency");
        }

        return QStringLiteral("ERROR");
    };

    auto op2str = [](Operation op) -> QString {
        switch (op) {
        case SET:
            return QStringLiteral("set");
        case DELETE:
            return QStringLiteral("delete");
        }

        return QStringLiteral("ERROR");
    };

    res[QStringLiteral("type")] = QStringLiteral("kwargs");
    res[QStringLiteral("function")] = func2str(m_func);
    res[QStringLiteral("id")] = m_id;
    res[QStringLiteral("operation")] = op2str(m_op);
    res[QStringLiteral("kwargs")] = m_kwargs;

    return res;
}

void MesonKWARGSModify::set(QString kwarg, QJsonValue value)
{
    m_kwargs[kwarg] = value;
}

void MesonKWARGSModify::unset(QString kwarg)
{
    if (isSet(kwarg)) {
        m_kwargs.remove(kwarg);
    }
}

void MesonKWARGSModify::clear()
{
    auto kwargs = m_kwargs.keys();
    for (auto i : kwargs) {
        m_kwargs.remove(i);
    }
}

bool MesonKWARGSModify::isSet(QString kwarg)
{
    return m_kwargs.contains(kwarg);
}

MesonKWARGSModify::Function MesonKWARGSModify::function() const
{
    return m_func;
}

MesonKWARGSModify::Operation MesonKWARGSModify::operation() const
{
    return m_op;
}

QString MesonKWARGSModify::id() const
{
    return m_id;
}

// Constructors

MesonKWARGSProjectModify::MesonKWARGSProjectModify(MesonKWARGSModify::Operation op)
    : MesonKWARGSModify(PROJECT, op, QStringLiteral("/"))
{
}
MesonKWARGSTargetModify::MesonKWARGSTargetModify(MesonKWARGSModify::Operation op, QString id)
    : MesonKWARGSModify(TARGET, op, id)
{
}
MesonKWARGSDependencyModify::MesonKWARGSDependencyModify(MesonKWARGSModify::Operation op, QString id)
    : MesonKWARGSModify(DEPENDENCY, op, id)
{
}

// Destructors

MesonKWARGSModify::~MesonKWARGSModify() {}
MesonKWARGSProjectModify::~MesonKWARGSProjectModify() {}
MesonKWARGSTargetModify::~MesonKWARGSTargetModify() {}
MesonKWARGSDependencyModify::~MesonKWARGSDependencyModify() {}
