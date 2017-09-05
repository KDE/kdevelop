/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvslogjob.h"
#include "debug.h"

#include <QRegExp>
#include <QDateTime>

#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>

CvsLogJob::CvsLogJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : CvsJob(parent, verbosity)
{
}

CvsLogJob::~CvsLogJob()
{
}

QVariant CvsLogJob::fetchResults()
{
    // Convert job's output into KDevelop::VcsEvent
    QList<QVariant> events;
    parseOutput(output(), events);

    return events;
}

void CvsLogJob::parseOutput(const QString& jobOutput, QList<QVariant>& events)
{
    static QRegExp rx_sep( "[-=]+" );
    static QRegExp rx_rev( "revision ((\\d+\\.?)+)" );
    static QRegExp rx_branch( "branches:\\s+(.*)" );
    static QRegExp rx_date( "date:\\s+([^;]*);\\s+author:\\s+([^;]*).*" );


    QStringList lines = jobOutput.split('\n');

    KDevelop::VcsEvent item;
    bool firstSeperatorReached = false;
    QString log;

    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];
//         qCDebug(PLUGIN_CVS) << "line:" << s ;

        if (rx_rev.exactMatch(s)) {
//             qCDebug(PLUGIN_CVS) << "MATCH REVISION" ;
            KDevelop::VcsRevision rev;
            rev.setRevisionValue( rx_rev.cap(1), KDevelop::VcsRevision::FileNumber );
            item.setRevision( rev );
        } else if (rx_branch.exactMatch(s)) {
//             qCDebug(PLUGIN_CVS) << "MATCH BRANCH" ;
        } else if (rx_date.exactMatch(s)) {
//             qCDebug(PLUGIN_CVS) << "MATCH DATE" ;
            QString date = rx_date.cap(1);
            // cut out the part that matches the Qt::ISODate format
            date.truncate(19);

            item.setDate( QDateTime::fromString( date, Qt::ISODate ) );
            item.setAuthor( rx_date.cap(2) );
        } else  if (rx_sep.exactMatch(s)) {
//             qCDebug(PLUGIN_CVS) << "MATCH SEPARATOR" ;
            if (firstSeperatorReached) {
                item.setMessage( log );
                log.clear();

                events.append( qVariantFromValue( item ) );

                KDevelop::VcsEvent empty;
                item = empty;
            } else {
                firstSeperatorReached = true;
            }
        } else {
            if (firstSeperatorReached) {
//                 qCDebug(PLUGIN_CVS) << "ADDING LOG" ;
                log += s+'\n';
            }
        }
    }
}

