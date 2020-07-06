/*  This file is part of KDevelop
    Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KDEVELOPSESSIONDATA_H
#define KDEVELOPSESSIONDATA_H

// Qt
#include <QMetaType>
#include <QString>

struct KDevelopSessionData
{
    QString id;
    QString name;
    QString description;

    bool operator==(const KDevelopSessionData& other) const
    {
        return id == other.id && name == other.name && description == other.description;
    }
};

Q_DECLARE_METATYPE(KDevelopSessionData)
Q_DECLARE_TYPEINFO(KDevelopSessionData, Q_MOVABLE_TYPE);

#endif
