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

#ifndef CMAKEMODELITEMS_H
#define CMAKEMODELITEMS_H

#include <project/projectmodel.h>
#include <QJsonObject>

class CMakeJsonItem
{
public:
    CMakeJsonItem(const QJsonObject& object)
        : m_object(object)
    {}
    virtual ~CMakeJsonItem() {}

    void setObject(const QJsonObject &obj) { m_object = obj; }

protected:
    virtual void adoptObject() = 0;

    QJsonObject m_object;
};

class CMakeExecutableItem : public KDevelop::ProjectExecutableTargetItem, public CMakeJsonItem
{
public:
    CMakeExecutableItem(const QJsonObject& object, KDevelop::ProjectBaseItem* parent = 0);

    virtual QUrl builtUrl() const;
    virtual QUrl installedUrl() const;
    virtual void adoptObject();
};

class CMakeLibraryItem : public KDevelop::ProjectLibraryTargetItem, public CMakeJsonItem
{
public:
    CMakeLibraryItem(const QJsonObject& object, KDevelop::ProjectBaseItem* parent = 0);

    virtual void adoptObject();
};

#endif
