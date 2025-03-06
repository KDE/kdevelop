/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTCHANGESMODEL_H
#define KDEVPLATFORM_PROJECTCHANGESMODEL_H

#include <vcs/models/vcsfilechangesmodel.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

#include "projectexport.h"

class KJob;
namespace KDevelop {
class IProject;
class IDocument;

class KDEVPLATFORMPROJECT_EXPORT ProjectChangesModel : public VcsFileChangesModel
{
    Q_OBJECT
    public:
        enum Role { ProjectNameRole = LastItemRole+1 };
        explicit ProjectChangesModel(QObject* parent);
        ~ProjectChangesModel() override;
        
        void updateState(KDevelop::IProject* p, const KDevelop::VcsStatusInfo& status);

        void changes(KDevelop::IProject* project, const QList<QUrl>& urls, KDevelop::IBasicVersionControl::RecursionMode mode);

        /**
         * @return the top-level item for a given project or @c nullptr if no such item
         */
        [[nodiscard]] QStandardItem* projectItem(const KDevelop::IProject* project) const;

    public Q_SLOTS:
        void reloadAll();
        void reload(const QList<KDevelop::IProject*>& p);
        void reload(const QList<QUrl>& p);
        
        void addProject(KDevelop::IProject* p);
        void removeProject(KDevelop::IProject* p);
        
        void statusReady(KJob* job);
        void documentSaved(KDevelop::IDocument*);
        void itemsAdded(const QModelIndex& idx, int start, int end);
        void jobUnregistered(KJob*);
        void repositoryBranchChanged(const QUrl& url);
        void branchNameReady(KDevelop::VcsJob* job);
};

}

#endif // KDEVPLATFORM_PROJECTCHANGESMODEL_H
