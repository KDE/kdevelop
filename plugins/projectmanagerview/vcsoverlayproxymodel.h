/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_VCSOVERLAYPROXYMODEL_H
#define KDEVPLATFORM_PLUGIN_VCSOVERLAYPROXYMODEL_H

#include <project/projectmodel.h>
#include <QIdentityProxyModel>

class QUrl;
namespace KDevelop {
    class IProject;
    class VcsJob;
}

class VcsOverlayProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
    public:
        enum Roles {
            VcsStatusRole = KDevelop::ProjectModel::LastRole
        };
        explicit VcsOverlayProxyModel(QObject* parent = nullptr);
        ~VcsOverlayProxyModel();

        QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const override;

    private Q_SLOTS:
        void addProject(KDevelop::IProject* p);
        void removeProject(KDevelop::IProject* p);
        void repositoryBranchChanged(const QUrl& url);
        void branchNameReady(KDevelop::VcsJob* job);

    private:
        QModelIndex indexFromProject(QObject* project);
        QHash<KDevelop::IProject*, QString> m_branchName;
};

#endif // KDEVPLATFORM_PLUGIN_VCSOVERLAYPROXYMODEL_H
