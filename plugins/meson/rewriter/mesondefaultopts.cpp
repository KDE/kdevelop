/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesondefaultopts.h"
#include <QJsonObject>

MesonRewriterDefaultOpts::MesonRewriterDefaultOpts(MesonRewriterDefaultOpts::Action action)
    : m_action(action)
{
}

MesonRewriterDefaultOpts::~MesonRewriterDefaultOpts() {}

QJsonObject MesonRewriterDefaultOpts::command()
{
    QJsonObject res;

    res[QStringLiteral("type")] = QStringLiteral("default_options");
    res[QStringLiteral("operation")] = m_action == SET ? QStringLiteral("set") : QStringLiteral("delete");
    res[QStringLiteral("options")] = m_data;

    return res;
}

MesonRewriterDefaultOpts::Action MesonRewriterDefaultOpts::action() const
{
    return m_action;
}

void MesonRewriterDefaultOpts::clear()
{
    m_data = QJsonObject();
}

void MesonRewriterDefaultOpts::set(const QString& name, const QString& value)
{
    m_data[name] = value;
}
