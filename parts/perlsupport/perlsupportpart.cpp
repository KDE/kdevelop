/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kregexp.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "classstore.h"

#include "perlsupportpart.h"
#include "perlsupportfactory.h"
#include "parsedclass.h"
#include "parsedmethod.h"


PerlSupportPart::PerlSupportPart(KDevApi *api, QObject *parent, const char *name)
    : KDevLanguageSupport(api, parent, name)
{
    setInstance(PerlSupportFactory::instance());
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );

}


PerlSupportPart::~PerlSupportPart()
{}


void PerlSupportPart::projectOpened()
{
    kdDebug(9007) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFileToProject(const QString &)),
             this, SLOT(addedFileToProject(const QString &)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString &)),
             this, SLOT(removedFileFromProject(const QString &)) );

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PerlSupportPart::projectClosed()
{
}


void PerlSupportPart::maybeParse(const QString fileName)
{
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    QString extension = fi.extension();
    if (extension == "pl" || extension == "pm") {
        classStore()->removeWithReferences(fileName);
        parse(fileName);
    }
}


void PerlSupportPart::initialParse()
{
    kdDebug(9016) << "initialParse()" << endl;
    
    if (project()) {
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allSourceFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
            kdDebug(9016) << "maybe parse " << (*it) << endl;
            maybeParse(*it);
        }
        
        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        kdDebug(9016) << "No project" << endl;
    }
}


void PerlSupportPart::addedFileToProject(const QString &fileName)
{
    kdDebug(9016) << "addedFileToProject()" << endl;
    maybeParse(fileName);
    emit updatedSourceInfo();
}


void PerlSupportPart::removedFileFromProject(const QString &fileName)
{
    kdDebug(9016) << "removedFileFromProject()" << endl;
    classStore()->removeWithReferences(fileName);
    emit updatedSourceInfo();
}


void PerlSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9016) << "savedFile()" << endl;

    if (project()->allSourceFiles().contains(fileName)) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PerlSupportPart::features()
{
    return Functions;
}


void PerlSupportPart::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);

    KRegExp subre("^[ \t]*sub[ \t]+([A-Za-z_]+).*$");

    QCString line;
    int lineNo = 0;
    while (!stream.atEnd()) {
        line = stream.readLine().stripWhiteSpace().latin1();
        if (subre.match(line)) {
            ParsedMethod *sub = new ParsedMethod;
            sub->setName(subre.group(1));
            sub->setDefinedInFile(fileName);
            sub->setDefinedOnLine(lineNo);
            
            ParsedMethod *old = classStore()->globalContainer.getMethod(sub);
            if (!old)
                classStore()->globalContainer.addMethod(sub);
        }
        ++lineNo;
    }
    
    f.close();
}

#include "perlsupportpart.moc"
