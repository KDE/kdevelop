/*
 * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
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
        explicit ExecutePlasmoidPlugin(QObject *parent, const QVariantList & = QVariantList() );
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
