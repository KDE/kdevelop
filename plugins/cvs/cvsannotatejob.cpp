/***************************************************************************
 *   Copyright 2008 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cvsannotatejob.h"
#include "debug.h"

#include <QUrl>
#include <QDir>
#include <QLocale>
#include <QDateTime>

#include <vcs/vcsrevision.h>

CvsAnnotateJob::CvsAnnotateJob(KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity)
    : CvsJob(parent, verbosity)
{
}

CvsAnnotateJob::~CvsAnnotateJob()
{
}

QVariant CvsAnnotateJob::fetchResults()
{
    // Convert job's output into KDevelop::VcsAnnotation
    KDevelop::VcsAnnotation annotateInfo;
    parseOutput(output(), getDirectory(), annotateInfo);

    QList<QVariant> lines;
    for(int i=0; i < annotateInfo.lineCount(); i++) {
        KDevelop::VcsAnnotationLine line = annotateInfo.line(i);
        lines.append( qVariantFromValue( line ) );
    }

    return lines;
}

void CvsAnnotateJob::parseOutput(const QString& jobOutput, const QString& workingDirectory, KDevelop::VcsAnnotation& annotateInfo)
{
    // each annotation line looks like this:
    // 1.1 (kdedev 10-Nov-07): #include <QApplication>
    static QRegExp re("([^\\s]+)\\s+\\(([^\\s]+)\\s+([^\\s]+)\\):\\s(.*)");

    // the file is pomoted like this:
    // Annotations for main.cpp
    static QRegExp reFile("Annotations for\\s(.*)");

    QStringList lines = jobOutput.split('\n');

    QString filename;
    for (int i=0, linenumber=0; i<lines.count(); ++i) {
        QString s = lines[i];

        if (re.exactMatch(s)) {
            KDevelop::VcsAnnotationLine item;

            item.setLineNumber( linenumber );
            item.setText( re.cap(4) );
            item.setAuthor( re.cap(2)  );

            KDevelop::VcsRevision rev;
            rev.setRevisionValue( re.cap(1), KDevelop::VcsRevision::FileNumber );
            item.setRevision( rev );

            // cvs annotate always prints the date with english month names.
            // Using QDate::fromString() directly would fail as it works with
            // localized month names. So we let QLocale do the work .
            QDate date(QLocale::c().toDate(re.cap(3), QLatin1String("dd-MMM-yy")));
            if (date.year() < 1970)
                date = date.addYears(100);
            item.setDate( QDateTime(date, QTime(), Qt::UTC) );

            annotateInfo.insertLine( linenumber, item );
            linenumber++;
        } else if (reFile.exactMatch(s)) {
            QUrl url = QUrl::fromLocalFile(workingDirectory + QDir::separator() + reFile.cap(1));
            annotateInfo.setLocation( url );
        } else {
            qCDebug(PLUGIN_CVS) << "Unmatched:"<<s<<endl;
        }
    }
}

