/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EXECUTEPLASMOIDPLUGIN_H
#define EXECUTEPLASMOIDPLUGIN_H

#include <interfaces/iplugin.h>
#include <execute/iexecuteplugin.h>
#include <QVariantList>

class PlasmoidExecutionConfigType;
class QUrl;
class KJob;

class ExecutePlasmoidPlugin : public KDevelop::IPlugin, public IExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES( IExecutePlugin )
    public:
        explicit ExecutePlasmoidPlugin(QObject* parent, const KPluginMetaData& metaData,
                                       const QVariantList& = QVariantList());
        ~ExecutePlasmoidPlugin() override;
        void unload() override;

        QUrl executable(KDevelop::ILaunchConfiguration* config, QString& error) const override;
        QStringList arguments(KDevelop::ILaunchConfiguration* config, QString& error) const override;
        KJob* dependencyJob(KDevelop::ILaunchConfiguration* config) const override;
        QString environmentProfileName(KDevelop::ILaunchConfiguration* config) const override;
        QString nativeAppConfigTypeId() const override;
        QString terminal(KDevelop::ILaunchConfiguration* config) const override;
        bool useTerminal(KDevelop::ILaunchConfiguration* config) const override;
        QUrl workingDirectory(KDevelop::ILaunchConfiguration* config) const override;

    private:
        PlasmoidExecutionConfigType* m_configType;
};

#endif // EXECUTEPLASMOIDPLUGIN_H
