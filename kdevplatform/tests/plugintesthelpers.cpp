/*
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plugintesthelpers.h"

#include <KPluginMetaData>

#include <QJsonObject>

namespace KDevelop {

KPluginMetaData makeTestPluginMetaData(const QString& name)
{
    const QJsonObject pluginJsonObject{{QStringLiteral("KPlugin"),
                                        QJsonObject{
                                            {QStringLiteral("Id"), name.toLower()},
                                            {QStringLiteral("Name"), name},
                                        }}};

    return KPluginMetaData(pluginJsonObject, QString());
}

}
