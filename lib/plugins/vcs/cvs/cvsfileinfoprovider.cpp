/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsfileinfoprovider.h"

#include <KDebug>
#include <QDir>

#include "cvsproxy.h"
#include "cvsjob.h"

CvsFileInfoProvider::CvsFileInfoProvider(CvsProxy * proxy, QObject* parent)
    : QObject(parent), m_proxy(proxy)
{
}

CvsFileInfoProvider::~CvsFileInfoProvider()
{
}

bool CvsFileInfoProvider::requestStatusASync(const KUrl & directory, KDevelop::IVersionControl::WorkingMode mode)
{
    bool recursive = (mode == KDevelop::IVersionControl::Recursive)?true:false;

    CvsJob* job = m_proxy->status( directory.path(), recursive );
    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SLOT( slotJobFinished(KJob*) ));

        // Mergind stdout and stderr is needed in this case, as K3Process will not emit the stderr signal
        // immediatly when stderr output is received. As cvs tells directory changes via stderr, we would 
        // be unable to tell wich part from the ouput to stdout belongs to which directory.
        job->setCommunicationMode( K3Process::Stdout | K3Process::MergedStderr );
        job->start();
        return true;
    }

    return false;
}

QList< KDevelop::VcsFileInfo > CvsFileInfoProvider::requestStatusSync(const KUrl & directory, KDevelop::IVersionControl::WorkingMode mode)
{
    bool recursive = (mode == KDevelop::IVersionControl::Recursive)?true:false;

    CvsJob* job = m_proxy->status( directory.path(), recursive );

    //see comment in requestStatusASync() why we do this here
    job->setCommunicationMode( K3Process::Stdout | K3Process::MergedStderr );

    if ( job->exec() ) {
        QList<KDevelop::VcsFileInfo> infos;
        parseOutput(job->output(), infos);

        kDebug() << "Fetched status in sync:" << endl;
        foreach (KDevelop::VcsFileInfo info, infos) {
             kDebug() << info.toString() << endl;
        }

        return infos;
    }
    return QList<KDevelop::VcsFileInfo>();
}

void CvsFileInfoProvider::slotJobFinished(KJob * job)
{
    kDebug() << k_funcinfo << endl;

    if ( job->error() )
    {
        return;
    }

    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (!cvsjob) {
        return;
    }

    QList<KDevelop::VcsFileInfo> infos;
    parseOutput(cvsjob->output(), infos);

    foreach (KDevelop::VcsFileInfo info, infos) {
         kDebug() << info.toString() << endl;
    }

    emit statusReady( infos );
}

void CvsFileInfoProvider::parseOutput(const QString & output, QList<KDevelop::VcsFileInfo>& infos)
{
    QString filename;
    QString status;
    QString reporev;
    QString workrev;

    static QRegExp re_start("^=+$");
    static QRegExp re_file("File:\\s+(.*)\\s+Status:\\s+(.*)");
    static QRegExp re_workrev("\\s+Working revision:\\s+([\\d\\.]*).*");
    static QRegExp re_reporev("\\s+Repository revision:\\s+([\\d\\.]*).*");
    static QRegExp re_dirchange("cvs status: Examining\\s+(.*)");

    QString currentDir;

    QStringList lines = output.split("\n");
    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];

        if (s.isEmpty())
            continue;

        if ( re_start.exactMatch(s) ) {
            if ( !filename.isEmpty() ) {
//                 kDebug() << "File: " << filename << " Status: " << status
//                          << " working: " << workrev << " repo: " << reporev << endl;

                // join the current directory (if any) and the found filename ...
                QString file = currentDir;
                if (file.length() > 0)
                    file += QDir::separator();
                file += filename;

                // ... and create a VcsFileInfo entry
                KDevelop::VcsFileInfo info( file, workrev, reporev,
                                  String2EnumState( status ) );
                infos << info;
            }
            filename.clear();
            status.clear();
            reporev.clear();
            workrev.clear();
        } else if ( re_file.exactMatch(s) ) {
            filename = re_file.cap(1).trimmed();
            status = re_file.cap(2).trimmed();
        } else if ( re_workrev.exactMatch(s) ) {
            workrev = re_workrev.cap(1);
        } else if ( re_reporev.exactMatch(s) ) {
            reporev = re_reporev.cap(1);
        } else if ( re_dirchange.exactMatch(s) ) {
            currentDir = re_dirchange.cap(1);
            if (currentDir == ".")
                currentDir.clear();
        }
    }
}

KDevelop::VcsFileInfo::VcsFileState CvsFileInfoProvider::String2EnumState(QString stateAsString)
{
    if (stateAsString == "Up-to-date")
        return KDevelop::VcsFileInfo::Uptodate;
    else if (stateAsString == "Locally Modified")
        return KDevelop::VcsFileInfo::Modified;
    else if (stateAsString == "Locally Added")
        return KDevelop::VcsFileInfo::Added;
    else if (stateAsString == "Unresolved Conflict")
        return KDevelop::VcsFileInfo::Conflict;
    else if (stateAsString == "Needs Patch")
        return KDevelop::VcsFileInfo::NeedsPatch;
    else if (stateAsString == "Needs Checkout")
        return KDevelop::VcsFileInfo::NeedsCheckout;
    else
        return KDevelop::VcsFileInfo::Unknown;
}

#include "cvsfileinfoprovider.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
