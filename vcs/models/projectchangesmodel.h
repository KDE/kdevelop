/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#ifndef KDEVPLATFORM_PROJECTCHANGESMODEL_H
#define KDEVPLATFORM_PROJECTCHANGESMODEL_H

#include <vcs/models/vcsfilechangesmodel.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

#include "vcsexport.h"

class KJob;
namespace KDevelop {
class IProject;
class IDocument;

class KDEVPLATFORMVCS_EXPORT ProjectChangesModel : public VcsFileChangesModel
{
    Q_OBJECT
    public:
        enum Role { ProjectNameRole = Qt::UserRole };
        ProjectChangesModel(QObject* parent);
        virtual ~ProjectChangesModel();
        
        void updateState(KDevelop::IProject* p, const KDevelop::VcsStatusInfo& status);
        
        QStandardItem* projectItem(KDevelop::IProject* p) const;
        static QStandardItem* fileItemForProject(QStandardItem* projectItem, const QUrl& url);

        void changes(KDevelop::IProject* project, const QList<QUrl>& urls, KDevelop::IBasicVersionControl::RecursionMode mode);
        
    public slots:
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
