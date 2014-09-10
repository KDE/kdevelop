/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsstatusjob.h"

#include <KDebug>
#include <QUrl>
#include <QDir>
#include <QStringList>

#include <vcs/vcsstatusinfo.h>


CvsStatusJob::CvsStatusJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : CvsJob(parent, verbosity)
{
}

CvsStatusJob::~CvsStatusJob()
{
}

QVariant CvsStatusJob::fetchResults()
{
    // Convert job's output into KDevelop::VcsStatusInfo
    QList<QVariant> infos;
    parseOutput(output(), infos);

    return infos;
}

void CvsStatusJob::addInfoToList(QList<QVariant>& infos, 
        const QString& currentDir, const QString& filename, 
        const QString& statusString)
{
    KDevelop::VcsStatusInfo::State cvsState = String2EnumState( statusString );

    QString correctedFilename = filename;
    if (cvsState == KDevelop::VcsStatusInfo::ItemDeleted) {
        // cvs status writes "no file" in front of the filename
        // in case the file was locally removed
        correctedFilename.remove("no file ");
    }

    // join the current directory (if any) and the found filename ...
    // note: current directy is always relative to the directory where the 
    //       cvs command was executed
    QString file = currentDir;
    if (file.length() > 0) {
        file += QDir::separator();
    }
    file += correctedFilename;

    // ... and create a VcsFileInfo entry
    KDevelop::VcsStatusInfo info;
    info.setUrl(QUrl::fromLocalFile(QString(getDirectory() + QDir::separator() + file)));
    info.setState(cvsState);

    kDebug(9500) << "Added status of: " << info.url() << endl;
    infos << qVariantFromValue( info );
}

void CvsStatusJob::parseOutput(const QString& jobOutput, QList<QVariant>& infos)
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

    QStringList lines = jobOutput.split('\n');
    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];

        if (s.isEmpty())
            continue;

        if ( re_start.exactMatch(s) ) {
            if ( !filename.isEmpty() ) {
//                kDebug(9500) << "File:" << filename << "Status:" << status
//                    << "working:" << workrev << "repo:" << reporev << endl;

                addInfoToList( infos, currentDir, filename, status );
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

    if ( !filename.isEmpty() ) {
//        kDebug(9500) << "File:" << filename << "Status:" << status
//            << "working:" << workrev << "repo:" << reporev << endl;

        addInfoToList( infos, currentDir, filename, status );
    }

}

KDevelop::VcsStatusInfo::State CvsStatusJob::String2EnumState(const QString& stateAsString)
{
    KDevelop::VcsStatusInfo::State state;

    if (stateAsString == "Up-to-date")
        return KDevelop::VcsStatusInfo::ItemUpToDate;
    else if (stateAsString == "Locally Modified")
        return KDevelop::VcsStatusInfo::ItemModified;
    else if (stateAsString == "Locally Added")
        return KDevelop::VcsStatusInfo::ItemAdded;
    else if (stateAsString == "Locally Removed")
        return KDevelop::VcsStatusInfo::ItemDeleted;
    else if (stateAsString == "Unresolved Conflict")
        return KDevelop::VcsStatusInfo::ItemHasConflicts;
    else if (stateAsString == "Needs Patch") 
        return KDevelop::VcsStatusInfo::ItemUpToDate;
    else
        return KDevelop::VcsStatusInfo::ItemUnknown;

    return state;
}

