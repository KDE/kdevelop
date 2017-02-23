/* This file is part of KDevelop
Copyright 2007 Andreas Pakulat <apaku@gmx.de>
Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KDEVPLATFORM_ENVIRONMENTPROFILELISTMODEL_H
#define KDEVPLATFORM_ENVIRONMENTPROFILELISTMODEL_H

#include <QAbstractItemModel>
#include "util/environmentprofilelist.h"

class QStringList;

namespace KDevelop
{

// Subclassing EnvironmentProfileList instead of having as a member, to have access to protected API
class EnvironmentProfileListModel : public QAbstractItemModel, protected EnvironmentProfileList
{
    Q_OBJECT

public:
    explicit EnvironmentProfileListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int profileIndex(const QString& profileName) const;
    int defaultProfileIndex() const;
    QString profileName(int profileIndex) const;
    bool hasProfile(const QString& profileName) const;
    QMap<QString, QString>& variables(const QString& profileName);

    int addProfile(const QString& newProfileName);
    int cloneProfile(const QString& newProfileName, const QString& sourceProfileName);
    void removeProfile(int profileIndex);
    void setDefaultProfile(int profileIndex);

    void loadFromConfig(KConfig* config);
    void saveToConfig(KConfig* config);

Q_SIGNALS:
    void profileAboutToBeRemoved(const QString& profileName);
    void defaultProfileChanged(int defaultProfileIndex);
};

}

#endif
