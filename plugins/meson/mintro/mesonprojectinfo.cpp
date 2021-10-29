/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
