/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for DVCS                                                      *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

#include "dvcsplugin.h"
#include "dvcsjob.h"

LogView::LogView(KDevelop::DistributedVersionControlPlugin* plugin, DVCSjob* job, QWidget *parent)
    : QWidget(parent), Ui::LogViewBase(), m_plugin(plugin)
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

    DVCSjob * dvcsJob = dynamic_cast<DVCSjob*>(job);
    if (!dvcsJob) {
        return;
    }

    QList<DVCScommit> logEntries;
    m_plugin->proxy()->parseOutput(dvcsJob->output(), logEntries);

    if (logEntries.size() == 0) {
        textbrowser->append(i18n("No log information found"));
    } else {
        QString html;

        foreach(DVCScommit item, logEntries) {
            html += "<b>"+i18n("Commit")+":</b> "+item.commit+"<br>";
            html += "<b>"+i18n("Author")+":</b> "+item.author+"<br>";
            html += "<b>"+i18n("Date")+":</b> "+item.date+"<br>";
            html += item.log+"<br>";
            html += "<br>";
        }
        html += "<br>";

        textbrowser->setHtml( html );
    }
}

#include "logview.moc"
