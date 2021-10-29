/*
    SPDX-FileCopyrightText: 2019 José Millán Soto <jmillan@kde-espana.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_REBASEDIALOG_H
#define KDEVPLATFORM_PLUGIN_REBASEDIALOG_H

#include <QDialog>
#include <QUrl>

class GitPlugin;

namespace KDevelop { class BranchesListModel; }

namespace Ui { class RebaseDialog; }

class RebaseDialog : public QDialog
{
    Q_OBJECT
    public:
        RebaseDialog(GitPlugin* plugin, const QUrl &repository, QWidget* parent=nullptr);
        ~RebaseDialog() override;

    protected Q_SLOTS:
        void performRebase();

    private:
        GitPlugin *m_plugin;
        QUrl m_repository;
        KDevelop::BranchesListModel *m_model;
        Ui::RebaseDialog *m_ui;
};

#endif // KDEVPLATFORM_PLUGIN_REBASEDIALOG_H
