/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2013 Sven Brauch <svenbrauch@googlemail.com>
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "cache.h"

#include <QString>
#include <QDir>

#include <kglobal.h>
#include <kstandarddirs.h>

QmlJS::Cache::Cache()
{
}

QmlJS::Cache& QmlJS::Cache::instance()
{
    static Cache *c = nullptr;

    if (!c) {
        c = new Cache();
    }

    return *c;
}

QString QmlJS::Cache::modulePath(const QString& uri, const QString& version)
{
    QString cacheKey = uri + version;
    QString path = m_modulePaths.value(cacheKey, QString());

    if (!path.isNull()) {
        return path;
    }

    // Look for <uri>_<version>.qml in the shipped files
    QString fileName = (version.isNull() ? uri : QString("%1_%2.qml").arg(uri, version));
    path = KGlobal::dirs()->findResource("data",
        QString("kdevqmljssupport/qmlplugins/%1").arg(fileName)
    );

    if (!path.isNull()) {
        m_modulePaths.insert(cacheKey, path);
        return path;
    }

    // Look for <uri> (with the dots replaced with slashes) in the standard KDE
    // QML imports dir.
    QStringList dirs = KGlobal::dirs()->findDirs("module",
        QString("imports/%1").arg(QString(uri).replace(QLatin1Char('.'), QDir::separator()))
    );

    if (dirs.count() != 0) {
        path = dirs.first();
    }

    m_modulePaths.insert(cacheKey, path);
    return path;
}
