/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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
        virtual ~StashManagerDialog();
        
    public slots:
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
        QWidget* m_mainWidget;
        GitPlugin* m_plugin;
        QDir m_dir;
};

class StashModel : public QStandardItemModel
{
    Q_OBJECT
    public:
        explicit StashModel(const QDir& dir, GitPlugin* git, QObject* parent = 0);
        
    private slots:
        void stashListReady(KJob*);
};

#endif // KDEVPLATFORM_PLUGIN_STASHMANAGERDIALOG_H
