/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevelopsessionsplugin.h"

// plugin
#include "sessionlistmodel.h"
// Qt
#include <QQmlEngine>

void KDevelopSessionsPlugin::registerTypes(const char* uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.kdevelopsessions"));

    qmlRegisterType<SessionListModel>(uri, 1, 0, "SessionListModel");
}
