/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef FLATPAKPLUGIN_H
#define FLATPAKPLUGIN_H

#include <interfaces/iruntime.h>
#include <interfaces/iplugin.h>
#include <util/path.h>
// Qt
#include <QProcess>
#include <QHash>

class FlatpakPlugin : public KDevelop::IPlugin
{
Q_OBJECT
public:
    FlatpakPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~FlatpakPlugin() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

private:
    void runtimeChanged(KDevelop::IRuntime* newRuntime);
    void rebuildCurrent();
    void exportCurrent();
    void setupArches();
    void executeOnRemoteDevice();
    void createRuntime(const KDevelop::Path &file, const QString &arch);

    QHash<KDevelop::Path, KDevelop::IRuntime *> m_runtimes;
};

#endif
