/*
 * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef EXECUTEPLUGIN_H
#define EXECUTEPLUGIN_H

#include <iplugin.h>
#include <QtCore/QVariant>
#include <QtCore/QProcess>

#include <irunprovider.h>
namespace KDevelop
{

class ExecutePlugin : public KDevelop::IPlugin, public KDevelop::IRunProvider
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IRunProvider)

  public:
    ExecutePlugin(QObject *parent, const QVariantList & = QVariantList() );
    virtual ~ExecutePlugin();

    virtual void unload();

    virtual QStringList instrumentorsProvided() const;

    /**
     * Request the execution of \a run.
     */
    virtual bool execute(const IRun& run, int serial);

    /**
     * Request the aborting of a run with the given \a serial number.
     */
    virtual void abort(int serial);

  Q_SIGNALS:
    // implementations from IRunProvider
    void finished(int serial);
    void output(int serial, const QString& line, IRunProvider::OutputTypes type);

  private Q_SLOTS:
    void readyReadStandardOutput();
    void readyReadStandardError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

  private:
    void readFrom(QProcess* process, QProcess::ProcessChannel channel);

    QMap<int, QProcess*> m_runners;
};

}

#endif // EXECUTEPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
