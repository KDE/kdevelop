/*
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_BRANCH_MANAGER_H
#define KDEVPLATFORM_BRANCH_MANAGER_H

#include <QDialog>

class KJob;

namespace Ui { class BranchDialogBase; }

class QSortFilterProxyModel;

namespace KDevelop
{
class BranchesListModel;
class DistributedVersionControlPlugin;
}

class BranchManager : public QDialog
{
    Q_OBJECT
public:
    BranchManager(const QString& repository, KDevelop::DistributedVersionControlPlugin* executor, QWidget *parent = nullptr);
    ~BranchManager() override;

Q_SIGNALS:
    void checkedOut(KJob*);

private Q_SLOTS:
    void createBranch();
    void deleteBranch();
    void renameBranch();
    void checkoutBranch();
    void mergeBranch();
    void diffFromBranch();
    void diffJobFinished(KJob* job);

private:
    QString m_repository;
    KDevelop::DistributedVersionControlPlugin* m_dvcPlugin;

    Ui::BranchDialogBase* m_ui;
    KDevelop::BranchesListModel* m_model;
    QSortFilterProxyModel* m_filterModel;
};

#endif
