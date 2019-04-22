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

#include "mesonprojectinfo.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <debug.h>

MesonProjectInfo::MesonProjectInfo(const QJsonObject& json)
{
    fromJSON(json);
}

MesonProjectInfo::~MesonProjectInfo() {}

void MesonProjectInfo::fromJSON(const QJsonObject& json)
{
    m_name = json[QStringLiteral("descriptive_name")].toString();
    m_version = json[QStringLiteral("version")].toString();

    qCDebug(KDEV_Meson) << "MINTRO: Meson project" << m_name << "version" << m_version << "info loaded";
}

QString MesonProjectInfo::name() const
{
    return m_name;
}

QString MesonProjectInfo::version() const
{
    return m_version;
}
