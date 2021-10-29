/*
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

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

        explicit BranchesListModel(QObject* parent = nullptr);
        ~BranchesListModel() override;

        void initialize(KDevelop::IBranchingVersionControl* dvcsplugin, const QUrl& repo);

        QHash<int, QByteArray> roleNames() const override;

        Q_INVOKABLE void createBranch(const QString& baseBranch, const QString& newBranch);
        Q_INVOKABLE void removeBranch(const QString& branch);

        QUrl repository() const;
        KDevelop::IBranchingVersionControl* interface() const;
        void refresh();
        QString currentBranch() const;
        void setCurrentBranch(const QString& branch);

        KDevelop::IProject* project() const;
        void setProject(KDevelop::IProject* p);

    public Q_SLOTS:
        void resetCurrent();

    Q_SIGNALS:
        void currentBranchChanged();

    private:
        const QScopedPointer<class BranchesListModelPrivate> d_ptr;
        Q_DECLARE_PRIVATE(BranchesListModel)
};

}

#endif // KDEVPLATFORM_BRANCHESLISTMODEL_H
