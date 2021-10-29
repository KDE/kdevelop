/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
