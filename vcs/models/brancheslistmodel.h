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

#ifndef BRANCHESLISTMODEL_H
#define BRANCHESLISTMODEL_H

#include <QStandardItemModel>
#include <KUrl>

namespace KDevelop {
class IBranchingVersionControl;}

class BranchesListModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        BranchesListModel(QObject* parent = 0);
        void initialize(KDevelop::IBranchingVersionControl* dvcsplugin, const KUrl& repo);
        
        void createBranch(const QString& baseBranch, const QString& newBranch);
        void removeBranch(const QString& branch);
        
        KUrl repository() const { return repo; }
        KDevelop::IBranchingVersionControl* interface();
        void refresh();
        QString currentBranch() const;
        
    public slots:
        void resetCurrent();
        
    private:
        QString curBranch();
        
        KDevelop::IBranchingVersionControl* dvcsplugin;
        KUrl repo;
        QString m_currentBranch;
};

#endif // BRANCHESLISTMODEL_H
