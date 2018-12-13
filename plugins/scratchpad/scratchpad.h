/* This file is part of KDevelop
 *
 * Copyright 2018 Amish K. Naidu <amhndu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include <interfaces/iplugin.h>

#include <QFileIconProvider>

class ScratchpadToolViewFactory;

class QStandardItemModel;
class QModelIndex;
class QFileInfo;
class QString;
class QAction;

class Scratchpad
    : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    Scratchpad(QObject* parent, const QVariantList& args);

    QStandardItemModel* model() const;

    QAction* runAction() const;

    static QString dataDirectory();

    void createActionsForMainWindow(Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions) override;

    enum ExtraRoles {
        FullPathRole = Qt::UserRole + 1,
        RunCommandRole,
    };

public Q_SLOTS:
    void openScratch(const QModelIndex& index);
    void runScratch(const QModelIndex& index);
    void removeScratch(const QModelIndex& index);
    void createScratch(const QString& name);
    void renameScratch(const QModelIndex& index, const QString& previousName);
    void setCommand(const QModelIndex& index, const QString& command);

Q_SIGNALS:
    void actionFailed(const QString& message);

private:
    void addFileToModel(const QFileInfo& fileInfo);

    ScratchpadToolViewFactory* m_factory;
    QStandardItemModel* m_model;
    QFileIconProvider m_iconProvider;

    QAction* const m_runAction;
};

#endif // SCRATCHPAD_H
