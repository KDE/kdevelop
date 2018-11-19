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
    FlatpakPlugin(QObject *parent, const QVariantList & args);
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
