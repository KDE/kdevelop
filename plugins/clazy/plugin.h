/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_PLUGIN_H
#define KDEVCLAZY_PLUGIN_H

// KDevPlatform
#include <interfaces/iplugin.h>
// Qt
#include <QSharedPointer>

namespace Clazy
{

class ChecksDB;
class Analyzer;
class CheckSetSelectionManager;

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~Plugin() override;

public: // KDevelop::IPlugin API
    void unload() override;
    int configPages() const override { return 1; }
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

    int perProjectConfigPages() const override { return 1; }
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

public:
    QSharedPointer<const ChecksDB> checksDB() const;
    QSharedPointer<const ChecksDB> loadedChecksDB();

private:
    void reloadDB();

private:
    Analyzer* m_analyzer;
    QSharedPointer<ChecksDB> m_db;
    CheckSetSelectionManager* m_checkSetSelectionManager;
};

}

#endif
