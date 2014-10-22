/* KDevelop CMake Support
 *
 * Copyright 2007-2014 Aleix Pol <aleixpol@kde.org>
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

#include "cmakemodelitems.h"

#include <QJsonObject>
#include <QJsonArray>

CMakeLibraryItem::CMakeLibraryItem(const QJsonObject& object, KDevelop::ProjectBaseItem* parent)
    : ProjectLibraryTargetItem(parent->project(), object.value(QString("name")).toString(), parent)
    , CMakeJsonItem(object)
{
    adoptObject();
}

CMakeExecutableItem::CMakeExecutableItem(const QJsonObject& object, KDevelop::ProjectBaseItem* parent)
    : ProjectExecutableTargetItem(parent->project(), object.value(QString("name")).toString(), parent)
    , CMakeJsonItem(object)
{
    adoptObject();
}

QUrl CMakeExecutableItem::builtUrl() const
{
    return QUrl::fromLocalFile(m_object["location"].toString());
}

QUrl CMakeExecutableItem::installedUrl() const
{
    return QUrl();
}

#include <QDebug>

void adoptObjectInTarget(const QJsonObject& m_object, KDevelop::ProjectTargetItem* item)
{
    QJsonArray jsonSources = m_object["configs"].toArray().first().toObject()["sources"].toArray();
    QStringList sources;
    for(const QJsonValue& s: jsonSources) {
        sources += s.toString();
    }

    QList<KDevelop::ProjectFileItem*> files = item->fileList();
    for(KDevelop::ProjectFileItem* file : files) {
        int count = sources.removeAll(file->fileName());
        if(count == 0) {
            delete file;
        }
    }

    for(const QString& s: sources) {
        new KDevelop::ProjectFileItem(s, item);
    }
}

void CMakeExecutableItem::adoptObject()
{
    adoptObjectInTarget(m_object, this);
}

void CMakeLibraryItem::adoptObject()
{
    adoptObjectInTarget(m_object, this);
}

