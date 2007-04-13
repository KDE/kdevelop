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

#include "editorsview.h"

#include <QFileInfo>
#include <QTextBrowser>
#include <QRegExp>
#include <QMultiMap>
#include <KDebug>
#include <KMessageBox>

#include "cvspart.h"
#include "cvsjob.h"
#include "cvsproxy.h"

class CvsLocker {
public:
    QString user;
    QString date;
    QString machine;
    QString localrepo;
};

EditorsView::EditorsView(CvsPart* part, CvsJob* job, QWidget *parent)
    : QWidget(parent), Ui::EditorsViewBase(), m_part(part)
{
    Ui::EditorsViewBase::setupUi(this);

    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SLOT( slotJobFinished(KJob*) ));
    }
}

EditorsView::~EditorsView()
{
    kDebug() << k_funcinfo << endl;
}

void EditorsView::slotJobFinished(KJob* job)
{
    kDebug() << k_funcinfo << endl;

    if ( job->error() )
    {
        textbrowser->append( i18n("Listing editors failed") );
        return;
    }

    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (!cvsjob) {
        return;
    }

    static QRegExp re("([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+"
                        "([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+(.*)");
    static QRegExp subre("\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+"
                        "([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+([^\\s]+)\\s+(.*)");
    QString lastfilename;

    QStringList lines = cvsjob->output().split("\n");

    QMultiMap<QString,CvsLocker> lockedFiles;

    int found = 0;
    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];

        if (re.exactMatch(s)) {
            CvsLocker item;
            QString file = re.cap(1);
            item.user = re.cap(2);
            item.date = re.cap(5)+" "+re.cap(4)+" "+re.cap(7)+" "+re.cap(6);
            item.machine = re.cap(9);
            item.localrepo = re.cap(10);

            lockedFiles.insert(file, item);

            found++;

            lastfilename = file;
        } else {
            if (subre.exactMatch(s)) {
                CvsLocker item;
                item.user = subre.cap( 1 );
                item.date = subre.cap(4)+" "+subre.cap(3)+" "+subre.cap(6)+" "+subre.cap(5);
                item.machine = subre.cap(8);
                item.localrepo = subre.cap(9);

                lockedFiles.insert(lastfilename, item);

                found++;
            }
        }
    }

    if (!found) {
        textbrowser->append(i18n("No files from your query are marked as being edited."));
    } else {
        QString html;

        foreach (QString key, lockedFiles.uniqueKeys()) {
            html += "<h3>"+key+"</h3><br>";

            foreach(CvsLocker item, lockedFiles.values( key )) {
                html += "<b>"+i18n("User")+":</b>"+item.user+"<br>";
                html += "<b>"+i18n("Date")+":</b>"+item.date+"<br>";
                html += "<b>"+i18n("Machine")+":</b>"+item.machine+"<br>";
                html += "<b>"+i18n("Repository")+":</b>"+item.localrepo+"<br>";
                html += "<br>";
            }
            html += "<br>";
        }

        textbrowser->setHtml( html );
    }
}

#include "editorsview.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
