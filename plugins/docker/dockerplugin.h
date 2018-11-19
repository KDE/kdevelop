/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
    DockerPlugin(QObject *parent, const QVariantList & args);
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
