/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DOCKERPLUGIN_H
#define DOCKERPLUGIN_H

#include <interfaces/iruntime.h>
#include <interfaces/iplugin.h>
#include <util/path.h>
// Qt
#include <QProcess>
#include <QHash>

class DockerPreferencesSettings;

class DockerPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    DockerPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~DockerPlugin() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    void imagesListFinished(int code);

    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

Q_SIGNALS:
    ///only used by the test
    void imagesListed();

private:
    void runtimeChanged(KDevelop::IRuntime* newRuntime);

    QHash<KDevelop::Path, KDevelop::IRuntime *> m_runtimes;
    QScopedPointer<DockerPreferencesSettings> m_settings;
};

#endif
