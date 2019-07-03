/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVCLAZY_CHECKS_DB_H
#define KDEVCLAZY_CHECKS_DB_H

#include <QMap>
#include <QUrl>

namespace Clazy
{

struct Level;

struct Check
{
    const Level* level = nullptr;
    QString name;
    QString description;
};

struct Level
{
    QString name;
    QString displayName;
    QString description;

    QMap<QString, Check*> checks;
};

class ChecksDB
{
public:
    explicit ChecksDB(const QUrl& docsPath);
    ~ChecksDB();

    bool isValid() const;
    QString error() const;

    const QMap<QString, Level*>& levels() const;

    const QMap<QString, Check*>& checks() const;

private:
    Q_DISABLE_COPY(ChecksDB)

    QString m_error;

    QMap<QString, Check*> m_checks;
    QMap<QString, Level*> m_levels;
};

}

#endif
