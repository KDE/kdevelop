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

void MesonRewriterDefaultOpts::set(QString const& name, QString const& value)
{
    m_data[name] = value;
}
