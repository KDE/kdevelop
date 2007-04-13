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

class CvsLog {
public:
    QString revision;
    QString date;
    QString user;
    QString log;
};

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
    kDebug() << k_funcinfo << endl;
}

void LogView::slotJobFinished(KJob* job)
{
    kDebug() << k_funcinfo << endl;

    if ( job->error() )
    {
        textbrowser->append( i18n("Listing the files's log failed") );
        return;
    }

    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (!cvsjob) {
        return;
    }

    QStringList lines = cvsjob->output().split("\n");

    static QRegExp rx_sep( "[-=]+" );
    static QRegExp rx_rev( "revision ((\\d+\\.?)+)" );
    static QRegExp rx_branch( "branches:\\s+([^;]+).*" );
    static QRegExp rx_date( "date:\\s+([^;]*);\\s+author:\\s+([^;]*).*" );

    QList<CvsLog> logEntries;

    int found = 0;
    bool firstSeperatorReached = false;

    CvsLog item;

    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];
//         kDebug() << "line: " << s <<endl;

        if (rx_rev.exactMatch(s)) {
//             kDebug() << "MATCH REVISION" <<endl;
            item.revision = rx_rev.cap(1);
        } else if (rx_branch.exactMatch(s)) {
//             kDebug() << "MATCH BRANCH" <<endl;
        } else if (rx_date.exactMatch(s)) {
//             kDebug() << "MATCH DATE" <<endl;
            item.date = rx_date.cap(1);
            item.user = rx_date.cap(2);
        } else  if (rx_sep.exactMatch(s)) {
//             kDebug() << "MATCH SEPERATOR" <<endl;
            if (firstSeperatorReached) {
                logEntries.append( item );

                item.user.clear();
                item.date.clear();
                item.revision.clear();
                item.log.clear();
            } else {
                firstSeperatorReached = true;
            }
            found++;
        } else {
            if (firstSeperatorReached) {
//                 kDebug() << "ADDING LOG" <<endl;
                item.log += s+"\n";
            }
        }
    }

    if (!found) {
        textbrowser->append(i18n("No log information found"));
    } else {
        QString html;

        foreach(CvsLog item, logEntries) {
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

#include "logview.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
