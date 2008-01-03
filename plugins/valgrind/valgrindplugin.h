/* This file is part of KDevelop
 *  Copyright 2002 Harald Fernengel <harry@kdevelop.org>
 *  Copyright 2007 Hamish Rodda <rodda@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef VALGRINDPLUGIN_H
#define VALGRINDPLUGIN_H

#include <QPointer>
#include <QVariant>

#include <iplugin.h>
#include <irunprovider.h>
#include <istatus.h>

class QTreeView;
class ValgrindModel;
class ValgrindControl;
class ValgrindModel;

class ValgrindPlugin : public KDevelop::IPlugin, public KDevelop::IRunProvider//, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IRunProvider)
    //Q_INTERFACES(KDevelop::IStatus)

    friend class ValgrindControl;

public:
    ValgrindPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~ValgrindPlugin();

    KUrl valgrindExecutable() const;

    QList<ValgrindModel*> models() const;
    
    // BEGIN IRunProvider
    virtual QStringList instrumentorsProvided() const;
    virtual bool execute(const KDevelop::IRun& run, int serial);
    virtual void abort(int serial);

signals:
    void finished(int serial);
    void output(int serial, const QString& line, KDevelop::IRunProvider::OutputTypes type);

    void newModel(ValgrindModel* model);

//   void savePartialProjectSession( QDomElement* el );
//   void restorePartialProjectSession( const QDomElement* el );

private slots:
    void slotExecValgrind();
    void slotExecCalltree();
    void loadOutput();

private:
    QString m_lastExec, m_lastParams, m_lastValExec, m_lastValParams,
        m_lastCtExec, m_lastCtParams, m_lastKcExec;

    QHash<int, ValgrindControl*> m_controls;
};

#endif // VALGRINDPLUGIN_H
