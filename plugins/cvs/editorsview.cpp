/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editorsview.h"

#include <QFileInfo>
#include <QTextBrowser>
#include <QRegExp>
#include <KMessageBox>
#include <KI18n/KLocalizedString>

#include "cvsplugin.h"
#include "cvsjob.h"
#include "cvsproxy.h"


EditorsView::EditorsView(CvsJob* job, QWidget *parent)
    : QWidget(parent), Ui::EditorsViewBase()
{
    Ui::EditorsViewBase::setupUi(this);

    if (job) {
        connect(job, &CvsJob::result,
                this, &EditorsView::slotJobFinished);
    }
}

EditorsView::~EditorsView()
{
}

void EditorsView::slotJobFinished(KJob* job)
{
    if ( job->error() )
    {
        textbrowser->append( i18n("Listing editors failed") );
        return;
    }

    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (!cvsjob) {
        return;
    }


    QMultiMap<QString,CvsLocker> lockedFiles;

    parseOutput(cvsjob->output(), lockedFiles);

    if (lockedFiles.size() == 0) {
        textbrowser->append(i18n("No files from your query are marked as being edited."));
    } else {
        QString html;

        foreach (const QString &key, lockedFiles.uniqueKeys()) {
            html += "<h3>"+key+"</h3><br>";

            foreach(const CvsLocker &item, lockedFiles.values( key )) {
                html += "<b>"+i18n("User")+":</b> "+item.user+"<br>";
                html += "<b>"+i18n("Date")+":</b> "+item.date+"<br>";
                html += "<b>"+i18n("Machine")+":</b> "+item.machine+"<br>";
                html += "<b>"+i18n("Repository")+":</b> "+item.localrepo+"<br>";
                html += "<br>";
            }
            html += "<br>";
        }

        textbrowser->setHtml( html );
    }
}

void EditorsView::parseOutput(const QString& jobOutput, QMultiMap<QString,CvsLocker>& editorsInfo)
{
    // the first line contains the filename and than the locker information
    static QRegExp re("([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+"
                        "([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+(.*)");
    // if there are more than one locker of a single file, the second line for a file
    // only contains the locker information (no filename)
    static QRegExp subre("\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+"
                        "([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+(.*)");

    QString lastfilename;

    QStringList lines = jobOutput.split('\n');

    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];

        if (re.exactMatch(s)) {
            CvsLocker item;
            QString file = re.cap(1);
            item.user = re.cap(2);
            item.date = re.cap(5)+' '+re.cap(4)+' '+re.cap(7)+' '+re.cap(6);
            item.machine = re.cap(9);
            item.localrepo = re.cap(10);

            editorsInfo.insert(file, item);

            lastfilename = file;
        } else {
            if (subre.exactMatch(s)) {
                CvsLocker item;
                item.user = subre.cap( 1 );
                item.date = subre.cap(4)+' '+subre.cap(3)+' '+subre.cap(6)+' '+subre.cap(5);
                item.machine = subre.cap(8);
                item.localrepo = subre.cap(9);

                editorsInfo.insert(lastfilename, item);
            }
        }
    }
}

