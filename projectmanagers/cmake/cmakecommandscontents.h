/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
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

#ifndef CMAKECOMMANDSCONTENTS_H
#define CMAKECOMMANDSCONTENTS_H

#include <QVector>
#include <QMap>
#include <QAbstractItemModel>

#include "cmakedocumentation.h"

class CMakeCommandsContents : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CMakeCommandsContents(QObject* parent);

    void processOutput(int code);

    int rowCount(const QModelIndex & parent) const override;

    QVariant data(const QModelIndex & index, int role) const override;

    QString descriptionForIdentifier(const QString& id, CMakeDocumentation::Type t) const;
    QVector<QString> names(CMakeDocumentation::Type t) const;
    CMakeDocumentation::Type typeFor(const QString &id) const;
    void showItemAt(const QModelIndex &idx) const;

    QModelIndex parent(const QModelIndex & child) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;

private:
    QMap<QString, CMakeDocumentation::Type> m_typeForName;
    QVector<QVector<QString>> m_namesForType;
};
#endif // CMAKECOMMANDSCONTENTS_H
