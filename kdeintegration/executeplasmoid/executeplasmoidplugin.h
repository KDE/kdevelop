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
#include <QtCore/QVariant>
#include <QtCore/QProcess>

class PlasmoidExecutionConfigType;
class QUrl;
class KJob;

class ExecutePlasmoidPlugin : public KDevelop::IPlugin, public IExecutePlugin
{
    Q_OBJECT
    Q_INTERFACES( IExecutePlugin )
    public:
        ExecutePlasmoidPlugin(QObject *parent, const QVariantList & = QVariantList() );
        virtual ~ExecutePlasmoidPlugin();
        virtual void unload() override;

        virtual QUrl executable(KDevelop::ILaunchConfiguration* config, QString& error) const override;
        virtual QStringList arguments(KDevelop::ILaunchConfiguration* config, QString& error) const override;
        virtual KJob* dependencyJob(KDevelop::ILaunchConfiguration* config) const override;
        virtual QString environmentGroup(KDevelop::ILaunchConfiguration* config) const override;
        virtual QString nativeAppConfigTypeId() const override;
        virtual QString terminal(KDevelop::ILaunchConfiguration* config) const override;
        virtual bool useTerminal(KDevelop::ILaunchConfiguration* config) const override;
        virtual QUrl workingDirectory(KDevelop::ILaunchConfiguration* config) const override;

    private:
        PlasmoidExecutionConfigType* m_configType;
};

#endif // EXECUTEPLASMOIDPLUGIN_H
