/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "logview.h"

#include <QFileInfo>
#include <QTextBrowser>
#include <QRegExp>
#include <QMultiMap>
#include <KDebug>
#include <KMessageBox>

#include "cvspart.h"
#include "cvsjob.h"
#include "cvsproxy.h"


LogView::LogView(CvsPart* part, CvsJob* job, QWidget *parent)
    : QWidget(parent), Ui::LogViewBase(), m_part(part)
{
    Ui::LogViewBase::setupUi(this);

    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SLOT( slotJobFinished(KJob*) ));
    }
}

LogView::~LogView()
{
}

void LogView::slotJobFinished(KJob* job)
{
    if ( job->error() )
    {
        textbrowser->append( i18n("Listing the files's log failed") );
        return;
    }

    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (!cvsjob) {
        return;
    }

    QList<CvsRevision> logEntries;
    parseOutput(cvsjob->output(), logEntries);

    if (logEntries.size() == 0) {
        textbrowser->append(i18n("No log information found"));
    } else {
        QString html;

        foreach(CvsRevision item, logEntries) {
            html += "<b>"+i18n("Revision")+":</b> "+item.revision+"<br>";
            html += "<b>"+i18n("User")+":</b> "+item.user+"<br>";
            html += "<b>"+i18n("Date")+":</b> "+item.date+"<br>";
            html += item.log+"<br>";
            html += "<br>";
        }
        html += "<br>";

        textbrowser->setHtml( html );
    }
}

void LogView::parseOutput(const QString& jobOutput, QList<CvsRevision>& revisions)
{
    static QRegExp rx_sep( "[-=]+" );
    static QRegExp rx_rev( "revision ((\\d+\\.?)+)" );
    static QRegExp rx_branch( "branches:\\s+(.*)" );
    static QRegExp rx_date( "date:\\s+([^;]*);\\s+author:\\s+([^;]*).*" );


    QStringList lines = jobOutput.split("\n");

    CvsRevision item;
    bool firstSeperatorReached = false;

    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];
//         kDebug(9500) << "line:" << s ;

        if (rx_rev.exactMatch(s)) {
//             kDebug(9500) << "MATCH REVISION" ;
            item.revision = rx_rev.cap(1);
        } else if (rx_branch.exactMatch(s)) {
//             kDebug(9500) << "MATCH BRANCH" ;
        } else if (rx_date.exactMatch(s)) {
//             kDebug(9500) << "MATCH DATE" ;
            item.date = rx_date.cap(1);
            item.user = rx_date.cap(2);
        } else  if (rx_sep.exactMatch(s)) {
//             kDebug(9500) << "MATCH SEPERATOR" ;
            if (firstSeperatorReached) {
                revisions.append( item );

                item.user.clear();
                item.date.clear();
                item.revision.clear();
                item.log.clear();
            } else {
                firstSeperatorReached = true;
            }
        } else {
            if (firstSeperatorReached) {
//                 kDebug(9500) << "ADDING LOG" ;
                item.log += s+'\n';
            }
        }
    }
}

#include "logview.moc"
