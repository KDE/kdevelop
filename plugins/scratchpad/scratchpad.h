/*
    SPDX-FileCopyrightText: 2018 Amish K. Naidu <amhndu@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include <interfaces/iplugin.h>

#include <QFileIconProvider>
#include <QString>

class ScratchpadToolViewFactory;

class QStandardItemModel;
class QModelIndex;
class QFileInfo;
class QAction;

class Scratchpad
    : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    Scratchpad(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);

    void unload() override;

    QStandardItemModel* model() const;

    QAction* runAction() const;

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

    const QString m_dataDirectory;
    ScratchpadToolViewFactory* m_factory;
    QStandardItemModel* m_model;
    QFileIconProvider m_iconProvider;

    QAction* const m_runAction;
};

#endif // SCRATCHPAD_H
