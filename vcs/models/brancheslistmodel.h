/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef KDEVPLATFORM_BRANCHESLISTMODEL_H
#define KDEVPLATFORM_BRANCHESLISTMODEL_H

#include <QStandardItemModel>
#include <QUrl>

#include <vcs/vcsexport.h>

namespace KDevelop {
class IBranchingVersionControl;
class IProject;
class BranchesListModelPrivate;

class KDEVPLATFORMVCS_EXPORT BranchesListModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(KDevelop::IProject* project READ project WRITE setProject)
    Q_PROPERTY(QString currentBranch READ currentBranch WRITE setCurrentBranch NOTIFY currentBranchChanged)
    public:
        enum Roles { CurrentRole = Qt::UserRole+1 };

        BranchesListModel(QObject* parent = 0);
        ~BranchesListModel();

        void initialize(KDevelop::IBranchingVersionControl* dvcsplugin, const QUrl& repo);

        virtual QHash<int, QByteArray> roleNames() const override;

        void createBranch(const QString& baseBranch, const QString& newBranch);
        void removeBranch(const QString& branch);

        QUrl repository() const;
        KDevelop::IBranchingVersionControl* interface();
        void refresh();
        QString currentBranch() const;
        void setCurrentBranch(const QString& branch);

        KDevelop::IProject* project() const;
        void setProject(KDevelop::IProject* p);

    public slots:
        void resetCurrent();

    signals:
        void currentBranchChanged();

    private:
        const QScopedPointer<BranchesListModelPrivate> d;
};

}

#endif // KDEVPLATFORM_BRANCHESLISTMODEL_H
