/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ENVIRONMENTPROFILELISTMODEL_H
#define KDEVPLATFORM_ENVIRONMENTPROFILELISTMODEL_H

#include <QAbstractItemModel>
#include "util/environmentprofilelist.h"

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
