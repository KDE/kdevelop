/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef OUTPUTVIEWCOMMAND_H
#define OUTPUTVIEWCOMMAND_H

#include <QtCore/QObject>
#include <QtCore/QProcess>

class KUrl;
class QStringList;
class QString;
class QStandardItemModel;
class ProcessLineMaker;
template <typename T1, typename T2> class QMap;
class IOutputViewItemFactory;

class OutputViewCommand : public QObject
{
Q_OBJECT
public:
    OutputViewCommand( const KUrl& workdir, const QStringList& command,
                       const QMap<QString, QString>& env, QStandardItemModel* model = 0,
                       IOutputViewItemFactory *factory = 0 );
    virtual ~OutputViewCommand();
    virtual void start();

    void setModel( QStandardItemModel *model );
    QStandardItemModel *model();
    QString title();

    static QMap<QString,QString> buildEnvMap( const QStringList& );
    static QStringList buildEnvList( const QMap<QString,QString>& );

    private Q_SLOTS:
        void procReadStdout( const QStringList& lineList );
        void procReadStderr( const QStringList& lineList );
        void procFinished( int, QProcess::ExitStatus );
    signals:
        void commandFinished( const QString& command );
        void commandFailed( const QString& command );
    private:
        QProcess *m_proc;
        ProcessLineMaker *m_procLineMaker;
        QStandardItemModel* m_model;
        QString m_command;
        IOutputViewItemFactory *m_factory;
};


#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
