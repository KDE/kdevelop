/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_STASHMANAGERDIALOG_H
#define KDEVPLATFORM_PLUGIN_STASHMANAGERDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QDir>

class KJob;
namespace Ui { class StashManager; }

class GitPlugin;

class StashManagerDialog : public QDialog
{
    Q_OBJECT
    public:
        explicit StashManagerDialog(const QDir& stashed, GitPlugin* plugin, QWidget* parent);
        ~StashManagerDialog() override;
        
    public Q_SLOTS:
        void showStash();
        void applyClicked();
        void branchClicked();
        void popClicked();
        void dropClicked();
        void stashesFound();
        
    private:
        QString selection() const;
        void runStash(const QStringList& arguments);
        
        Ui::StashManager* m_ui;
        GitPlugin* m_plugin;
        QDir m_dir;
};

class StashModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        enum ItemRoles {
            RefRole = Qt::UserRole+1, /**< the stash item ref*/
            MessageRole,              /**< the message associated with the stash item (possibly empty)**/
            CommitHashRole,           /**< a short hash of the commit on which the stash was made */
            CommitDescRole,           /**< a brief description of the commit on which the stash was made (possibly empty) */
            BranchRole,               /**< the name of the branch on which the stash was made (possibly empty)*/
            DateRole,                 /**< the date when the stash was created */
        };
        explicit StashModel(const QDir& dir, GitPlugin* git, QObject* parent = nullptr);
        
    private Q_SLOTS:
        void stashListReady(KJob*);
};

#endif // KDEVPLATFORM_PLUGIN_STASHMANAGERDIALOG_H
