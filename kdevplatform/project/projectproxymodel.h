/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTPROXYMODEL_H
#define KDEVPLATFORM_PROJECTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "projectexport.h"

namespace KDevelop {
    class ProjectModel;
    class ProjectBaseItem;
}

class KDEVPLATFORMPROJECT_EXPORT ProjectProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    public:
        explicit ProjectProxyModel(QObject *parent);
        bool lessThan (const QModelIndex & left, const QModelIndex & right) const override;

        QModelIndex proxyIndexFromItem(KDevelop::ProjectBaseItem* item) const;
        KDevelop::ProjectBaseItem* itemFromProxyIndex(const QModelIndex&) const;

        void showTargets(bool visible);

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        KDevelop::ProjectModel* projectModel() const;
        bool m_showTargets;

};

#endif
