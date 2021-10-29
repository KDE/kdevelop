/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonkwargsmodify.h"

#include <QStringList>

MesonKWARGSModify::MesonKWARGSModify(MesonKWARGSModify::Function fn, MesonKWARGSModify::Operation op, const QString& id)
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

void MesonKWARGSModify::set(const QString& kwarg, const QJsonValue& value)
{
    m_kwargs[kwarg] = value;
}

void MesonKWARGSModify::unset(const QString& kwarg)
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

bool MesonKWARGSModify::isSet(const QString& kwarg)
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
MesonKWARGSTargetModify::MesonKWARGSTargetModify(MesonKWARGSModify::Operation op, const QString& id)
    : MesonKWARGSModify(TARGET, op, id)
{
}
MesonKWARGSDependencyModify::MesonKWARGSDependencyModify(MesonKWARGSModify::Operation op, const QString& id)
    : MesonKWARGSModify(DEPENDENCY, op, id)
{
}

// Destructors

MesonKWARGSModify::~MesonKWARGSModify() {}
MesonKWARGSProjectModify::~MesonKWARGSProjectModify() {}
MesonKWARGSTargetModify::~MesonKWARGSTargetModify() {}
MesonKWARGSDependencyModify::~MesonKWARGSDependencyModify() {}
