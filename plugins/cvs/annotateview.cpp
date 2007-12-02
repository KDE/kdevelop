/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "annotateview.h"

#include <QTextBrowser>
#include <QRegExp>
#include <QDir>

#include "cvsplugin.h"
#include "cvsjob.h"


AnnotateView::AnnotateView(CvsPlugin* plugin, CvsJob* job, QWidget *parent)
    : QWidget(parent), Ui::AnnotateViewBase(), m_plugin(plugin)
{
    Ui::AnnotateViewBase::setupUi(this);

    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SLOT( slotJobFinished(KJob*) ));
    }
}

AnnotateView::~AnnotateView()
{
}

void AnnotateView::slotJobFinished(KJob* job)
{
    if ( job->error() )
    {
        textbrowser->append( i18n("Listing annotations failed") );
        return;
    }

    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (!cvsjob) {
        return;
    }


    KDevelop::VcsAnnotation annotateInfo;

    parseOutput(cvsjob->output(), cvsjob->getDirectory(), annotateInfo);

    /// @todo better us a QModel and a QTableView here
    QString html;

    html += annotateInfo.location().path()+"\n\n";
    for(int i=1; i < annotateInfo.lineCount(); i++) {
        KDevelop::VcsAnnotationLine line = annotateInfo.line(i);

        html += QString::number(line.lineNumber())+":";
        html += line.revision().revisionValue().toString()+":";
        html += line.author()+":";
        html += line.text()+"\n";
    }

    textbrowser->setPlainText( html );
}

void AnnotateView::parseOutput(const QString& jobOutput, const QString& workingDirectory, KDevelop::VcsAnnotation& annotateInfo)
{
    // each annotation line looks like this:
    // 1.1 (kdedev 10-Nov-07): #include <QApplication>
    static QRegExp re("([^\\s]+)\\s+\\(([^\\s]+)\\s+([^\\s]+)\\):\\s(.*)");

    // the file is pomoted like this:
    // Annotations for main.cpp
    static QRegExp reFile("Annotations for\\s(.*)");

    QStringList lines = jobOutput.split("\n");

    QString filename;
    for (int i=0; i<lines.count(); ++i) {
        QString s = lines[i];

        if (re.exactMatch(s)) {
            KDevelop::VcsAnnotationLine item;

            item.setLineNumber( i+1 );
            item.setText( re.cap(4) );
            item.setAuthor( re.cap(2)  );

            KDevelop::VcsRevision rev;
            rev.setRevisionValue( re.cap(1), KDevelop::VcsRevision::FileNumber );
            item.setRevision( rev );

            ///@todo find correct time format code
            //item.setDate( QDateTime::fromString( re.cap(3)/*, Qt::ISODate*/ ) );

            annotateInfo.insertLine( i+1, item );
        } else if (reFile.exactMatch(s)) {
            KUrl url(workingDirectory + QDir::separator() + reFile.cap(1));
            annotateInfo.setLocation( url );
        }
    }
}

#include "annotateview.moc"
