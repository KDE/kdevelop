/*
 * This file is part of KDevelop
 * Copyright 2019 José Millán Soto <jmillan@kde-espana.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
